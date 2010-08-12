#ifndef __MINIMAL_H__
#define __MINIMAL_H__

#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <pthread.h>
#include "../PsxCommon.h"


#define DISPLAY_X 320
#define DISPLAY_Y 240

#define GP2X_BYTE_SIZE_SCREEN (320*240)

enum  { GP2X_UP=0x1,       GP2X_LEFT=0x4,       GP2X_DOWN=0x10,  GP2X_RIGHT=0x40,
        GP2X_START=1<<8,   GP2X_SELECT=1<<9,    GP2X_L=1<<10,    GP2X_R=1<<11,
        GP2X_A=1<<12,      GP2X_B=1<<13,        GP2X_X=1<<14,    GP2X_Y=1<<15,
        GP2X_VOL_UP=1<<23, GP2X_VOL_DOWN=1<<22, GP2X_PUSH=1<<27 };
                                            
#define gp2x_video_RGB_color16(R,G,B) ((unsigned short)(((((R)&0xF8)<<8)|(((G)&0xFC)<<3)|(((B)&0xF8)>>3))))

#define gp2x_screen16 screenbuffer

typedef struct gp2x_font        { int x,y,w,wmask,h,fg,bg,solid; unsigned char *data; } gp2x_font;

#ifdef __cplusplus
extern "C" {
#endif
extern float __audioVolume;
extern int app_OpenSound(void);
extern void app_CloseSound(void);
extern void app_StopSound();
extern void app_StartSound();
extern  void app_MuteSound();
extern  void app_DemuteSound();
extern int __emulation_run;
extern unsigned long global_enable_audio;

extern unsigned short BaseAddress[320*240];
extern unsigned short* videobuffer;
extern unsigned short* screenbuffer;

extern void updateScreen();
extern void gp2x_sound_play(void *buff, int len);
extern unsigned long gp2x_joystick_read(void);
extern void           gp2x_printf(gp2x_font *, int, int, const char *, ...);
extern void           gp2x_printf_init(gp2x_font *, int, int, void *, int, int, int);
extern void           gp2x_printfchar15(gp2x_font *, unsigned char);

extern void           gp2x_init(int, int, int, int, int, int, int);
extern void           gp2x_deinit(void);
extern void gp2x_video_flip(void);
extern u64 gp2x_timer_read(void);
extern void gp2x_change_res(int w, int h);
extern void gp2x_video_RGB_clearscreen16(void);
#ifdef __cplusplus
}
#endif

#define gp2x_timer_delay(T) usleep(T * 1000)
#define gp2x_timer_raw() gp2x_timer_read()
#define gp2x_timer_raw_second() (1000)
#define gp2x_timer_raw_to_ticks(t) (t)
#define gp2x_video_flip_single() gp2x_video_flip()

#define gp2x_sound_volume(LEFT,RIGHT)

#endif
