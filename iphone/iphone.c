#include "../PsxCommon.h"
#include "minimal.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#import <AudioToolbox/AudioQueue.h>

#include "JoyPad.h"

const int isStereo = 0;
#define AUDIO_BUFFERS 2
#define AUDIO_BUFFER_SIZE (735 * 4)

extern void app_MuteSound(void);

typedef struct AQCallbackStruct {
    AudioQueueRef queue;
    UInt32 frameCount;
    AudioQueueBufferRef mBuffers[AUDIO_BUFFERS];
    AudioStreamBasicDescription mDataFormat;
} AQCallbackStruct;

unsigned long global_enable_audio = 0;
AQCallbackStruct in;
int soundInit = 0;
float __audioVolume = 1.0;
unsigned long gp2x_pad_status = 0;

extern void sound_callback(void *userdata, unsigned char *stream, int len);

static void AQBufferCallback(
							 void *userdata,
							 AudioQueueRef outQ,
							 AudioQueueBufferRef outQB)
{
	unsigned char *coreAudioBuffer;
	coreAudioBuffer = (unsigned char*) outQB->mAudioData;
	
	outQB->mAudioDataByteSize = AUDIO_BUFFER_SIZE;
	AudioQueueSetParameter(outQ, kAudioQueueParam_Volume, __audioVolume);
	//fprintf(stderr, "sound_lastlen %d\n", sound_lastlen);
	sound_callback(NULL, coreAudioBuffer, AUDIO_BUFFER_SIZE);
	AudioQueueEnqueueBuffer(outQ, outQB, 0, NULL);
}

int app_OpenSound() {
    Float64 sampleRate = 44100.0;
    int i;
    UInt32 bufferBytes;
	
    app_MuteSound();

#ifndef NOSOUND
	  if(global_enable_audio == 0)
	  {
      return 0;
	  }
	  
    soundInit = 0;
	
    in.mDataFormat.mSampleRate = sampleRate;
    in.mDataFormat.mFormatID = kAudioFormatLinearPCM;
    in.mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger
	| kAudioFormatFlagIsPacked;
    in.mDataFormat.mBytesPerPacket    =   4;
    in.mDataFormat.mFramesPerPacket   =   isStereo ? 1 : 2;
    in.mDataFormat.mBytesPerFrame     =   isStereo ? 4 : 2;
    in.mDataFormat.mChannelsPerFrame  =   isStereo ? 2 : 1;
    in.mDataFormat.mBitsPerChannel    =   16;
	
	
    /* Pre-buffer before we turn on audio */
    UInt32 err;
    err = AudioQueueNewOutput(&in.mDataFormat,
							  AQBufferCallback,
							  NULL,
							  NULL, //CFRunLoopGetMain(), //NULL,
							  kCFRunLoopDefaultMode,
							  0,
							  &in.queue);
	
	bufferBytes = AUDIO_BUFFER_SIZE;
	
	for (i=0; i<AUDIO_BUFFERS; i++) 
	{
		err = AudioQueueAllocateBuffer(in.queue, bufferBytes, &in.mBuffers[i]);
		/* "Prime" by calling the callback once per buffer */
		//AQBufferCallback (&in, in.queue, in.mBuffers[i]);
		in.mBuffers[i]->mAudioDataByteSize = AUDIO_BUFFER_SIZE; //samples_per_frame * 2; //inData->mDataFormat.mBytesPerFrame; //(inData->frameCount * 4 < (sndOutLen) ? inData->frameCount * 4 : (sndOutLen));
		AudioQueueEnqueueBuffer(in.queue, in.mBuffers[i], 0, NULL);
	}
	
	soundInit = 1;
	err = AudioQueueStart(in.queue, NULL);
#endif	
	return 0;
}

void app_CloseSound(void) {  
	if( soundInit == 1 )
	{
#ifndef NOSOUND	  
		AudioQueueDispose(in.queue, true);
#endif
		soundInit = 0;
	}
}


extern void app_MuteSound(void) {
	if( soundInit == 1 )
	{
		app_CloseSound();
	}
}

extern void app_DemuteSound(void) {
	if( soundInit == 0 )
	{
		app_OpenSound();
	}
}

unsigned long gp2x_joystick_read(void)
{
	return gp2x_pad_status;
}

