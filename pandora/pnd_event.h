#ifndef PND_EVENT_H
#define PND_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

void PND_Setup_Controls( void );
void PND_Close_Controls( void );

void PND_SendAllEvents( int *x, int *y, int *state, unsigned long *keys );
void PND_SendKeyEvents( unsigned long *keys );
void PND_SendRelEvents( int *x, int *y, int *state );
void PND_SendAbsEvents( void );

#ifdef __cplusplus
}
#endif

#endif
