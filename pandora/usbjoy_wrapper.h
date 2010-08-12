#ifndef __USBJOYWRAPPER__
#define __USBJOYWRAPPER__

#include "usbjoy.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct usbjoy * joys [8];
extern int num_of_joys;
extern int usbjoy_player1;
extern int usbjoy_tate;

extern void gp2x_usbjoy_init(void);
extern void gp2x_usbjoy_close (void);
extern unsigned long gp2x_usbjoy_check (int n);

#ifdef __cplusplus
}
#endif

#endif /* __USBJOYWRAPPER__ */
