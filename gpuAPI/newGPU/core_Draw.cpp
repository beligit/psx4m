#include "newGPU.h"
#include "minimal.h"

u16 bgr2rgb[65536];
#ifdef IPHONE
extern int __emulation_run;
extern int __emulation_saving;
extern int __emulation_paused;
extern int __saved;
extern int __autosave;
extern s8* packfile;
extern s8 save_filename[1024];
#endif
#ifdef PANDORA
extern "C" void convert_screen_320_240_BGR555_640_480_RGB565(void* dest, void* src, u32 dest_pitch_bytes);
#endif

///////////////////////////////////////////////////////////////////////////////
#include "raster.h"
#include "inner_Blit.h"

///////////////////////////////////////////////////////////////////////////////
void clutInit(void)
{
#if 1
	int r,g,b,i;
	
	i = 0;
	for(b = 0; b < 32; b++)
	{
		for(g = 0; g < 32; g++)
		{
			for(r = 0; r < 32; r++)
			{
				u16 pixbgr = (b<<10) | (g<<5) | (r);
				bgr2rgb[i] = ((pixbgr&(0x1f001f<<10))>>10) | ((pixbgr&(0x1f001f<<5))<<1) | ((pixbgr&(0x1f001f<<0))<<11);
				bgr2rgb[i+32768] = bgr2rgb[i];
				i++;
			}
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
void  gpuDebugRect(u16 col, u32 x0, u32 y0, u32 x1, u32 y1)
{
  if(x0>x1 || y0>y1 || x1>1024 || y1>512)
    return;

  x0 = (x0* PSX4ALL_WIDTH)/1024;  x1 = (x1*PSX4ALL_WIDTH )/1024;
  y0 = (y0*PSX4ALL_HEIGHT)/ 512;  y1 = (y1*PSX4ALL_HEIGHT)/ 512;

  u16* dest_screen16 = gp2x_screen16;
  dest_screen16 += (y0*PSX4ALL_WIDTH);

  // top
  if(y0!=y1)
  {
    for(u32 x=x0;x<x1;++x)  dest_screen16[x] = col;
    dest_screen16 += PSX4ALL_WIDTH;
  }
  
  //  middle
  if(x1)  --x1;
  while(++y0<y1)
  {
    dest_screen16[x0] = dest_screen16[x1] = col;
    dest_screen16 += PSX4ALL_WIDTH;
  }
  ++x1;

  // bottom
  {
    for(u32 x=x0;x<x1;++x)  dest_screen16[x] = col;
  }
}
///////////////////////////////////////////////////////////////////////////////
void  gpuDisplayVideoMem()
{
#if !defined(PANDORA) && !defined(IPHONE)
	u16* dest_screen16 = gp2x_screen16;
  for(int y=0;y<240;++y)
  {
  	u16* src_screen16  = &((u16*)GPU_FrameBuffer)[FRAME_OFFSET(0,(y<<9)/PSX4ALL_HEIGHT)];
    GPU_BlitWSSWSSWSSWSSWSSS(	src_screen16, dest_screen16, false);
    dest_screen16 += PSX4ALL_WIDTH;
  }

  gpuDebugRect(0x00FF, DrawingArea[0],    DrawingArea[1],   DrawingArea[2],   DrawingArea[3]);
  gpuDebugRect(0xFF00, DisplayArea[0],    DisplayArea[1],   DisplayArea[0]+DisplayArea[2], DisplayArea[1]+DisplayArea[3]);
  gpuDebugRect(0x0FF0, DirtyArea[0],      DirtyArea[1],     DirtyArea[2],     DirtyArea[3]);
  gpuDebugRect(0xFFFF, LastDirtyArea[0],  LastDirtyArea[1], LastDirtyArea[2], LastDirtyArea[3]);

  gp2x_video_flip();
#endif
}

///////////////////////////////////////////////////////////////////////////////
void gpuVideoOutput(void)
{
	static s16 old_res_horz, old_res_vert, old_rgb24;
	s16 h0, x0, y0, w0, h1;

	x0 = DisplayArea[0];
	y0 = DisplayArea[1];

	w0 = DisplayArea[2];
	h0 = DisplayArea[3];  // video mode

	h1 = DisplayArea[7] - DisplayArea[5]; // display needed
	if (h0 == 480)
		h1 = Min2(h1*2,480);

#ifdef ZAURUS
	if(SDL_MUSTLOCK(gp2x_sdlwrapper_screen)) SDL_LockSurface(gp2x_sdlwrapper_screen);
#endif
	u32 isRGB24 = (GPU_GP1 & 0x00200000 ? 32 : 0);

	/* Clear the screen if resolution changed to prevent interlacing and clipping to clash */
	if( (w0 != old_res_horz || h1 != old_res_vert || (s16)isRGB24 != old_rgb24) )
	{
		// Update old resolution
		old_res_horz = w0;
		old_res_vert = h1;
		old_rgb24 = (s16)isRGB24;
		
#ifdef PANDORA
#if defined(WITH_SDL_VIDEO)
    if(!isRGB24)
    {
      gp2x_change_res(w0, h1);
    }
    else
    {
      gp2x_change_res(800, 480);
      gp2x_video_RGB_clearscreen16();
    }
#else
    gp2x_change_res(w0, h1);
#endif
#else
		// Finally, clear the screen for this special case
		gp2x_video_RGB_clearscreen16();
#endif
	}

	u16* dest_screen16 = gp2x_screen16;
	u16* src_screen16  = &((u16*)GPU_FrameBuffer)[FRAME_OFFSET(x0,y0)];

#if defined(PANDORA) || defined(IPHONE)
#if !defined(PANDORA)
  if(h0==256)
  {
  	h0 = 240;
  	src_screen16  += ((h1-h0)>>1)*1024;
  	h1 = h0;
  }
#endif
#else
	//  Height centering
  int sizeShift = 1;
  if(h0==256)
    h0 = 240;
  else
  if(h0==480)
    sizeShift = 2;

  if(h1>h0)
  {
		src_screen16  += ((h1-h0)>>sizeShift)*1024;
    h1 = h0;
  }
  else
  if(h1<h0)
    dest_screen16 += ((h0-h1)>>sizeShift)*PSX4ALL_WIDTH;

	/* Main blitter */
  int incY = (h0==480) ? 2 : 1;
#endif

#if defined(PANDORA)
if(!isRGB24)
{
  u32* dest_screen32 = (u32*)gp2x_screen16;
  u32* src_screen32  = (u32*)(&((u16*)GPU_FrameBuffer)[FRAME_OFFSET(x0,y0)]);
	
  int x, y;
  for(y = 0; y < h1; y++)
  {
    for(x = 0; x < w0>>1; x++)
    {
      const u32 pixbgr = src_screen32[x];
      *dest_screen32++ = ((pixbgr&(0x1f001f<<10))>>10) | ((pixbgr&(0x1f001f<<5))<<1) | ((pixbgr&(0x1f001f<<0))<<11);
    }
    src_screen32 += 512;
  }
}
else
{
  u32* dest_screen32 = (u32*)gp2x_screen16;
	u8* src_screen8  = (u8*)(&((u16*)GPU_FrameBuffer)[FRAME_OFFSET(x0,y0)]);
	
  int x, y;
  for(y = 0; y < h1; y++)
  {
    for(x = 0; x < w0 * 3; x += 6)
    {
      *dest_screen32++ = ((u32)gp2x_video_RGB_color16( src_screen8[x+3], src_screen8[x+4], src_screen8[x+5] ) << 16) | gp2x_video_RGB_color16( src_screen8[x+0], src_screen8[x+1], src_screen8[x+2] );
    }
    src_screen8 += 2048;
  }  
}
#else
{
	void (*GPU_Blitter)(void* src, u16* dst16, u32 height);

	switch( w0 )
	{
	case 256:
			if(h1 <= 240)
			{
				GPU_Blitter = GPU_Blit_256_240_ROT90CW_2X;
			}
			else
			{
				GPU_Blitter = GPU_Blit_256_480_ROT90CW_2X;
			}
		break;
	case 368:
			if(h1 <= 240) 
			{
				GPU_Blitter = GPU_Blit_368_240_ROT90CW_2X;
			}
			else
			{
				GPU_Blitter = GPU_Blit_368_480_ROT90CW_2X;
			}
		break;
	case 320:
			if(h1 <= 240) 
			{
#ifdef PANDORA
			  if(!isRGB24)
			  {
          convert_screen_320_240_BGR555_640_480_RGB565(dest_screen16, src_screen16, PSX4ALL_PITCH);
          return;
        }
#endif
				GPU_Blitter = GPU_Blit_320_240_ROT90CW_2X;
			}
			else
			{
				GPU_Blitter = GPU_Blit_320_480_ROT90CW_2X;
			}
		break;
	case 384:
			if(h1 <= 240)
			{
				GPU_Blitter = GPU_Blit_384_240_ROT90CW_2X;
			}
			else
			{
				GPU_Blitter = GPU_Blit_384_480_ROT90CW_2X;
			}
		break;
	case 512:
			if(h1 <= 240)
			{
				GPU_Blitter = GPU_Blit_512_240_ROT90CW_2X;
			}
			else
			{
				GPU_Blitter = GPU_Blit_512_480_ROT90CW_2X;
			}
		break;
	case 640:
			GPU_Blitter = GPU_Blit_640_480_ROT90CW;
		break;
	default: return;
	}
  /* Blit span */
	GPU_Blitter(src_screen16,	dest_screen16, h1);
}
#endif

#if 0
//y0 -= framesProgresiveInt ? incY : 0;
  for(int y1=y0+h1; y0<y1; y0+=incY)
	{
		/* Blit span */
		switch( w0 )
		{
		case 256:
			if( 0 == (y0&linesInterlace) )
				GPU_BlitWWDWW(	src_screen16,	dest_screen16, isRGB24);
			break;
		case 368:
			if( 0 == (y0&linesInterlace) )
				GPU_BlitWWWWWWWWS(	src_screen16,	dest_screen16, isRGB24, 4);
			break;
		case 320:
			if( 0 == (y0&linesInterlace) )
				GPU_BlitWW(	src_screen16,	dest_screen16, isRGB24);
			break;
		case 384:
			if( 0 == (y0&linesInterlace) )
				GPU_BlitWWWWWS(	src_screen16,	dest_screen16, isRGB24);
			break;
		case 512:
			if( 0 == (y0&linesInterlace) )
				GPU_BlitWS(	src_screen16, dest_screen16, isRGB24);
			break;
		case 640:
			if( 0 == (y0&linesInterlace) )
				GPU_BlitWS(	src_screen16, dest_screen16, isRGB24);
			break;
		}

		dest_screen16 += PSX4ALL_WIDTH;
		src_screen16  += h0==480 ? 2048 : 1024;
	}
#endif

#ifdef ZAURUS
	if(SDL_MUSTLOCK(gp2x_sdlwrapper_screen)) SDL_UnlockSurface(gp2x_sdlwrapper_screen);
#endif

/* DEBUG
gp2x_printf(NULL, 0, 10, "WIDTH %d HEIGHT %d", w0, h1 );
*/
}

int curDelay     = 0 ;
int curDelay_inc = gp2x_timer_raw_second()/1000;
int skCount = 0;
int skRate  = 0;

///////////////////////////////////////////////////////////////////////////////
void  gpuSkipUpdate()
{
  ++frameRateCounter;

  isNewDisplay = false;

  static u32 s_LastFrame=0;
  u32 curFlip  = gp2x_timer_raw();
  u32 curFrame = curFlip-s_LastFrame;

  if(!isSkip)
  {

    ++frameRealCounter;
  	gpuVideoOutput();

#if !defined(PANDORA) && !defined(IPHONE)
    if(DisplayArea[3] == 480)
    {
      linesInterlace = 1;
    }
    else
#endif


    if( linesInterlace != linesInterlace_user )
    {
      linesInterlace = linesInterlace_user;
      gp2x_video_RGB_clearscreen16();
    }
  }
  curFlip  = gp2x_timer_raw()-curFlip;

  if(displayFrameInfo && (!isSkip))
  {
    int ypos = 0;
    gp2x_printf(NULL, 0, ypos,"VS:%04.4g fps:NA real:NA fs(%d/%d) (%3d,%2d,%2d)ms", float(vsincRate)/100.0f, skipCount, skipRate, gp2x_timer_raw_to_ticks(curFrame),gp2x_timer_raw_to_ticks(curFlip),gp2x_timer_raw_to_ticks(curDelay));
    //gp2x_printf(NULL, 0, ypos,"VS:%04.4g fps:%04.4g real:%04.4g fs(%d/%d) (%3d,%2d,%2d)ms", float(vsincRate)/100.0f, float(frameRate)/100.0f, float(realRate)/100.0f, skipCount, skipRate, gp2x_timer_raw_to_ticks(curFrame),gp2x_timer_raw_to_ticks(curFlip),gp2x_timer_raw_to_ticks(curDelay));
    //gp2x_printf(NULL, 0, ypos+=10,"W:%u H:%u H1:%u (%u-%u)", DisplayArea[2],DisplayArea[3],DisplayArea[7]-DisplayArea[5]);
  #ifdef ENABLE_GPU_PRIM_STATS
    int polis = statF3 + statFT3 + statG3 + statGT3;
    gp2x_printf(NULL, 0,(ypos+=10),"PPF (%4d): PPS (%5d): ", polis,  (polis*realRate)/100 );
    gp2x_printf(NULL, 0,(ypos+=10),"types F(%4d) FT(%4d) G(%4d)  GT(%4d)", statF3, statFT3, statG3, statGT3);
  #endif
  #ifdef ENABLE_GPU_PROFILLER
    if(displayGpuStats)
    {
      gp2x_printf(NULL, 0,(ypos+=10),"dmaChainTime (%5d): %04.4g%% (%3dms)", dmaChainCount,  PROFILE_RATIO((100*dmaChainTime ),curFrame), gp2x_timer_raw_to_ticks(dmaChainTime) );
      gp2x_printf(NULL, 0,(ypos+=10),"gpuPolyTime  (%5d): %04.4g%% (%3dms)", gpuPolyCount,   PROFILE_RATIO((100*gpuPolyTime  ),curFrame), gp2x_timer_raw_to_ticks(gpuPolyTime)  );
      gp2x_printf(NULL, 0,(ypos+=10),"gpuPixelTime (%5d): %04.4g%% (%3dms)", gpuPixelCount,  PROFILE_RATIO((100*gpuPixelTime ),curFrame), gp2x_timer_raw_to_ticks(gpuPixelTime) );
      gp2x_printf(NULL, 0,(ypos+=10),"gpuRasterTime(%5d): %04.4g%% (%3dms)", gpuRasterCount, PROFILE_RATIO((100*gpuRasterTime),curFrame), gp2x_timer_raw_to_ticks(gpuRasterTime));
      gp2x_printf(NULL, 0,(ypos+=10),"dmaMemTime   (%5d): %04.4g%% (%3dms)", dmaMemCount,    PROFILE_RATIO((100*dmaMemTime),   curFrame), gp2x_timer_raw_to_ticks(dmaMemTime)   );
    }
  #endif
  }
  PROFILE_RESET(gpuPolyTime,gpuPolyCount);
  PROFILE_RESET(gpuRasterTime,gpuRasterCount);
  PROFILE_RESET(gpuPixelTime,gpuPixelCount);
  PROFILE_RESET(dmaChainTime,dmaChainCount);
  PROFILE_RESET(dmaMemTime,dmaMemCount);
  statF3 = statFT3 = statG3 = statGT3 = 0;
  statLF = statLG  = statS  = statT   = 0;

  if(!isSkip)
    gp2x_video_flip();

  s_LastFrame = gp2x_timer_raw();

  if(skCount-->0)
  {
    isSkip = 1;
  }
  else
  {
    isSkip = 0;
  }

  if(--skRate<=0)
  {
    skCount = Min2(skipCount,skipRate?skipRate-1:0);
    skRate  = skipRate;
  }
}

///////////////////////////////////////////////////////////////////////////////
void  NEWGPU_vSinc(void)
{
	u32    newtime;
	u32    diffintime = 0;

#ifdef IPHONE
  static u32 frame_ticks = 0;
  
	if(__autosave)
	{
    frame_ticks++;
  	if( frame_ticks >= 18000 )
  	{
  		char filename[1024];
  		
  		sprintf(filename, "%s-last-autosave.svs", packfile);
      gp2x_printf(NULL, 0, 20, "autosaving");
  		SaveState(filename);

  		frame_ticks = 0;
  	}
  }
  
  do 
  {
		if( !__emulation_run || __emulation_saving ) 
		{
			char buffer[1024];
			char filename[1024];
			time_t curtime;
			struct tm *loctime;
		

			if(__emulation_saving)
			{
				if(__saved != 0 && __emulation_saving == 2)
				{
					sprintf(filename, "%s", save_filename);
				}
				else
				{
					curtime = time (NULL);
					loctime = localtime (&curtime);
					strftime (buffer, 1024, "%y%m%d-%I%M%S", loctime);
					sprintf(filename, "%s-%s.svs", packfile, buffer);
				}
				SaveState(filename);
				__emulation_saving = 0;
			}
			
			if( !__emulation_run )
			{
        SysClose();
			}
		}
		if(__emulation_paused)
		{
      usleep(100000);
		}
  } while(__emulation_paused);
#endif

	/*	NOTE: GP1 must have the interlace bit toggle here,
		since it shouldn't be in readStatus as it was previously */
  GPU_GP1 ^= 0x80000000;

  if ( (GPU_GP1&0x08000000) ) // dma transfer NO update posible...
  		return;

  if ( (GPU_GP1&0x00800000) ) // Display disabled
  		return;

  ++vsincRateCounter;

  gpuSkipUpdate();

  if(curDelay>0)
    gp2x_timer_delay(curDelay);

  newtime = gp2x_timer_raw();
	if( (diffintime=newtime-systime) >= (gp2x_timer_raw_second()) ) // poll 2 times per second
	{
    vsincRate = (u64)(vsincRateCounter*100)*gp2x_timer_raw_second() / diffintime;
    frameRate = (u64)(frameRateCounter*100)*gp2x_timer_raw_second() / diffintime;
    realRate  = (u64)(frameRealCounter*100)*gp2x_timer_raw_second() / diffintime;

    if(enableFrameLimit && frameRate)
    {
      int inc    = gp2x_timer_raw_second() > 1000 ? gp2x_timer_raw_second()/5000 : gp2x_timer_raw_second()/1000;
      int target =(isPAL==0 ? 60 : 50) * 100;
      int range  = target*5/100;

      if(vsincRate>(target+range))
      {
        curDelay_inc = (curDelay_inc>0) ? (curDelay_inc+inc) : (-curDelay_inc);
        curDelay    += curDelay_inc;
      }
      else
      if(vsincRate<(target-range))
      {
        curDelay_inc = (curDelay_inc<0) ? (curDelay_inc-inc) : (-curDelay_inc);
        curDelay    += curDelay_inc;
      }
    }
    vsincRateCounter = 0;
		frameRateCounter = 0;
		frameRealCounter = 0;
		systime = gp2x_timer_raw();
	}
}
