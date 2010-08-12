
#include "minimal.h"

int num_of_joys = 0;
struct usbjoy *joys[8];

void gp2x_usbjoy_init (void) {
	/* Open available joysticks -GnoStiC */
	int i, n = 0;

	printf("\n");
	for (i = 0; i < 4; i++) {
		joys[n] = joy_open(i+1);
		if (joys[n] && joy_buttons(joys[n]) > 0) {
			printf ("+-Joystick %d: \"%s\", buttons = %i\n", i+1, joy_name(joys[n]), joy_buttons(joys[n]));
			n++;
		}
	}
	num_of_joys = n;

	printf("Found %d Joystick(s)\n",num_of_joys);
	printf("\n");
}

void gp2x_usbjoy_update (void) {
	/* Update Joystick Event Cache */
	int q, foo;
	for (q=0; q < num_of_joys; q++) {
		foo = joy_update (joys[q]);
	}
}

unsigned long gp2x_usbjoy_check (int joyno) {
	/* Check Joystick */
	unsigned long q, joyExKey = 0;
	struct usbjoy *joy = joys[joyno];

	if (joy != NULL) {
		joy_update (joy);
		if (joy_getaxe(JOYUP, joy))    { joyExKey |= GP2X_UP; }
		if (joy_getaxe(JOYDOWN, joy))  { joyExKey |= GP2X_DOWN; }
		if (joy_getaxe(JOYLEFT, joy))  { joyExKey |= GP2X_LEFT; }
		if (joy_getaxe(JOYRIGHT, joy)) { joyExKey |= GP2X_RIGHT; }

		/* loop through joy buttons to check if they are pushed */
		for (q=0; q<joy_buttons (joy); q++) {
			if (joy_getbutton (q, joy)) {
				if (joy->type == JOY_TYPE_LOGITECH) {
					switch (q) {
						case 0: joyExKey |= GP2X_A; break;
						case 1: joyExKey |= GP2X_X; break;
						case 2: joyExKey |= GP2X_B; break;
						case 3: joyExKey |= GP2X_Y; break;
					}
				} else {
					switch (q) {
						case 0: joyExKey |= GP2X_Y; break;
						case 1: joyExKey |= GP2X_B; break;
						case 2: joyExKey |= GP2X_X; break;
						case 3: joyExKey |= GP2X_A; break;
					}
				}

				switch (q) {
					case  4: joyExKey |= GP2X_L; break;
					case  5: joyExKey |= GP2X_R; break;
					case  6: joyExKey |= GP2X_L; break; /* left shoulder button 2 */
					case  7: joyExKey |= GP2X_R; break; /* right shoulder button 2 */
					case  8: joyExKey |= GP2X_SELECT;break;
					case  9: joyExKey |= GP2X_START; break;
					case 10: joyExKey |= GP2X_PUSH;  break;
					case 11: joyExKey |= GP2X_PUSH;  break;
				}
			}
		}
	}
	return joyExKey;
}

unsigned long gp2x_usbjoy_check2 (int joyno) {
	/* Check Joystick, don't map to gp2x joy */
	unsigned long q, to, joyExKey = 0;
	struct usbjoy *joy = joys[joyno];

	if (joy != NULL) {
		if (joy_getaxe(JOYUP, joy))    { joyExKey |= 1 << 0; }
		if (joy_getaxe(JOYDOWN, joy))  { joyExKey |= 1 << 1; }
		if (joy_getaxe(JOYLEFT, joy))  { joyExKey |= 1 << 2; }
		if (joy_getaxe(JOYRIGHT, joy)) { joyExKey |= 1 << 3; }

		/* loop through joy buttons to check if they are pushed */
		to = joy->numbuttons;
		if (to > 32-4) to = 32-4;
		for (q=0; q < to; q++)
			if (joy->statebuttons[q]) joyExKey |= 1 << (q+4);
	}
	return joyExKey;
}



void gp2x_usbjoy_close (void) {
	int i;
	for (i=0; i<num_of_joys; i++) {
		joy_close (joys[i]);
		joys[i] = NULL;
	}
	num_of_joys = 0;
}


