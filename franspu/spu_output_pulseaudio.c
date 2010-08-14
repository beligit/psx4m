#include "spu_callback.h"
#include <minimal.h>

#include <pulse/pulseaudio.h>

unsigned char* SPU_async_X(int nsamples);

////////////////////////////////////////////////////////////////////////
// pulseaudio structs
////////////////////////////////////////////////////////////////////////

typedef struct {
     pa_threaded_mainloop *mainloop;
     pa_context *context;
     pa_mainloop_api *api;
     pa_stream *stream;
     pa_sample_spec spec;
     int first;
     pa_buffer_attr buffer_attributes;
} Device;

typedef struct {
     unsigned int frequency;
     unsigned int latency_in_msec;
} Settings;

////////////////////////////////////////////////////////////////////////
// pulseaudio globals
////////////////////////////////////////////////////////////////////////

static Device device = {
     NULL,
     NULL,
     NULL,
     NULL
};

static Settings settings = {
     44100,
     20,
};

////////////////////////////////////////////////////////////////////////
// CALLBACKS FOR THREADED MAINLOOP
////////////////////////////////////////////////////////////////////////
static void context_state_cb (pa_context *context, void *userdata)
{
     Device *dev = (Device *) userdata;

     if ((context == NULL) || (dev == NULL))
	  return;

     switch (pa_context_get_state (context))
     {
     case PA_CONTEXT_READY:
     case PA_CONTEXT_TERMINATED:
     case PA_CONTEXT_FAILED:
	  pa_threaded_mainloop_signal (dev->mainloop, 0);
	  break;

     case PA_CONTEXT_UNCONNECTED:
     case PA_CONTEXT_CONNECTING:
     case PA_CONTEXT_AUTHORIZING:
     case PA_CONTEXT_SETTING_NAME:
	  break;
     }
}

static void stream_state_cb (pa_stream *stream, void * userdata)
{
     Device *dev = (Device *) userdata;
    
     if ((stream == NULL) || (dev == NULL))
	  return;

     switch (pa_stream_get_state (stream))
     {
     case PA_STREAM_READY:
     case PA_STREAM_FAILED:
     case PA_STREAM_TERMINATED:
	  pa_threaded_mainloop_signal (dev->mainloop, 0);
	  break;

     case PA_STREAM_UNCONNECTED:
     case PA_STREAM_CREATING:
	  break;
     }
}

static void stream_latency_update_cb (pa_stream *stream, void *userdata)
{
     Device *dev = (Device *) userdata;

     if ((stream == NULL) || (dev == NULL))
	  return;

     pa_threaded_mainloop_signal (dev->mainloop, 0);
}

static void stream_request_cb (pa_stream *stream, size_t length, void *userdata)
{
     Device *dev = (Device *) userdata;

     if ((stream == NULL) || (dev == NULL))
	  return;
	 
	 length -= length % dev->buffer_attributes.minreq;
     if ((!iSoundMuted) && dev->mainloop != NULL && length > 0)
     {
	  pa_threaded_mainloop_lock (dev->mainloop);
	  if (pa_stream_write (dev->stream, SPU_async_X(length/2), length, NULL, 0LL, PA_SEEK_RELATIVE) < 0)
	  {
	       SysPrintf ("Could not perform write\n");
	  }
	  else
	  {
	       //fprintf (stderr, "Wrote %d bytes\n", length);
	       pa_threaded_mainloop_unlock (dev->mainloop);
	  }
     }

     pa_threaded_mainloop_signal (dev->mainloop, 0);
}

////////////////////////////////////////////////////////////////////////
// SETUP SOUND
////////////////////////////////////////////////////////////////////////

void SetupSound (void)
{
#ifndef NOSOUND
     int error_number;

     // Acquire mainloop ///////////////////////////////////////////////////////
     device.mainloop = pa_threaded_mainloop_new ();
     if (device.mainloop == NULL)
     {
	  SysPrintf ("Could not acquire PulseAudio main loop");
	  return;
     }

     // Acquire context ////////////////////////////////////////////////////////
     device.api = pa_threaded_mainloop_get_api (device.mainloop);
     device.context = pa_context_new (device.api, "PSX4ALL");
     pa_context_set_state_callback (device.context, context_state_cb, &device);

     if (device.context == NULL)
     {
	  SysPrintf ("Could not acquire PulseAudio device context");
	  return;
     }

     // Connect to PulseAudio server ///////////////////////////////////////////
     if (pa_context_connect (device.context, NULL, (pa_context_flags_t) 0, NULL) < 0)
     {
	  error_number = pa_context_errno (device.context);
	  SysPrintf ("Could not connect to PulseAudio server: %s", pa_strerror(error_number));
	  return;
     }

     // Run mainloop until sever context is ready //////////////////////////////
     pa_threaded_mainloop_lock (device.mainloop);
     if (pa_threaded_mainloop_start (device.mainloop) < 0)
     {
	  SysPrintf ("Could not start mainloop");
	  return;
     }

     pa_context_state_t context_state;
     context_state = pa_context_get_state (device.context);
     while (context_state != PA_CONTEXT_READY)
     {
	  context_state = pa_context_get_state (device.context);
	  if (! PA_CONTEXT_IS_GOOD (context_state))
	  {
	       error_number = pa_context_errno (device.context);
	       SysPrintf ("Context state is not good: %s", pa_strerror (error_number));
	       return;
	  }
	  else if (context_state == PA_CONTEXT_READY)
	       break;
	  else
	       SysPrintf ("PulseAudio context state is %d", context_state);
	  pa_threaded_mainloop_wait (device.mainloop);
     }

     // Set sample spec ////////////////////////////////////////////////////////
     device.spec.format = PA_SAMPLE_S16LE;
     device.spec.channels = 1;
     device.spec.rate = settings.frequency;

     device.buffer_attributes.tlength = pa_bytes_per_second (& device.spec) / 5;
     device.buffer_attributes.maxlength = device.buffer_attributes.tlength * 3;
     device.buffer_attributes.minreq = device.buffer_attributes.tlength / 3;
     device.buffer_attributes.prebuf = device.buffer_attributes.tlength;

     //fprintf (stderr, "Total space: %u\n", device.buffer_attributes.maxlength);
     //fprintf (stderr, "Minimum request size: %u\n", device.buffer_attributes.minreq);
     //fprintf (stderr, "Bytes needed before playback: %u\n", device.buffer_attributes.prebuf);
     //fprintf (stderr, "Target buffer size: %lu\n", device.buffer_attributes.tlength);

     // Acquire new stream using spec //////////////////////////////////////////
     device.stream = pa_stream_new (device.context, "PSX4ALL", &device.spec, NULL);
     if (device.stream == NULL)
     {
	  error_number = pa_context_errno (device.context);
	  SysPrintf ("Could not acquire new PulseAudio stream: %s", pa_strerror (error_number));
	  return;
     }

     // Set callbacks for server events ////////////////////////////////////////
     pa_stream_set_state_callback (device.stream, stream_state_cb, &device);
     pa_stream_set_write_callback (device.stream, stream_request_cb, &device);
     pa_stream_set_latency_update_callback (device.stream, stream_latency_update_cb, &device);

     // Ready stream for playback //////////////////////////////////////////////
     pa_stream_flags_t flags = (pa_stream_flags_t) (PA_STREAM_ADJUST_LATENCY | PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_AUTO_TIMING_UPDATE);
     //pa_stream_flags_t flags = (pa_stream_flags_t) (PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_AUTO_TIMING_UPDATE | PA_STREAM_EARLY_REQUESTS);
     if (pa_stream_connect_playback (device.stream, NULL, &device.buffer_attributes, flags, NULL, NULL) < 0)
     {
	  pa_context_errno (device.context);
	  SysPrintf ("Could not connect for playback: %s", pa_strerror (error_number));
	  return;
     }

     // Run mainloop until stream is ready /////////////////////////////////////
     pa_stream_state_t stream_state;
     stream_state = pa_stream_get_state (device.stream);
     while (stream_state != PA_STREAM_READY)
     {
	  stream_state = pa_stream_get_state (device.stream);

	  if (stream_state == PA_STREAM_READY)
	       break;

	  else if (! PA_STREAM_IS_GOOD (stream_state))
	  {
	       error_number = pa_context_errno (device.context);
	       SysPrintf ("Stream state is not good: %s", pa_strerror (error_number));
	       return;
	  }
	  else
	       SysPrintf ("PulseAudio stream state is %d", stream_state);
	  pa_threaded_mainloop_wait (device.mainloop);
     }

     pa_threaded_mainloop_unlock (device.mainloop);

     SysPrintf ("PulseAudio should be connected");
     return;
#endif
}

////////////////////////////////////////////////////////////////////////
// REMOVE SOUND
////////////////////////////////////////////////////////////////////////
void RemoveSound (void)
{
#ifndef NOSOUND
     if (device.mainloop != NULL)
	  pa_threaded_mainloop_stop (device.mainloop);

     // Release in reverse order of acquisition
     if (device.stream != NULL)
     {
	  pa_stream_unref (device.stream);
	  device.stream = NULL;

     }
     if (device.context != NULL)
     {
	  pa_context_disconnect (device.context);
	  pa_context_unref (device.context);
	  device.context = NULL;
     }

     if (device.mainloop != NULL)
     {
	  pa_threaded_mainloop_free (device.mainloop);
	  device.mainloop = NULL;
     }

#endif
}

