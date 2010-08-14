#include <gtk/gtk.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <hildon/hildon.h>
#include "minimal.h"

GdkImage *image;
HildonAnimationActor *actor;
GtkWidget *window, *drawing;

#define X_RES           800
#define Y_RES           480
#define D_WIDTH			640
#define D_HEIGHT		480
		
int screen_size;
unsigned short* screenbuffer = NULL;

unsigned long keys = 0;

static void
window_key_proxy (GtkWidget *widget,
		     GdkEventKey *event,
		     gpointer user_data)
{
	unsigned long key = 0;
	
	switch (event->keyval) {
		case GDK_Left:
			key = GP2X_LEFT;
			break;
		case GDK_Right:
			key = GP2X_RIGHT;
			break;
		case GDK_Up:
			key = GP2X_UP;
			break;
		case GDK_Down:
			key = GP2X_DOWN;
			break;

		case GDK_x:
			key = GP2X_B;
			break;
		case GDK_z:
			key = GP2X_X;
			break;
		case GDK_s:
			key = GP2X_Y;
			break;
		case GDK_a:
			key = GP2X_A;
			break;
			
		case GDK_space:
			key = GP2X_SELECT;
			break;
		case GDK_KP_Enter:
		case GDK_Return:
			key = GP2X_START;
			break;

		case GDK_q:
			key = GP2X_VOL_DOWN;
			break;
		case GDK_r:
			key = GP2X_VOL_UP;
			break;
		case GDK_w:
			key = GP2X_L;
			break;
		case GDK_e:
			key = GP2X_R;
			break;

		default:
			key = 0;
	}
	
	if (event->type == GDK_KEY_PRESS) {
		keys |= key;
	}
	else if (event->type == GDK_KEY_RELEASE) {
		keys &= ~key;
	}

	//printf("Key 0x%x %s (0x%x)\n", key, event->type == GDK_KEY_PRESS ? "pressed" : "released", keys);
}

void hildon_quit()
{
	gp2x_deinit();
	gtk_main_quit();
	exit(0);
}

void hildon_init(int *argc, char ***argv)
{
	gtk_init (argc, argv);

	window = hildon_stackable_window_new ();
	gtk_widget_realize (window);
	gtk_window_fullscreen (GTK_WINDOW(window));
	g_signal_connect (G_OBJECT (window), "key-press-event",
				G_CALLBACK (window_key_proxy), NULL);
	g_signal_connect (G_OBJECT (window), "key-release-event",
				G_CALLBACK (window_key_proxy), NULL);
	g_signal_connect (G_OBJECT (window), "delete_event",
				G_CALLBACK (hildon_quit), NULL);
	gtk_widget_add_events (window,
				GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

	actor = HILDON_ANIMATION_ACTOR (hildon_animation_actor_new());
	hildon_animation_actor_set_position (actor, (X_RES - D_WIDTH)/2, (Y_RES - D_HEIGHT)/2 );
	hildon_animation_actor_set_parent (actor, GTK_WINDOW (window));

	drawing = gtk_image_new ();

	gtk_container_add (GTK_CONTAINER (actor), drawing);

	gtk_widget_show_all (GTK_WIDGET (actor));
	gtk_widget_show_all (GTK_WIDGET (window));
}

void gp2x_change_res(int w, int h)
{
	SysPrintf ("PSX resolution change: %dx%d", w, h);
	
	if (w <= 0 || h <= 0)
		return;

	if (image) gdk_image_destroy(image);
	image = gdk_image_new( GDK_IMAGE_FASTEST, gdk_visual_get_system(), w, h );

	screenbuffer = (unsigned short*) image->mem;
	screen_size = image->bpl * (h / (linesInterlace_user+1)) * image->bpp;

	gtk_image_set_from_image (GTK_IMAGE(drawing), image, NULL);

	gtk_window_resize (GTK_WINDOW (actor), w, h);
	hildon_animation_actor_set_scale (actor,
				(gdouble)D_WIDTH / (gdouble)w,
				(gdouble)D_HEIGHT / (gdouble)h
	);
}

unsigned long gp2x_joystick_read(void)
{
	//printf("gp2x_joystick_read\n");
	/* process GTK+ events */
	while (gtk_events_pending())
		gtk_main_iteration();

	return keys;
}

void gp2x_video_RGB_clearscreen16(void)
{
	if (gp2x_screen16 && screen_size)
		memset(gp2x_screen16, 0, screen_size);
}

void updateScreen()
{
	gtk_widget_queue_draw (drawing);
}

void gp2x_printfchar15(gp2x_font *f, unsigned char c)
{
  unsigned short *dst=&((unsigned short*)gp2x_screen16)[f->x+f->y*(image->bpl>>1)],w,h=f->h;
//unsigned char  *src=f->data[ (c%16)*f->w + (c/16)*f->h ];
  unsigned char  *src=&f->data[c*10];

 if(f->solid)
         while(h--)
         {
          w=f->wmask;
          while(w)
          {
           if( *src & w ) *dst++=f->fg; else *dst++=f->bg;
           w>>=1;
          }
          src++;    

          dst+=(image->bpl>>1)-(f->w);
         }
 else
         while(h--)
         {
          w=f->wmask;
          while(w)
          {
           if( *src & w ) *dst=f->fg;
           dst++;
           w>>=1;
          }
          src++;

          dst+=(image->bpl>>1)-(f->w);
         }
}

