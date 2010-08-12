#include "pnd_event.h"
#include "pnd_type.h"
#include "minimal.h"

unsigned long pnd_keys = 0;
char event_name[30];
int fd_usbk, fd_usbm, fd_gpio, fd_pndk, fd_nub1, fd_nub2, fd_ts, rd, i, j, k;
struct input_event ev[64];
int version;
unsigned short id[4];
unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
char dev_name[256] = "Unknown";
int absolute[5];

char pnd_ts[20]   = "ADS784x Touchscreen";
char pnd_nub1[9]  = "vsense66";
char pnd_nub2[9]  = "vsense67";
char pnd_key[19]  = "omap_twl4030keypad";
char pnd_gpio[10] = "gpio-keys";

#define DEV_TS	 0
#define DEV_NUB1 1
#define DEV_NUB2 2
#define DEV_PNDK 3
#define DEV_GPIO 4
#define DEV_USBK 5
#define DEV_USBM 6

#define NUB1_CUTOFF 100
#define NUB2_CUTOFF 5
#define NUB2_SCALE  10

static int mx, my, buttonstate;

void PND_Setup_Controls( void )
{
	int event_key = 0;
	int event_mouse = 0;

	printf( "Setting up Pandora Controls\n" );

  pnd_keys = 0;
  
  // Static Controls
	// Pandora keyboard
	fd_pndk = PND_OpenEventDeviceByName(pnd_key);
	// Pandora buttons
	fd_gpio = PND_OpenEventDeviceByName(pnd_gpio);
	// Pandora touchscreen
	//fd_ts = PND_OpenEventDeviceByName(pnd_ts);
	// Pandora analog nub's
	fd_nub1 = PND_OpenEventDeviceByName(pnd_nub1);
	//fd_nub2 = PND_OpenEventDeviceByName(pnd_nub2);
    // Dynamic Controls
	// USB keyboard
	if( event_key > 0 ) {
		fd_usbk = PND_OpenEventDeviceByID(event_key);
	} else {
		printf( "No device selected for USB keyboard\n" );
	}

	// USB mouse
	if( event_mouse > 0 ) {
		fd_usbm = PND_OpenEventDeviceByID(event_mouse);
	} else {
		printf( "No device selected for USB mouse\n" );
	}
}

void PND_Close_Controls( void )
{
	printf( "Closing Pandora Controls\n" );

	if( fd_pndk > 0 )
		close(fd_pndk );
	if( fd_gpio > 0 )
		close(fd_gpio );
	if( fd_ts > 0 )
		close(fd_ts );
	if( fd_nub1 > 0 )
		close(fd_nub1 );
	if( fd_nub2 > 0 )
		close(fd_nub2 );
	if( fd_usbk > 0 )
		close(fd_usbk );
	if( fd_usbm > 0 )
		close(fd_usbm );
}

void PND_SendAllEvents( int *x, int *y, int *state, unsigned long *keys )
{
	// Read the events from the controls
	PND_SendKeyEvents(keys);
	PND_SendRelEvents( x, y, state );
	PND_SendAbsEvents();
}

void PND_SendKeyEvents( unsigned long *keys )
{
	PND_ReadEvents( fd_pndk, DEV_PNDK );
	PND_ReadEvents( fd_gpio, DEV_GPIO );
	PND_ReadEvents( fd_usbk, DEV_USBK );
	PND_ReadEvents( fd_nub1, DEV_NUB1 );
	
  *keys = pnd_keys;
}

void PND_SendRelEvents( int *x, int *y, int *state )
{
	PND_ReadEvents( fd_nub2, DEV_NUB2 );
	PND_ReadEvents( fd_usbm, DEV_USBM );

	*x = mx;
	*y = my;
	*state = buttonstate;
}

void PND_SendAbsEvents( void )
{
	PND_ReadEvents( fd_ts,   DEV_TS );
}

void PND_ReadEvents( int fd, int device )
{
	if( fd != 0 )
	{
		rd = read(fd, ev, sizeof(struct input_event) * 64);

		if (rd > (int) sizeof(struct input_event))
		{
			for (i = 0; i < rd / sizeof(struct input_event); i++)
			{
				PND_CheckEvent( &ev[i], device );
			}
		}
	}
}

void PND_CheckEvent( struct input_event *event, int device )
{
	unsigned long sym, value;
	long rel_x, rel_y;
	static long nub2_x = 0, nub2_y = 0;

	//printf( "Device %d Type %d Code %d Value %d\n", device, event->type, event->code, event->value );

	rel_x	= 0;
	rel_y	= 0;
	sym	= 0;
	value	= event->value;
	switch( event->type )
	{
		case EV_KEY:
			switch( event->code )
			{
				case KEY_UP:
					sym = GP2X_UP;
					break;
				case KEY_DOWN:
					sym = GP2X_DOWN;
					break;
				case KEY_LEFT:
					sym = GP2X_LEFT;
					break;
				case KEY_RIGHT:
					sym = GP2X_RIGHT;
					break;
				case KEY_MENU:		// menu
					sym = GP2X_PUSH;
					break;
				case KEY_LEFTALT:	// start
					sym = GP2X_START;
					break;
				case KEY_LEFTCTRL:	// select
					sym = GP2X_SELECT;
					break;
				case KEY_PAGEUP:	// up (y)
					sym = GP2X_Y;
					break;
				case KEY_END:		// left (a)
					sym = GP2X_A;
					break;
				case KEY_PAGEDOWN:	// down (x)
					sym = GP2X_X;
					break;
				case KEY_HOME:		// right (b)
					sym = GP2X_B;
					break;
				case KEY_RIGHTSHIFT:	// left shoulder
					sym = GP2X_L;
					break;
				case KEY_RIGHTCTRL:	// right shoulder
					sym = GP2X_R;
					break;
				case BTN_LEFT:
          sym = GP2X_VOL_DOWN;
					break;
				case BTN_RIGHT:
					sym = GP2X_VOL_UP;
          break;
				default:
//					if( keymap[event->code]>0 )
//						sym = keymap[event->code];
					break;
			}
			break;
		case EV_REL:
			switch( device )
			{
				case DEV_USBM:
					switch( event->code )
					{
						case REL_X:
							rel_x = value;
							break;
						case REL_Y:
							rel_y = value;
							break;
					}
					break;
			}
			break;
		case EV_ABS:
			switch( device )
			{
				case DEV_TS:
#if 0
					if( event->code == ABS_X ) {
						rel_x = abs_x - value;
						abs_x = value;
					}
					if( event->code == ABS_Y ) {
						rel_y = abs_y - value;
						abs_y = value;
					}
#endif
					break;
				case DEV_NUB1:
					if( event->code == ABS_X ) {
						//printf( "nub1 x %3d\n", value );
						if( abs(value) > NUB1_CUTOFF ) {
							if( value > 0 ) {
                pnd_keys &= ~(GP2X_LEFT);
								sym   = GP2X_RIGHT;
								value = 1;
							}
							else if( value < 0 ) {
                pnd_keys &= ~(GP2X_RIGHT);
								sym   = GP2X_LEFT;
								value = 1;						    
							}
						}
						else {
              pnd_keys &= ~(GP2X_LEFT);
              pnd_keys &= ~(GP2X_RIGHT);
						}
					}

					if( event->code == ABS_Y ) {
						//printf( "nub1 y %3d\n", value );
						if( abs(value) > NUB1_CUTOFF ) {
							if( value > 0 ) {
                pnd_keys &= ~(GP2X_UP);
								sym   = GP2X_DOWN;
								value = 1;
							}
							else if( value < 0 ) {
                pnd_keys &= ~(GP2X_DOWN);
								sym   = GP2X_UP;
								value = 1;
							}
						}
						else {
              pnd_keys &= ~(GP2X_UP);
              pnd_keys &= ~(GP2X_DOWN);
						}

					}
					break;
				case DEV_NUB2:
					if(event->code == ABS_X)
					{
						if( abs(value) > NUB2_CUTOFF ) {
							nub2_x = value / NUB2_SCALE;
						}
						else {
							nub2_x = 0;
						}
					}

					if(event->code == ABS_Y)
					{
						if( abs(value) > NUB2_CUTOFF ) {
							nub2_y = value / NUB2_SCALE;
						}
						else {
							nub2_y = 0;
						}
					}
					break;
			}
			break;
	}

	if( sym != 0 ) {
    if( value == 0 ) {
      pnd_keys &= ~sym;
    }
    else {
      pnd_keys |= sym;      
    }
	}

	if( rel_x != 0 || rel_y != 0 )
	{
		if( rel_x != 0 ) mx = rel_x*10;
		if( rel_y != 0 ) my = rel_y*10;
	}

	//printf( "nub2 x %3d y %3d\n", nub2_x, nub2_y );
	mx = nub2_x;
	my = nub2_y;
}

int PND_OpenEventDeviceByID( int event_id )
{
	int fd;

	snprintf( event_name, sizeof(event_name), "/dev/input/event%d", event_id );
	printf( "Device: %s\n", event_name );
	if ((fd = open(event_name, O_RDONLY |  O_NDELAY)) < 0) {
		perror("ERROR: Could not open device");
		return 0;
	}

	if (ioctl(fd, EVIOCGVERSION, &version)) {
		perror("evtest: can't get version");
		return 0;
	}

	printf("Input driver version is %d.%d.%d\n",
		version >> 16, (version >> 8) & 0xff, version & 0xff);

	ioctl(fd, EVIOCGID, id);
	printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
		id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

	ioctl(fd, EVIOCGNAME(sizeof(dev_name)), dev_name);
	printf("Input device name: \"%s\"\n", dev_name);

	return fd;
}

int PND_OpenEventDeviceByName( char device_name[] )
{
	int fd;

	for (i = 0; 1; i++)
	{
		snprintf( event_name, sizeof(event_name), "/dev/input/event%d", i );
		//printf( "Device: %s\n", event_name );
		if ((fd = open(event_name, O_RDONLY |  O_NDELAY)) < 0) {
			perror("ERROR: Could not open device");
			return 0;
		}
		if (fd < 0) break; /* no more devices */

		ioctl(fd, EVIOCGNAME(sizeof(dev_name)), dev_name);
		if (strcmp(dev_name, device_name) == 0)
		{
			if (ioctl(fd, EVIOCGVERSION, &version)) {
				perror("evtest: can't get version");
				return 0;
			}

			printf("Input driver version is %d.%d.%d\n",
				version >> 16, (version >> 8) & 0xff, version & 0xff);

			ioctl(fd, EVIOCGID, id);
			printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
				id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

			ioctl(fd, EVIOCGNAME(sizeof(dev_name)), dev_name);
			printf("Input device name: \"%s\"\n", dev_name);
		  
			return fd;
		}
		close(fd); /* we don't need this device */
	}
	return 0;
}
