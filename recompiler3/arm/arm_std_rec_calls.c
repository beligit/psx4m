#include "../../PsxCommon.h"

u32 psxBranchTest_rec(u32 cycles, u32 pc)
{
	/* Misc helper */
	psxRegs->pc = pc;
	psxRegs->cycle += cycles * BIAS_CYCLE_INC;

	/* Make sure interrupts  always when mcd is active */
	//if( mcdst != 0 || (psxRegs->cycle - psxRegs->psx_next_io_base)  >= psxRegs->psx_next_io_count )
	{
		psxBranchTest();
	}

	u32 compiledpc = (u32)PC_REC32(psxRegs->pc);

	if( compiledpc != 0 )
	{
		return compiledpc;
	}

	return recRecompile();
}

#ifdef IPHONE
extern void sys_icache_invalidate(const void* Addr, size_t len);
extern void __clear_cache (char *beg, char *end);

void clear_insn_cache(u32 start, u32 end, u32 type)
{
	//__clear_cache((char *)start,(char *)end);

	sys_icache_invalidate((void*)start, end - start);
}
#endif

#ifdef PANDORA
static void clear_insn_cache(u32 start, u32 end, int type) 
{
  //flush cache
  register unsigned long _beg __asm ("a1") = start;            // block start
  register unsigned long _end __asm ("a2") = end;      // block end
  register unsigned long _flg __asm ("a3") = 0;
  register unsigned long _par __asm ("r7") = 0xf0002;
  __asm __volatile ("swi 0x0      @ sys_cacheflush"
    : // no outputs
    : "r" (_beg), "r" (_end), "r" (_flg), "r" (_par)
  );
}
#endif
