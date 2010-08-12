#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PsxCommon.h"

#include "Spu.h"

extern int  skipCount; // Def: 0
extern int  skipRate; // Def: 1

int OpenPlugins(void)
{
	int ret;
#if defined(PSP) && defined(PSP_GPU)
  GPU_SelectPluggin(2);
#elif defined(MAEMO_CHANGES) && defined(PEOPS_GPU)
  GPU_SelectPluggin(4);
#elif defined(PANDORA)
  GPU_SelectPluggin(0); // 3
#else
  GPU_SelectPluggin(0);
#endif

	ret = CDR_init();
	if (ret < 0) { SysPrintf("CDRinit error : %d", ret); return -1; }
	ret = GPU_init();
	if (ret < 0) { SysPrintf("GPUinit error: %d", ret); return -1; }
	ret = SPU_init();
	if (ret < 0) { SysPrintf("SPUinit error: %d", ret); return -1; }
	ret = PAD1_init(1);
	if (ret < 0) { SysPrintf("PAD1init error: %d", ret); return -1; }
	ret = PAD2_init(2);
	if (ret < 0) { SysPrintf("PAD2init error: %d", ret); return -1; }

	ret = CDR_open();
	if (ret < 0) { SysPrintf("Error Opening CDR Component"); return -1; }
	ret = SPU_open();
	if (ret < 0) { SysPrintf("Error Opening SPU Component"); return -1; }
	// EDIT SPU_registerCallback(SPUirq);
  //ret = GPU_open(0);
	//if (ret < 0) { SysPrintf("Error Opening GPU Component"); return -1; }
	ret = PAD1_open();
	if (ret < 0) { SysPrintf("Error Opening PAD1 Component"); return -1; }
	ret = PAD2_open();
	if (ret < 0) { SysPrintf("Error Opening PAD2 Component"); return -1; }

	return 0;
}

void ClosePlugins() {
	s32 ret;

	ret = CDR_close();
	if (ret < 0) { SysMessage("Error Closing CDR Plugin"); }
	ret = SPU_close();
	if (ret < 0) { SysMessage("Error Closing SPU Plugin"); }
	ret = PAD1_close();
	if (ret < 0) { SysMessage("Error Closing PAD1 Plugin"); }
	ret = PAD2_close();
	if (ret < 0) { SysMessage("Error Closing PAD2 Plugin"); }
	//ret = GPU_close();
	//if (ret < 0) { SysMessage("Error Closing GPU Plugin"); }

	CDR_shutdown();
	GPU_done();
//GPU_shutdown();
	SPU_shutdown();
	PAD1_shutdown();
	PAD2_shutdown();
}

static u8 buf[256];
u8 stdpar[10] = { 0x00, 0x41, 0x5a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
u8 mousepar[8] = { 0x00, 0x12, 0x5a, 0xff, 0xff, 0xff, 0xff };
u8 analogpar[9] = { 0x00, 0xff, 0x5a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static s32 bufcount, bufc;

PadDataS padd1, padd2;

#define PSE_PAD_TYPE_MOUSE 1
#define PSE_PAD_TYPE_NEGCON 2
#define PSE_PAD_TYPE_ANALOGPAD 7
#define PSE_PAD_TYPE_ANALOGJOY 5
#define PSE_PAD_TYPE_STANDARD 4

u8 _PADstartPoll(PadDataS *pad) {
	bufc = 0;

	switch (pad->controllerType) {
		case PSE_PAD_TYPE_MOUSE:
			mousepar[3] = pad->buttonStatus & 0xff;
			mousepar[4] = pad->buttonStatus >> 8;
			mousepar[5] = pad->moveX;
			mousepar[6] = pad->moveY;

			memcpy(buf, mousepar, 7);
			bufcount = 6;
			break;
		case PSE_PAD_TYPE_NEGCON: // npc101/npc104(slph00001/slph00069)
			analogpar[1] = 0x23;
			analogpar[3] = pad->buttonStatus & 0xff;
			analogpar[4] = pad->buttonStatus >> 8;
			analogpar[5] = pad->rightJoyX;
			analogpar[6] = pad->rightJoyY;
			analogpar[7] = pad->leftJoyX;
			analogpar[8] = pad->leftJoyY;

			memcpy(buf, analogpar, 9);
			bufcount = 8;
			break;
		case PSE_PAD_TYPE_ANALOGPAD: // scph1150
			analogpar[1] = 0x73;
			analogpar[3] = pad->buttonStatus & 0xff;
			analogpar[4] = pad->buttonStatus >> 8;
			analogpar[5] = pad->rightJoyX;
			analogpar[6] = pad->rightJoyY;
			analogpar[7] = pad->leftJoyX;
			analogpar[8] = pad->leftJoyY;

			memcpy(buf, analogpar, 9);
			bufcount = 8;
			break;
		case PSE_PAD_TYPE_ANALOGJOY: // scph1110
			analogpar[1] = 0x53;
			analogpar[3] = pad->buttonStatus & 0xff;
			analogpar[4] = pad->buttonStatus >> 8;
			analogpar[5] = pad->rightJoyX;
			analogpar[6] = pad->rightJoyY;
			analogpar[7] = pad->leftJoyX;
			analogpar[8] = pad->leftJoyY;

			memcpy(buf, analogpar, 9);
			bufcount = 8;
			break;
		case PSE_PAD_TYPE_STANDARD:
		default:
			stdpar[3] = pad->buttonStatus & 0xff;
			stdpar[4] = pad->buttonStatus >> 8;

			memcpy(buf, stdpar, 5);
			bufcount = 4;
	}

	return buf[bufc++];
}

u8 _PADpoll(u8 value) {
	if (bufc > bufcount) return 0xFF;
	return buf[bufc++];
}

u8 PAD1_startPoll(s32 pad) {
	PadDataS padd;

	PAD1_readPort1(&padd);

	return _PADstartPoll(&padd);
}

u8 PAD1_poll(u8 value) {
	return _PADpoll(value);
}

u8 PAD2_startPoll(s32 pad) {
	PadDataS padd;

	PAD2_readPort2(&padd);

	return _PADstartPoll(&padd);
}

u8 PAD2_poll(u8 value) {
	return 0xFF; //_PADpoll(value);
}

s32  PadFlags = 0;

s32 PAD1_init(s32 flags) {
	PadFlags |= flags;
	
	return 0;
}

s32 PAD2_init(s32 flags) {
	PadFlags |= flags;
	
	return 0;
}

s32 PAD1_shutdown(void) {
	return 0;
}


s32 PAD2_shutdown(void) {
	return 0;
}


s32 PAD1_open(void)
{
	return 0;
}


s32 PAD2_open(void)
{
	return 0;
}


s32 PAD1_close(void) {
	return 0;
}


s32 PAD2_close(void) {
	return 0;
}

u16 ReadZodKeys()
{
	u16 pad_status = 0xffff;
	u32 keys = gp2x_joystick_read();

#if defined(PANDORA)
  if( (keys & GP2X_L) && (keys & GP2X_R) && (keys & GP2X_X) && (keys & GP2X_Y) && (keys & GP2X_A) && (keys & GP2X_B) )
  {
    SysClose();
  }
#endif

  if(	((keys & GP2X_LEFT) || (keys & GP2X_RIGHT)) && (keys & GP2X_L) && (keys & GP2X_VOL_DOWN) )
	{
    if(keys & GP2X_LEFT)
    {
      if(skipCount > 0)
      {
        skipCount--;
        usleep(1000000);
      }
    }
    else if(keys & GP2X_RIGHT)
    {
      if(skipCount < skipRate - 1)
      {
        skipCount++;  
        usleep(1000000);      
      }
    }
	}
	else
	{
	  if(	keys & GP2X_VOL_DOWN ) // L2
  	{
  		pad_status &= ~(1<<8);
  	}
  	if (keys & GP2X_L)
  	{
  		pad_status &= ~(1<<10); // L ?
  	}
  }
  
  if(	((keys & GP2X_LEFT) || (keys & GP2X_RIGHT)) && (keys & GP2X_R) && (keys & GP2X_VOL_UP) )
	{
    if(keys & GP2X_LEFT)
    {
      if(skipRate > 1)
      {
        skipRate--;
        usleep(1000000);
      }
    }
    else if(keys & GP2X_RIGHT)
    {
      skipRate++;
      usleep(1000000);
    }
	}	
	else 
	{
  	if( keys & GP2X_VOL_UP ) // R2
  	{
  		pad_status &= ~(1<<9);
  	}
  	if (keys & GP2X_R)
  	{
  		pad_status &= ~(1<<11); // R ?
  	}
  }
  
	if (keys & GP2X_UP)
	{
		pad_status &= ~(1<<4); 
	}
	if (keys & GP2X_DOWN)
	{
		pad_status &= ~(1<<6);
	}
	if (keys & GP2X_LEFT)
	{
		pad_status &= ~(1<<7);
	}
	if (keys & GP2X_RIGHT)
	{
		pad_status &= ~(1<<5);
	}
	if (keys & GP2X_START)
	{
		pad_status &= ~(1<<3);
	}
	if (keys & GP2X_SELECT)
	{			
		pad_status &= ~(1);
	}
		
	if (keys & GP2X_X)
	{
		pad_status &= ~(1<<14);
	}
	if (keys & GP2X_B)
	{
		pad_status &= ~(1<<13);
	}
	if (keys & GP2X_A)
	{
		pad_status &= ~(1<<15);
	}
	if (keys & GP2X_Y)
	{
		pad_status &= ~(1<<12);
	}

	return pad_status;
}

s32 PAD1_readPort1(PadDataS* pad) {
	u16 pad_status = 0xffff;
	//SysPrintf("start PAD1_readPort()\r\n");
	pad_status = ReadZodKeys();
	pad->buttonStatus = pad_status;
	pad->controllerType = 4; // standard	
  	return 0;
}

s32 PAD2_readPort1(PadDataS* pad) {
	u16 pad_status = 0xffff;
	//SysPrintf("start PAD1_readPort()\r\n");
	pad_status = 0xffff;
	pad->buttonStatus = pad_status;
	pad->controllerType = 4; // standard
 	return 0;
}

s32 PAD1_readPort2(PadDataS* a) {
	return -1;
}

s32 PAD2_readPort2(PadDataS* a) {
	return -1;
}

