///////////////////////////////////////////////////////////////////////////////
//  GPU inicialization/deinicialization functions
#include "newGPU.h"
#include "fixed.h"

#if defined(PSP) || defined(DREAMCAST)
#include "string.h"
#else
// EDIT: Not found and not needed: #include "memory.h"
#endif

///////////////////////////////////////////////////////////////////////////////
//#define	GetMem(p1)	malloc((p1))
//#define	FreeMem(p1)	free((p1))
#define CopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
//#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define	CopyMem(p1,p2,p3)	CopyMemory((p1),(p2),(p3))
//#define	ZeroMem(p1,p2)	ZeroMemory((p1),(p2))


///////////////////////////////////////////////////////////////////////////////
void gpuReset(void)
{
	//ZeroMem(&gpuFreezeBegin,
	//		(long) &gpuFreezeEnd - (long) &gpuFreezeBegin);
	GPU_GP1 = 0x14802000;
	TextureWindow[0] = 0;
	TextureWindow[1] = 0;
	TextureWindow[2] = 255;
	TextureWindow[3] = 255;
	DrawingArea[2] = 256;
	DrawingArea[3] = 240;
	DisplayArea[2] = 256;
	DisplayArea[3] = 240;
	DisplayArea[6] = 256;
	DisplayArea[7] = 240;
	skipNum = 1;
}

///////////////////////////////////////////////////////////////////////////////
BOOL  NEWGPU_init(void)
{
  gp2x_video_RGB_clearscreen16();
	clutInit();
	gpuReset();
  xInit();
	return (0);
}

///////////////////////////////////////////////////////////////////////////////
void  NEWGPU_done(void)
{
}


///////////////////////////////////////////////////////////////////////////////
BOOL  NEWGPU_freeze(BOOL bWrite, GPUFreeze* p2)
{
//u32 temp;
  /*
	if (p1 == 2) {
		temp = *(u32 *) p2;
		if ((temp < 0) || (temp > 8))
			return (0);
		return (1);
	}
  */
	if (!p2)
		return (0);

  if (p2->Version != 1)
		return (0);

  if (bWrite)
  {
		p2->GP1 = GPU_GP1;
		//CopyMem(p2->Control, &gpuFreezeBegin,
		//		(long) &gpuFreezeEnd - (long) &gpuFreezeBegin);
		CopyMem(p2->FrameBuffer, (u16*)GPU_FrameBuffer, FRAME_BUFFER_SIZE);
		p2->isPAL = isPAL;
		p2->isDisplaySet = isDisplaySet;
		CopyMem(p2->DisplayArea, DisplayArea, 8*4);
		CopyMem(p2->DirtyArea, DirtyArea, 4*4);
		CopyMem(p2->LastDirtyArea, LastDirtyArea, 4*4);
		CopyMem(p2->CheckArea, CheckArea, 4*4);

		CopyMem(p2->TextureWindow, TextureWindow, 4*4);
		CopyMem(p2->DrawingArea, DrawingArea, 4*4);
		CopyMem(p2->DrawingOffset, DrawingOffset, 2*4);
		p2->MaskU = MaskU;
		p2->MaskV = MaskV;
		p2->Masking = Masking;
		p2->PixelMSB = PixelMSB;
		return (1);
	}
  else
	{
		GPU_GP1 = p2->GP1;
		//CopyMem(&gpuFreezeBegin, p2->Control,
		//		(long) &gpuFreezeEnd - (long) &gpuFreezeBegin);
		CopyMem((u16*)GPU_FrameBuffer, p2->FrameBuffer, FRAME_BUFFER_SIZE);
		isPAL = p2->isPAL;
		isDisplaySet = p2->isDisplaySet;
		CopyMem(DisplayArea, p2->DisplayArea, 8*4);
		CopyMem(DirtyArea, p2->DirtyArea, 4*4);
		CopyMem(LastDirtyArea, p2->LastDirtyArea, 4*4);
		CopyMem(CheckArea, p2->CheckArea, 4*4);
		
		CopyMem(TextureWindow, p2->TextureWindow, 4*4);
		CopyMem(DrawingArea, p2->DrawingArea, 4*4);
		CopyMem(DrawingOffset, p2->DrawingOffset, 2*4);
		MaskU = p2->MaskU;
		MaskV = p2->MaskV;
		Masking = p2->Masking;
		PixelMSB = p2->PixelMSB;
		return (1);
	}
	return (0);
}


