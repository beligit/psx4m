//#define NO_ZERO_REGISTER_OPTIMISATION	1

#include "../../PsxCommon.h"
#include <stdio.h> // For mprotect
#include <sys/mman.h> // For mprotect PROT_x defines
#include <sys/errno.h> // For mprotect PROT_x defines

extern u32 *recMem;

#include "recompiler.h"

#include "arm_std_rec_calls.c"
#include "arm_std_rec_globals.c"
#include "arm_std_rec_debug.c"
#include "arm_std_rec_regcache.c"

#include "../../evaluator/evaluator.cpp.h"
#include "../../generator/arm/generator.cpp.h"

static void recReset();
static u32 recRecompile();
static void recClear(u32 Addr, u32 Size);

extern void (*recBSC[64])();
extern void (*recSPC[64])();
extern void (*recREG[32])();
extern void (*recCP0[32])();
extern void (*recCP2[64])();
extern void (*recCP2BSC[32])();

psxRegisters 	recRegs;
u8*		current_translation_ptr;
u32 	opcode;
u32* 	recMemStart;
u32		isInBios = 0;
u32 	loadedpermregs = 0;
u32		end_block = 0;

#ifdef WITH_DISASM

#define make_stub_label(name)							\
 { (void *)name, #name }                                                	\

disasm_label stub_labels[] =
{
  make_stub_label(gteMFC2),
  make_stub_label(gteMTC2),
  make_stub_label(gteLWC2),
  make_stub_label(gteSWC2),
  make_stub_label(gteRTPS),
  make_stub_label(gteOP),
  make_stub_label(gteNCLIP),
  make_stub_label(gteDPCS),
  make_stub_label(gteINTPL),
  make_stub_label(gteMVMVA),
  make_stub_label(gteNCDS),
  make_stub_label(gteNCDT),
  make_stub_label(gteCDP),
  make_stub_label(gteNCCS),
  make_stub_label(gteCC),
  make_stub_label(gteNCS),
  make_stub_label(gteNCT),
  make_stub_label(gteSQR),
  make_stub_label(gteDCPL),
  make_stub_label(gteDPCT),
  make_stub_label(gteAVSZ3),
  make_stub_label(gteAVSZ4),
  make_stub_label(gteRTPT),
  make_stub_label(gteGPF),
  make_stub_label(gteGPL),
  make_stub_label(gteNCCT),
  make_stub_label(psxMemRead8),
  make_stub_label(psxMemReadS8),
  make_stub_label(psxMemRead16),
  make_stub_label(psxMemReadS16),
  make_stub_label(psxMemRead32),
  make_stub_label(psxMemWrite8),
  make_stub_label(psxMemWrite16),
  make_stub_label(psxMemWrite32),
  make_stub_label(psxException),
  make_stub_label(psxBranchTest_rec)
};

const u32 num_stub_labels = sizeof(stub_labels) / sizeof(disasm_label);

#define DISASM_INIT								\
  	/*translation_log_fp = fopen("/Applications/psx4iphone.app/translation_log.txt", "a+"); */    	\
  	fprintf(stderr, "Block PC %x (MIPS) -> %p\n", pc, recMemStart);                 \

#define DISASM_MIPS								\
		disasm_mips_instruction(psxRegs->code,disasm_buffer,pc);	\
    		fprintf(stderr, "%08x: %08x %s\n", pc, 		\
			psxRegs->code, disasm_buffer);   			\

#define DISASM_ARM								\
	fprintf(stderr, "\n");                                      \
  	for(	current_translation_ptr = (u8*)recMemStart;            	    	\
   		(u32)current_translation_ptr < (u32)recMem; 			\
		current_translation_ptr += 4)  					\
  	{                                                                       \
    		opcode = *(u32*)current_translation_ptr;			\
		disasm_arm_instruction(opcode, disasm_buffer,                   \
     			(u32)current_translation_ptr, stub_labels,		\
			num_stub_labels);     			        	\
    		fprintf(stderr, "%08x: %s\n", 			\
			current_translation_ptr, disasm_buffer);	        \
  	}                                                                       \
                                                                              	\
  	fprintf(stderr, "\n");                                      \
 	/*fflush(stderr);*/                                             \
	/*gp2x_sync();*/								\
  	/*fclose(translation_log_fp);*/						\

#else

#define DISASM_MIPS								\

#define DISASM_ARM								\

#define DISASM_INIT								\

#endif

static u32 arm_disect_imm_32bit(u32 imm, u32 *stores, u32 *rotations)
{
  u32 store_count = 0;
  u32 left_shift = 0;
  u32 end_shift;
  u32 i;

  // Otherwise it'll return 0 things to store because it'll never
  // find anything.
  if(imm == 0)
  {
    rotations[0] = 0;
    stores[0] = 0;
    return 1;
  }

  if(imm & 0xF0000000 && imm & 0xF000000F)
  {
		stores[0] = ((imm & 0xF0000000)>>28) | ((imm & 0x0000000F)<<4);
		rotations[0] = 4;
		left_shift += 4;
		store_count++;
		end_shift = 28;
  }
  else 
  {
		end_shift = 32;
  }

  // Find chunks of non-zero data at 2 bit alignments.
  while(1)
  {
    for(; left_shift < end_shift; left_shift += 2)
    {
      if((imm >> left_shift) & 0x03)
        break;
    }

    if(left_shift == end_shift)
    {
      // We've hit the end of the useful data.
      return store_count;
    }

    // Hit the end, it might wrap back around to the beginning.
    if(left_shift >= (end_shift-8))
    {
      // There's nothing to wrap over to in the beginning
      stores[store_count] = (imm >> left_shift) & 0xFF;
      rotations[store_count] = (32 - left_shift) & 0x1F;
      return store_count + 1;
    }

    stores[store_count] = (imm >> left_shift) & 0xFF;
    rotations[store_count] = (32 - left_shift) & 0x1F;

    store_count++;
    left_shift += 8;
  }
}

#define REC_FUNC_TEST(f)							\
extern void psx##f(); 								\
void rec##f() 									\
{ 										\
	regClearJump();								\
	LoadImmediate32(pc, TEMP_1); 						\
	ARM_STR_IMM(ARM_POINTER, TEMP_1, PERM_REG_1, 648); 			\
	LoadImmediate32(psxRegs->code, TEMP_1); 				\
	ARM_STR_IMM(ARM_POINTER, TEMP_1, PERM_REG_1, 652); 			\
	CALLFunc((u32)psx##f); 							\
}										\

#include "rec_lsu.cpp.h" // Load Store Unit
#include "rec_gte.cpp.h" // Geometry Transformation Engine
#include "rec_alu.cpp.h" // Arithmetic Logical Unit
#include "rec_mdu.cpp.h" // Multiple Divide Unit
#include "rec_cp0.cpp.h" // Coprocessor 0
#include "rec_bcu.cpp.h" // Branch Control Unit

#define rec_recompile_start() 							\
{								              	\
		if( loadedpermregs == 0 ) 					\
		{ 								\
			LoadImmediate32((u32)psxRegs, PERM_REG_1); 		\
			loadedpermregs = 1; 					\
		} 							        \
}										\

void rec_flush_cache()
{
	clear_insn_cache((u32)recMemBase, ((u32)recMemBase) + RECMEM_SIZE, 0);
}

static void testingthis()
{
  fprintf(stderr, "testingthistestingthistestingthistestingthistestingthistestingthistestingthistestingthistestingthistestingthis\n");
  fflush(stderr);  
}

static u32 recRecompile()
{
#ifdef IPHONE
  if (mprotect(recMemBase, RECMEM_SIZE, PROT_READ|PROT_WRITE))
      fprintf(stderr, "Couldn't mprotect (write) recompiler memory!\n");
#endif

	if ( (u32)recMem - (u32)recMemBase >= RECMEM_SIZE_MAX )
		recReset();

	recMem = (u32*)(((u32)recMem + 64) & ~(63));
	recMemStart = recMem;

	regReset();

	PC_REC32(psxRegs->pc) = (u32)recMem;
	oldpc = pc = psxRegs->pc;

	DISASM_INIT

	if( isInBios )
	{
		if( isInBios == 1 )
		{
			isInBios = 2;
			ARM_PUSH(ARM_POINTER, SAVED_ALL_REGS);
		}
		else if( isInBios == 2 && psxRegs->pc == 0x80030000 )
		{
			PC_REC32(psxRegs->pc) = 0;
			isInBios = 0;
			ARM_POP(ARM_POINTER, SAVED_ALL_REGS);
			ARM_BX(ARM_POINTER, ARMREG_LR);
			clear_insn_cache((u32)recMemStart, (u32)recMem, 0);
#ifdef IPHONE
			if (mprotect(recMemBase, RECMEM_SIZE, PROT_READ|PROT_EXEC))
          fprintf(stderr, "Couldn't mprotect (exec) recompiler memory!\n");
#endif
			return (u32)recMemStart;
		}
	}

	rec_recompile_start();

	for (;;)
	{
		psxRegs->code = *(u32*)(psxMemRLUT[pc>>16] + (pc&0xffff));
		DISASM_MIPS
		pc+=4;
		recBSC[psxRegs->code>>26]();
    		memset(regcache.arm_islocked, 0, 16);
		if (end_block)
		{
			end_block = 0;
			recRet();
			DISASM_ARM
			clear_insn_cache((u32)recMemStart, (u32)recMem, 0);
#ifdef IPHONE
			if (mprotect(recMemBase, RECMEM_SIZE, PROT_READ|PROT_EXEC))
          fprintf(stderr, "Couldn't mprotect (exec) recompiler memory!\n");
#endif
			return (u32)recMemStart;
		}
	}

  // Never should be reached
	return 0;
}

static void recNULL() { }

static int recInit()
{
	int i;

	psxRegs = &recRegs;

  /*recMemBaseStatic = (u8*)malloc(RECMEM_SIZE + TARGET_ALLOCSIZE);
  if(recMemBaseStatic == NULL)
  {
    fprintf(stderr, "Error allocating memory\n");
    SysMessage("Error allocating memory"); return -1;
  }*/
#ifdef IPHONE
  recMemBase = (unsigned char *)(((unsigned long) recMemBaseStatic + TARGET_ALLOCSIZE) & ~(TARGET_PAGESIZE));
  
  if((recMemBase = (u8*)mmap(recMemBase, RECMEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED|MAP_ANON, -1, 0)) == MAP_FAILED)
  {
    fprintf(stderr, "Error mmaping memory: 0x%x\n", errno);
    exit(0);
  }
#else
  recMemBase = recMemBaseStatic;
#endif

	recMem = (u32*)recMemBase;	
	loadedpermregs = 0;	
	recReset();

	//recRAM = (char*) malloc(0x200000);
	//recROM = (char*) malloc(0x080000);
	if (recRAM == NULL || recROM == NULL || recMemBase == NULL || psxRecLUT == NULL) {
		SysMessage("Error allocating memory"); return -1;
	}

	for (i=0; i<0x80; i++) psxRecLUT[i + 0x0000] = (u32)&recRAM[(i & 0x1f) << 16];
	memcpy(psxRecLUT + 0x8000, psxRecLUT, 0x80 * 4);
	memcpy(psxRecLUT + 0xa000, psxRecLUT, 0x80 * 4);

	for (i=0; i<0x08; i++) psxRecLUT[i + 0xbfc0] = (u32)&recROM[i << 16];
  /*
  fprintf(stderr, "psxRegs 0x%x \n", psxRegs);
  fprintf(stderr, "psxMemWrite32 0x%x \n", psxMemWrite32);
  fprintf(stderr, "psxBranchTest_rec 0x%x \n", psxBranchTest_rec);
  */
	return 0;
}

static void recShutdown()
{
  /*if(recMemBaseStatic)
  {
    free(recMemBaseStatic);
    recMemBaseStatic = NULL;
  }*/
}

static void recSPECIAL()
{
	recSPC[_Funct_]();
}

static void recREGIMM()
{
	recREG[_Rt_]();
}

static void recCOP0()
{
	recCP0[_Rs_]();
}

static void recCOP2()
{
	recCP2[_Funct_]();
}

static void recBASIC()
{
	recCP2BSC[_Rs_]();
}

void (*recBSC[64])() =
{
	recSPECIAL, recREGIMM, recJ   , recJAL  , recBEQ , recBNE , recBLEZ, recBGTZ,
	recADDI   , recADDIU , recSLTI, recSLTIU, recANDI, recORI , recXORI, recLUI ,
	recCOP0   , recNULL  , recCOP2, recNULL , recNULL, recNULL, recNULL, recNULL,
	recNULL   , recNULL  , recNULL, recNULL , recNULL, recNULL, recNULL, recNULL,
	recLB     , recLH    , recLWL , recLW   , recLBU , recLHU , recLWR , recNULL,
	recSB     , recSH    , recSWL , recSW   , recNULL, recNULL, recSWR , recNULL,
	recNULL   , recNULL  , recLWC2, recNULL , recNULL, recNULL, recNULL, recNULL,
	recNULL   , recNULL  , recSWC2, recHLE  , recNULL, recNULL, recNULL, recNULL
};

void (*recSPC[64])() =
{
	recSLL , recNULL, recSRL , recSRA , recSLLV   , recNULL , recSRLV, recSRAV,
	recJR  , recJALR, recNULL, recNULL, recSYSCALL, recBREAK, recNULL, recNULL,
	recMFHI, recMTHI, recMFLO, recMTLO, recNULL   , recNULL , recNULL, recNULL,
	recMULT, recMULTU, recDIV, recDIVU, recNULL   , recNULL , recNULL, recNULL,
	recADD , recADDU, recSUB , recSUBU, recAND    , recOR   , recXOR , recNOR ,
	recNULL, recNULL, recSLT , recSLTU, recNULL   , recNULL , recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL   , recNULL , recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL   , recNULL , recNULL, recNULL
};

void (*recREG[32])() =
{
	recBLTZ  , recBGEZ  , recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL  , recNULL  , recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recBLTZAL, recBGEZAL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL  , recNULL  , recNULL, recNULL, recNULL, recNULL, recNULL, recNULL
};

void (*recCP0[32])() =
{
	recMFC0, recNULL, recCFC0, recNULL, recMTC0, recNULL, recCTC0, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recRFE , recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL
};

void (*recCP2[64])() =
{
	recBASIC, recRTPS , recNULL , recNULL, recNULL, recNULL , recNCLIP, recNULL, // 00
	recNULL , recNULL , recNULL , recNULL, recOP  , recNULL , recNULL , recNULL, // 08
	recDPCS , recINTPL, recMVMVA, recNCDS, recCDP , recNULL , recNCDT , recNULL, // 10
	recNULL , recNULL , recNULL , recNCCS, recCC  , recNULL , recNCS  , recNULL, // 18
	recNCT  , recNULL , recNULL , recNULL, recNULL, recNULL , recNULL , recNULL, // 20
	recSQR  , recDCPL , recDPCT , recNULL, recNULL, recAVSZ3, recAVSZ4, recNULL, // 28 
	recRTPT , recNULL , recNULL , recNULL, recNULL, recNULL , recNULL , recNULL, // 30
	recNULL , recNULL , recNULL , recNULL, recNULL, recGPF  , recGPL  , recNCCT  // 38
};

void (*recCP2BSC[32])() =
{
	recMFC2, recNULL, recCFC2, recNULL, recMTC2, recNULL, recCTC2, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL
};

static void recExecute()
{
	isInBios = 0;
	loadedpermregs = 0;	

	void (**recFunc)() = (void (**)()) (u32)PC_REC(psxRegs->pc);

	if (*recFunc == 0) 
		recRecompile();
      
	(*recFunc)();
}

static void recExecuteBlock()
{
	isInBios = 1;
	loadedpermregs = 0;	

	void (**recFunc)() = (void (**)()) (u32)PC_REC(psxRegs->pc);

	if (*recFunc == 0) 
		recRecompile();

	(*recFunc)();
}

static void recClear(u32 Addr, u32 Size)
{
	//memset((u32*)PC_REC(Addr), 0, (Size * 4));
	//memset(recRAM, 0, 0x200000);
	//printf("addr %x\n", Addr);

	memset((u32*)PC_REC(Addr), 0, (Size * 4));
	//memset(recRAM+0x0, 0, 0x200000);

	if( Addr == 0x8003d000 )
	{
		// temp fix for Buster Bros Collection and etc.
		memset(recRAM+0x4d88, 0, 0x8);
		//recReset();
		//memset(recRAM, 0, 0x200000);
		//memset(&recRAM[0x1362<<1], 0, (0x4));
	}
}

static void recReset()
{
	memset(recRAM, 0, 0x200000);
	memset(recROM, 0, 0x080000);

	//memset((void*)recMemBase, 0, RECMEM_SIZE);
	//rec_flush_cache();

	recMem = (u32*)recMemBase;

	regReset();

	branch = 0;	
	end_block = 0;
}

R3000Acpu psxRec =
{
	recInit,
	recReset,
	recExecute,
	recExecuteBlock,
	recClear,
	recShutdown
};

