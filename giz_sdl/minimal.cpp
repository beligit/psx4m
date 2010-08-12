#include "minimal.h"
#include "profiler.h"

#ifdef PROFILER_PSX4ALL
#include "timeval.h"
#endif

#define MAX_KEYS 16

unsigned long gp2x_keys[MAX_KEYS];

void gp2x_key_config();

extern int     psx4all_emulating;

int gp2x_sdlwrapper_bpp=8;

void		*gp2x_sdlwrapper_screen_pixels=NULL;
SDL_Surface	*gp2x_sdlwrapper_screen=NULL;
SDL_Surface	*hw_screen=NULL;
double		gp2x_sdlwrapper_ticksdivisor=1.0;

static          unsigned long   gp2x_ticks_per_second;
static          unsigned short *gp2x_sound_buffer;
static volatile unsigned long   gp2x_ticks;

int	scale_height = 120;
long	gp2x_scaling_enabled = 0;

extern int soundcard;
extern int closing_sound;
extern void init_sound(unsigned rate, int stereo, int bits);
extern void quit_sound(void);

int gp2x_double_buffer=0;

/* The font is generated from Xorg 6x10-L1.bdf */
static unsigned char gp2x_fontf[256][10] = {
{ 0x00>>2, 0xA8>>2, 0x00>>2, 0x88>>2, 0x00>>2, 0x88>>2, 0x00>>2, 0xA8>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x20>>2, 0x70>>2, 0xF8>>2, 0x70>>2, 0x20>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0xA8>>2, 0x54>>2, 0xA8>>2, 0x54>>2, 0xA8>>2, 0x54>>2, 0xA8>>2, 0x54>>2, 0xA8>>2, 0x54>>2, },
{ 0x00>>2, 0x90>>2, 0x90>>2, 0xF0>>2, 0x90>>2, 0x90>>2, 0x78>>2, 0x10>>2, 0x10>>2, 0x10>>2, },
{ 0x00>>2, 0xE0>>2, 0x80>>2, 0xC0>>2, 0x80>>2, 0xB8>>2, 0x20>>2, 0x30>>2, 0x20>>2, 0x20>>2, },
{ 0x00>>2, 0x70>>2, 0x80>>2, 0x80>>2, 0x70>>2, 0x70>>2, 0x48>>2, 0x70>>2, 0x48>>2, 0x48>>2, },
{ 0x00>>2, 0x80>>2, 0x80>>2, 0x80>>2, 0xF0>>2, 0x78>>2, 0x40>>2, 0x70>>2, 0x40>>2, 0x40>>2, },
{ 0x00>>2, 0x20>>2, 0x50>>2, 0x20>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x20>>2, 0x20>>2, 0xF8>>2, 0x20>>2, 0x20>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x90>>2, 0xD0>>2, 0xD0>>2, 0xB0>>2, 0x90>>2, 0x40>>2, 0x40>>2, 0x40>>2, 0x78>>2, },
{ 0x00>>2, 0x90>>2, 0x90>>2, 0x60>>2, 0x40>>2, 0x78>>2, 0x10>>2, 0x10>>2, 0x10>>2, 0x10>>2, },
{ 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0xE0>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0xE0>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x3C>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, },
{ 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x3C>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0xFC>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, },
{ 0xFC>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0xFC>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0xFF>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0xFC>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0xFC>>2, },
{ 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x3C>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, },
{ 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0xE0>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, },
{ 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0xFC>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0xFC>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, },
{ 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, },
{ 0x00>>2, 0x18>>2, 0x60>>2, 0x80>>2, 0x60>>2, 0x18>>2, 0x00>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0xC0>>2, 0x30>>2, 0x08>>2, 0x30>>2, 0xC0>>2, 0x00>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0xF8>>2, 0x50>>2, 0x50>>2, 0x50>>2, 0x50>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x08>>2, 0x10>>2, 0xF8>>2, 0x20>>2, 0xF8>>2, 0x40>>2, 0x80>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x30>>2, 0x48>>2, 0x40>>2, 0xE0>>2, 0x40>>2, 0x48>>2, 0xB0>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x20>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x00>>2, 0x20>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x50>>2, 0x50>>2, 0x50>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x50>>2, 0x50>>2, 0xF8>>2, 0x50>>2, 0xF8>>2, 0x50>>2, 0x50>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x20>>2, 0x70>>2, 0xA0>>2, 0x70>>2, 0x28>>2, 0x70>>2, 0x20>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x48>>2, 0xA8>>2, 0x50>>2, 0x20>>2, 0x50>>2, 0xA8>>2, 0x90>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x40>>2, 0xA0>>2, 0xA0>>2, 0x40>>2, 0xA8>>2, 0x90>>2, 0x68>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x10>>2, 0x20>>2, 0x40>>2, 0x40>>2, 0x40>>2, 0x20>>2, 0x10>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x40>>2, 0x20>>2, 0x10>>2, 0x10>>2, 0x10>>2, 0x20>>2, 0x40>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x88>>2, 0x50>>2, 0xF8>>2, 0x50>>2, 0x88>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x20>>2, 0x20>>2, 0xF8>>2, 0x20>>2, 0x20>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x30>>2, 0x20>>2, 0x40>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0xF8>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x20>>2, 0x70>>2, 0x20>>2, 0x00>>2, },
{ 0x00>>2, 0x08>>2, 0x08>>2, 0x10>>2, 0x20>>2, 0x40>>2, 0x80>>2, 0x80>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x20>>2, 0x50>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x50>>2, 0x20>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x20>>2, 0x60>>2, 0xA0>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0x08>>2, 0x30>>2, 0x40>>2, 0x80>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0xF8>>2, 0x08>>2, 0x10>>2, 0x30>>2, 0x08>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x10>>2, 0x30>>2, 0x50>>2, 0x90>>2, 0xF8>>2, 0x10>>2, 0x10>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0xF8>>2, 0x80>>2, 0xB0>>2, 0xC8>>2, 0x08>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x30>>2, 0x40>>2, 0x80>>2, 0xB0>>2, 0xC8>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0xF8>>2, 0x08>>2, 0x10>>2, 0x10>>2, 0x20>>2, 0x40>>2, 0x40>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0x98>>2, 0x68>>2, 0x08>>2, 0x10>>2, 0x60>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x20>>2, 0x70>>2, 0x20>>2, 0x00>>2, 0x20>>2, 0x70>>2, 0x20>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x20>>2, 0x70>>2, 0x20>>2, 0x00>>2, 0x30>>2, 0x20>>2, 0x40>>2, 0x00>>2, },
{ 0x00>>2, 0x08>>2, 0x10>>2, 0x20>>2, 0x40>>2, 0x20>>2, 0x10>>2, 0x08>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0xF8>>2, 0x00>>2, 0xF8>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x40>>2, 0x20>>2, 0x10>>2, 0x08>>2, 0x10>>2, 0x20>>2, 0x40>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0x10>>2, 0x20>>2, 0x20>>2, 0x00>>2, 0x20>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0x98>>2, 0xA8>>2, 0xB0>>2, 0x80>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x20>>2, 0x50>>2, 0x88>>2, 0x88>>2, 0xF8>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0xF0>>2, 0x48>>2, 0x48>>2, 0x70>>2, 0x48>>2, 0x48>>2, 0xF0>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0x80>>2, 0x80>>2, 0x80>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0xF0>>2, 0x48>>2, 0x48>>2, 0x48>>2, 0x48>>2, 0x48>>2, 0xF0>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0xF8>>2, 0x80>>2, 0x80>>2, 0xF0>>2, 0x80>>2, 0x80>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0xF8>>2, 0x80>>2, 0x80>>2, 0xF0>>2, 0x80>>2, 0x80>>2, 0x80>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0x80>>2, 0x80>>2, 0x98>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0xF8>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x38>>2, 0x10>>2, 0x10>>2, 0x10>>2, 0x10>>2, 0x90>>2, 0x60>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x88>>2, 0x90>>2, 0xA0>>2, 0xC0>>2, 0xA0>>2, 0x90>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x80>>2, 0x80>>2, 0x80>>2, 0x80>>2, 0x80>>2, 0x80>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x88>>2, 0x88>>2, 0xD8>>2, 0xA8>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x88>>2, 0x88>>2, 0xC8>>2, 0xA8>>2, 0x98>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0xF0>>2, 0x88>>2, 0x88>>2, 0xF0>>2, 0x80>>2, 0x80>>2, 0x80>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0xA8>>2, 0x70>>2, 0x08>>2, 0x00>>2, },
{ 0x00>>2, 0xF0>>2, 0x88>>2, 0x88>>2, 0xF0>>2, 0xA0>>2, 0x90>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0x80>>2, 0x70>>2, 0x08>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0xF8>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x50>>2, 0x50>>2, 0x50>>2, 0x20>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0xA8>>2, 0xA8>>2, 0xD8>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x88>>2, 0x88>>2, 0x50>>2, 0x20>>2, 0x50>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x88>>2, 0x88>>2, 0x50>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0xF8>>2, 0x08>>2, 0x10>>2, 0x20>>2, 0x40>>2, 0x80>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x40>>2, 0x40>>2, 0x40>>2, 0x40>>2, 0x40>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x80>>2, 0x80>>2, 0x40>>2, 0x20>>2, 0x10>>2, 0x08>>2, 0x08>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x10>>2, 0x10>>2, 0x10>>2, 0x10>>2, 0x10>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x20>>2, 0x50>>2, 0x88>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0xF8>>2, 0x00>>2, },
{ 0x20>>2, 0x10>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x70>>2, 0x08>>2, 0x78>>2, 0x88>>2, 0x78>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x80>>2, 0x80>>2, 0xB0>>2, 0xC8>>2, 0x88>>2, 0xC8>>2, 0xB0>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0x80>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x08>>2, 0x08>>2, 0x68>>2, 0x98>>2, 0x88>>2, 0x98>>2, 0x68>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0xF8>>2, 0x80>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x30>>2, 0x48>>2, 0x40>>2, 0xF0>>2, 0x40>>2, 0x40>>2, 0x40>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x78>>2, 0x88>>2, 0x88>>2, 0x78>>2, 0x08>>2, 0x88>>2, 0x70>>2, },
{ 0x00>>2, 0x80>>2, 0x80>>2, 0xB0>>2, 0xC8>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x20>>2, 0x00>>2, 0x60>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x08>>2, 0x00>>2, 0x18>>2, 0x08>>2, 0x08>>2, 0x08>>2, 0x48>>2, 0x48>>2, 0x30>>2, },
{ 0x00>>2, 0x80>>2, 0x80>>2, 0x88>>2, 0x90>>2, 0xE0>>2, 0x90>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x60>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0xD0>>2, 0xA8>>2, 0xA8>>2, 0xA8>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0xB0>>2, 0xC8>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0xB0>>2, 0xC8>>2, 0x88>>2, 0xC8>>2, 0xB0>>2, 0x80>>2, 0x80>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x68>>2, 0x98>>2, 0x88>>2, 0x98>>2, 0x68>>2, 0x08>>2, 0x08>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0xB0>>2, 0xC8>>2, 0x80>>2, 0x80>>2, 0x80>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x70>>2, 0x80>>2, 0x70>>2, 0x08>>2, 0xF0>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x40>>2, 0x40>>2, 0xF0>>2, 0x40>>2, 0x40>>2, 0x48>>2, 0x30>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x98>>2, 0x68>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x88>>2, 0x88>>2, 0x50>>2, 0x50>>2, 0x20>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x88>>2, 0x88>>2, 0xA8>>2, 0xA8>>2, 0x50>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x88>>2, 0x50>>2, 0x20>>2, 0x50>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x88>>2, 0x88>>2, 0x98>>2, 0x68>>2, 0x08>>2, 0x88>>2, 0x70>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0xF8>>2, 0x10>>2, 0x20>>2, 0x40>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x18>>2, 0x20>>2, 0x10>>2, 0x60>>2, 0x10>>2, 0x20>>2, 0x18>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x60>>2, 0x10>>2, 0x20>>2, 0x18>>2, 0x20>>2, 0x10>>2, 0x60>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x48>>2, 0xA8>>2, 0x90>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x20>>2, 0x00>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x20>>2, 0x78>>2, 0xA0>>2, 0xA0>>2, 0xA0>>2, 0x78>>2, 0x20>>2, 0x00>>2, },
{ 0x00>>2, 0x30>>2, 0x48>>2, 0x40>>2, 0xE0>>2, 0x40>>2, 0x48>>2, 0xB0>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x88>>2, 0x70>>2, 0x50>>2, 0x70>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x88>>2, 0x88>>2, 0x50>>2, 0x20>>2, 0xF8>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x00>>2, },
{ 0x00>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x00>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x80>>2, 0xE0>>2, 0x90>>2, 0x48>>2, 0x38>>2, 0x08>>2, 0x70>>2, 0x00>>2, },
{ 0x50>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0xA8>>2, 0xC8>>2, 0xA8>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x38>>2, 0x48>>2, 0x58>>2, 0x28>>2, 0x00>>2, 0x78>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x24>>2, 0x48>>2, 0x90>>2, 0x48>>2, 0x24>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x78>>2, 0x08>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x78>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0xE8>>2, 0xC8>>2, 0xC8>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0xF8>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x20>>2, 0x50>>2, 0x20>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x20>>2, 0x20>>2, 0xF8>>2, 0x20>>2, 0x20>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x30>>2, 0x48>>2, 0x10>>2, 0x20>>2, 0x78>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x70>>2, 0x08>>2, 0x30>>2, 0x08>>2, 0x70>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x10>>2, 0x20>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0xC8>>2, 0xB0>>2, 0x80>>2, 0x00>>2, },
{ 0x00>>2, 0x78>>2, 0xE8>>2, 0xE8>>2, 0x68>>2, 0x28>>2, 0x28>>2, 0x28>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x20>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x10>>2, 0x20>>2, },
{ 0x20>>2, 0x60>>2, 0x20>>2, 0x20>>2, 0x70>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x30>>2, 0x48>>2, 0x48>>2, 0x30>>2, 0x00>>2, 0x78>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x90>>2, 0x48>>2, 0x24>>2, 0x48>>2, 0x90>>2, 0x00>>2, 0x00>>2, },
{ 0x40>>2, 0xC0>>2, 0x40>>2, 0x40>>2, 0xE4>>2, 0x0C>>2, 0x14>>2, 0x3C>>2, 0x04>>2, 0x00>>2, },
{ 0x40>>2, 0xC0>>2, 0x40>>2, 0x40>>2, 0xE8>>2, 0x14>>2, 0x04>>2, 0x08>>2, 0x1C>>2, 0x00>>2, },
{ 0xC0>>2, 0x20>>2, 0x40>>2, 0x20>>2, 0xC8>>2, 0x18>>2, 0x28>>2, 0x78>>2, 0x08>>2, 0x00>>2, },
{ 0x00>>2, 0x20>>2, 0x00>>2, 0x20>>2, 0x20>>2, 0x40>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x40>>2, 0x20>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0xF8>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x10>>2, 0x20>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0xF8>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0x50>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0xF8>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x48>>2, 0xB0>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0xF8>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x50>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0xF8>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0x50>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0xF8>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x3C>>2, 0x50>>2, 0x90>>2, 0x9C>>2, 0xF0>>2, 0x90>>2, 0x9C>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0x80>>2, 0x80>>2, 0x80>>2, 0x88>>2, 0x70>>2, 0x20>>2, 0x40>>2, },
{ 0x40>>2, 0xF8>>2, 0x80>>2, 0x80>>2, 0xF0>>2, 0x80>>2, 0x80>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x10>>2, 0xF8>>2, 0x80>>2, 0x80>>2, 0xF0>>2, 0x80>>2, 0x80>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0xF8>>2, 0x80>>2, 0x80>>2, 0xF0>>2, 0x80>>2, 0x80>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x50>>2, 0xF8>>2, 0x80>>2, 0x80>>2, 0xF0>>2, 0x80>>2, 0x80>>2, 0xF8>>2, 0x00>>2, 0x00>>2, },
{ 0x40>>2, 0x20>>2, 0x70>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x10>>2, 0x20>>2, 0x70>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0x50>>2, 0x70>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x50>>2, 0x00>>2, 0x70>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0xF0>>2, 0x48>>2, 0x48>>2, 0xE8>>2, 0x48>>2, 0x48>>2, 0xF0>>2, 0x00>>2, 0x00>>2, },
{ 0x28>>2, 0x50>>2, 0x88>>2, 0xC8>>2, 0xA8>>2, 0x98>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x40>>2, 0x20>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x10>>2, 0x20>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0x50>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x28>>2, 0x50>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x50>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x88>>2, 0x50>>2, 0x20>>2, 0x50>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x98>>2, 0x98>>2, 0xA8>>2, 0xC8>>2, 0xC8>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x40>>2, 0x20>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x10>>2, 0x20>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0x50>>2, 0x00>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x50>>2, 0x00>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x10>>2, 0x20>>2, 0x88>>2, 0x88>>2, 0x50>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x80>>2, 0xF0>>2, 0x88>>2, 0xF0>>2, 0x80>>2, 0x80>>2, 0x80>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x70>>2, 0x88>>2, 0x90>>2, 0xA0>>2, 0x90>>2, 0x88>>2, 0xB0>>2, 0x00>>2, 0x00>>2, },
{ 0x40>>2, 0x20>>2, 0x00>>2, 0x70>>2, 0x08>>2, 0x78>>2, 0x88>>2, 0x78>>2, 0x00>>2, 0x00>>2, },
{ 0x10>>2, 0x20>>2, 0x00>>2, 0x70>>2, 0x08>>2, 0x78>>2, 0x88>>2, 0x78>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0x50>>2, 0x00>>2, 0x70>>2, 0x08>>2, 0x78>>2, 0x88>>2, 0x78>>2, 0x00>>2, 0x00>>2, },
{ 0x28>>2, 0x50>>2, 0x00>>2, 0x70>>2, 0x08>>2, 0x78>>2, 0x88>>2, 0x78>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x50>>2, 0x00>>2, 0x70>>2, 0x08>>2, 0x78>>2, 0x88>>2, 0x78>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0x50>>2, 0x20>>2, 0x70>>2, 0x08>>2, 0x78>>2, 0x88>>2, 0x78>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x78>>2, 0x14>>2, 0x7C>>2, 0x90>>2, 0x7C>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0x80>>2, 0x88>>2, 0x70>>2, 0x20>>2, 0x40>>2, },
{ 0x40>>2, 0x20>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0xF8>>2, 0x80>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x10>>2, 0x20>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0xF8>>2, 0x80>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0x50>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0xF8>>2, 0x80>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x50>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0xF8>>2, 0x80>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x40>>2, 0x20>>2, 0x00>>2, 0x60>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0x40>>2, 0x00>>2, 0x60>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0x50>>2, 0x00>>2, 0x60>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x50>>2, 0x00>>2, 0x60>>2, 0x20>>2, 0x20>>2, 0x20>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0xC0>>2, 0x30>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x28>>2, 0x50>>2, 0x00>>2, 0xB0>>2, 0xC8>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x00>>2, 0x00>>2, },
{ 0x40>>2, 0x20>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x10>>2, 0x20>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0x50>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x28>>2, 0x50>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x50>>2, 0x00>>2, 0x70>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x70>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x20>>2, 0x00>>2, 0xF8>>2, 0x00>>2, 0x20>>2, 0x00>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x00>>2, 0x00>>2, 0x78>>2, 0x98>>2, 0xA8>>2, 0xC8>>2, 0xF0>>2, 0x00>>2, 0x00>>2, },
{ 0x40>>2, 0x20>>2, 0x00>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x98>>2, 0x68>>2, 0x00>>2, 0x00>>2, },
{ 0x10>>2, 0x20>>2, 0x00>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x98>>2, 0x68>>2, 0x00>>2, 0x00>>2, },
{ 0x20>>2, 0x50>>2, 0x00>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x98>>2, 0x68>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x50>>2, 0x00>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0x98>>2, 0x68>>2, 0x00>>2, 0x00>>2, },
{ 0x00>>2, 0x10>>2, 0x20>>2, 0x88>>2, 0x88>>2, 0x98>>2, 0x68>>2, 0x08>>2, 0x88>>2, 0x70>>2, },
{ 0x00>>2, 0x00>>2, 0x80>>2, 0xF0>>2, 0x88>>2, 0x88>>2, 0x88>>2, 0xF0>>2, 0x80>>2, 0x80>>2, },
{ 0x00>>2, 0x50>>2, 0x00>>2, 0x88>>2, 0x88>>2, 0x98>>2, 0x68>>2, 0x08>>2, 0x88>>2, 0x70>>2, },
};

#define itslength(x) (sizeof(x)/sizeof(x[0]))

static gp2x_font gp2x_default_font;

void (*gp2x_printfchar)(gp2x_font *f, unsigned char c);

void gp2x_printfchar15(gp2x_font *f, unsigned char c)
{
  unsigned short *dst=&((unsigned short*)sdlscreen->pixels)[f->x+f->y*(sdlscreen->pitch>>1)],w,h=f->h;
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

          dst+=(sdlscreen->pitch>>1)-(f->w);
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

          dst+=(sdlscreen->pitch>>1)-(f->w);
         }
}

void gp2x_printf(gp2x_font *f, int x, int y, const char *format, ...)
{
 char buffer[4096]; int c; gp2x_font *g=&gp2x_default_font;
 va_list  args;

 va_start(args, format);
 vsprintf(buffer, format, args);

 if(f!=NULL) g=f;

 if(x<0) x=g->x; else g->x=x; 
 if(y<0) y=g->y; else g->y=y;

 for(c=0;buffer[c];c++)
 {
  switch(buffer[c])
  {
   case '\b': g->x=x;g->y=y; break;

   case '\n': g->y+=g->h;
   case '\r': g->x=x;
              break;

   default:   gp2x_printfchar(g, (unsigned char)buffer[c]);
              g->x+=g->w;
              break;
  }
 }

 gp2x_video_flip_single();
}

void gp2x_printf_init(gp2x_font *f, int w, int h, void *data, int fg, int bg, int solid)
{
 gp2x_printfchar=gp2x_printfchar15;
 f->x=f->y=0;
 f->wmask=1<<(w-1);
 f->w=w;
 f->h=h;
 f->data=(unsigned char *)data;
 f->fg=fg;
 f->bg=bg;
 f->solid=solid;
}

void gp2x_init(int ticks_per_second, int bpp, int rate, int bits, int stereo, int hz, int solid_font)
{
	static int first=1;
	static int ramtweaks=0;
	FILE* configf = fopen("config.txt", "r");

	gp2x_double_buffer=0;
	SDL_Init(SDL_INIT_VIDEO);
	gp2x_sdlwrapper_screen = SDL_SetVideoMode(320, 240, 16, SDL_FULLSCREEN);
	//gp2x_sdlwrapper_screen = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 16, 0xFFFF, 0xFFFF, 0xFFFF, 0);


	if(gp2x_sdlwrapper_screen == NULL) 
	{
		exit(0);
		return;
	}
	gp2x_sdlwrapper_bpp=bpp;
	gp2x_sdlwrapper_screen_pixels=gp2x_sdlwrapper_screen->pixels;

	SDL_EnableUNICODE(1);	
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_EventState(SDL_ACTIVEEVENT,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEMOTION,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP,SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT,SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE,SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT,SDL_IGNORE);
	SDL_ShowCursor(SDL_DISABLE);

	//init misc variables
	if(first)
	{	
		gp2x_ticks=0;
		gp2x_ticks_per_second=7372800/ticks_per_second;
	}

	//init font
	gp2x_printf_init(&gp2x_default_font,6,10,gp2x_fontf,0xFFFF,0x0000,solid_font);

	if( configf == NULL )
	{
		gp2x_key_config();
		configf = fopen("config.txt", "w");
		if( configf )
		{
			char keyz[1024];
			sprintf(keyz, "%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u", 
				gp2x_keys[0], gp2x_keys[1], gp2x_keys[2], 
				gp2x_keys[3], gp2x_keys[4], gp2x_keys[5], 
				gp2x_keys[6], gp2x_keys[7], gp2x_keys[8], 
				gp2x_keys[9], gp2x_keys[10], gp2x_keys[11], 
				gp2x_keys[12], gp2x_keys[13], gp2x_keys[14], 
				gp2x_keys[15]);
			fputs(keyz, configf);
			fclose(configf);
		}
	} else {
		char keyz[1024];
		fgets(keyz, 1022, configf );
		sscanf(keyz, "%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u:%u", 
			&gp2x_keys[0], &gp2x_keys[1], &gp2x_keys[2], 
			&gp2x_keys[3], &gp2x_keys[4], &gp2x_keys[5], 
			&gp2x_keys[6], &gp2x_keys[7], &gp2x_keys[8], 
			&gp2x_keys[9], &gp2x_keys[10], &gp2x_keys[11], 
			&gp2x_keys[12], &gp2x_keys[13], &gp2x_keys[14], 
			&gp2x_keys[15]);
		fclose(configf);
	}
}

void gp2x_change_res(int w, int h)
{
#if 0
	//SDL_Quit();
	//SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE);
        gp2x_sdlwrapper_screen = SDL_SetVideoMode(w, h, 16, SDL_HWSURFACE);

	if(gp2x_sdlwrapper_screen == NULL) 
	{
		exit(0);
		return;
	}
	gp2x_sdlwrapper_bpp=16;
	gp2x_sdlwrapper_screen_pixels=gp2x_sdlwrapper_screen->pixels;

	SDL_EnableUNICODE(1);
	SDL_EventState(SDL_ACTIVEEVENT,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEMOTION,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN,SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP,SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT,SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE,SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT,SDL_IGNORE);
	SDL_ShowCursor(SDL_DISABLE);
#endif
}

void gp2x_deinit(void)
{
  //fcloseall(); // TODO: Implement in WinCE

  SDL_Quit();

  exit(0);
}

static void keyprocess(Uint32 *st, unsigned long key, SDL_bool pressed)
{
	Uint32 val=0;
	Uint32 the_key = MAX_KEYS;
	for(int i = 0; i < MAX_KEYS; i++)
	{
		if(gp2x_keys[i] == key)
		{
			the_key = i; break;
		}
	}

	switch(the_key)
	{
		case 0:
			val=GP2X_UP; break;
		case 1:
			val=GP2X_DOWN; break;
		case 2:
			val=GP2X_LEFT; break;
		case 3:
			val=GP2X_RIGHT; break;
		case 4:
			val=GP2X_A; break;
		case 5:
			val=GP2X_B; break;
		case 6:
			val=GP2X_Y; break;
		case 7:
			val=GP2X_X; break;
		//case 8:
		//	val=GP2X_PUSH; break;
		case 9:
			val=GP2X_L; break;
		case 10:
			val=GP2X_R; break;
		case 11:
			val=GP2X_START; break;
		case 12:
			val=GP2X_SELECT; break;
		case 13:
			val=GP2X_VOL_UP; break;
		case 14:
			val=GP2X_VOL_DOWN; break;
		//case 15:
		//	val=GP2X_VOL_UP|GP2X_VOL_DOWN; break;

		default:
			return;
	}

	if (pressed)
		(*st) |= val;
	else
		(*st) &= ~val;
}

unsigned long gp2x_joystick_read(void)
{
	static Uint32 st=0;
	SDL_Event event;

	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_KEYDOWN:
				if(event.key.keysym.sym == SDLK_BACKSPACE)
				{
					gp2x_deinit();
				}
				keyprocess(&st,event.key.keysym.scancode,SDL_TRUE);
				keyprocess(&st,event.key.keysym.unicode,SDL_TRUE);
				keyprocess(&st,event.key.keysym.sym,SDL_TRUE);
				break;
			case SDL_KEYUP:
				keyprocess(&st,event.key.keysym.scancode,SDL_FALSE);
				keyprocess(&st,event.key.keysym.unicode,SDL_FALSE);
				keyprocess(&st,event.key.keysym.sym,SDL_FALSE);
				break;
		}
	}

	return st;
}

void gp2x_key_config()
{
	SDL_Event event;

	int key_num = 0;
	while( key_num < MAX_KEYS )
	{
		if( key_num == 8 || key_num == 15 ) 
		{
			key_num++;
			continue;
		}

		switch(key_num)
		{
		case 0: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS UP TO CONTINUE"); break;
		case 1: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS DOWN TO CONTINUE"); break;
		case 2: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS LEFT TO CONTINUE"); break;
		case 3: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS RIGHT TO CONTINUE"); break;
		case 4: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS BUTTON1 (SQUARE) TO CONTINUE"); break;
		case 5: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS BUTTON2 (CIRCLE) TO CONTINUE"); break;
		case 6: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS BUTTON3 (TRIANGLE) TO CONTINUE"); break;
		case 7: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS BUTTON4 (X) TO CONTINUE"); break;
		//case 8: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS PUSH BUTTON TO CONTINUE"); break;
		case 9: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS L BUTTON TO CONTINUE"); break;
		case 10: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS R BUTTON TO CONTINUE"); break;
		case 11: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS START BUTTON TO CONTINUE"); break;
		case 12: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS SELECT BUTTON TO CONTINUE"); break;
		case 13: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS VOL UP BUTTON TO CONTINUE"); break;
		case 14: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS VOL DOWN BUTTON TO CONTINUE"); break;
		//case 15: 	gp2x_printf(NULL, 10, 120, "KEY CONFIG: PRESS VOL MIDDLE BUTTON TO CONTINUE"); break;
		}
		
		gp2x_video_flip();
		
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
					{
					int keysyms;
					for(keysyms = 0; keysyms < 3; keysyms++)
					{
						int the_sym;
						if( keysyms == 0) the_sym = event.key.keysym.sym;
						if( keysyms == 1) the_sym = event.key.keysym.unicode;
						if( keysyms == 2) the_sym = event.key.keysym.scancode;

						if( the_sym ) 
						{
							int key_exists = 0;
							for(int  i = 0; i < MAX_KEYS; i++)
							{
								if(gp2x_keys[i] == the_sym)
								{
									key_exists = 1;
								}
							}
							if( key_exists == 0 )
							{
								gp2x_keys[key_num] = the_sym;
								break;
							}
						}
					}
					}
					break;

				case SDL_KEYUP:
					if( event.key.keysym.sym != 0 &&  event.key.keysym.sym == gp2x_keys[key_num]) key_num++;
					else if( event.key.keysym.unicode != 0 && event.key.keysym.unicode == gp2x_keys[key_num]) key_num++;
					else if( event.key.keysym.scancode != 0 && event.key.keysym.scancode == gp2x_keys[key_num]) key_num++;
					gp2x_video_RGB_clearscreen16();
					gp2x_video_flip();
					gp2x_timer_delay(1000);
					break;
			}
		}
	}
}

void gp2x_video_RGB_clearscreen16(void)
{
  memset(sdlscreen->pixels, 0, sdlscreen->pitch*sdlscreen->h);
}
