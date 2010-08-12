#include "../gpuAPI.h"
#include "fixed.h"


///////////////////////////////////////////////////////////////////////////////
void  codeGenTest();

///////////////////////////////////////////////////////////////////////////////
//  big precision inverse table.
s32 s_invTable[(1<<TABLE_BITS)];

///////////////////////////////////////////////////////////////////////////////
void  xInit()
{
  for(int i=1;i<=(1<<TABLE_BITS);++i)
  {
    double v = 1.0 / double(i);
    v *= double(0x80000000);
    s_invTable[i-1]=s32(v);
  }

  codeGenTest();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "newGPU.h"

///////////////////////////////////////////////////////////////////////////////
//  Code generation "hints"
#ifdef __WIN32__
#   undef GPU_ARM_ASM
#endif

///////////////////////////////////////////////////////////////////////////////
//  Option Masks
#define   L ((CF>>0)&1)
#define   B ((CF>>1)&1)
#define   M ((CF>>2)&1)
#define  BM ((CF>>3)&3)
#define  TM ((CF>>5)&3)
#define   G ((CF>>7)&1)

///////////////////////////////////////////////////////////////////////////////
//  Poligon innerloops driver
#include "inner_Poly.h"

void  codeGenTest()
{
  gpuPolySpanFn< (3 + (1<<3) + (1<<5) + (1<<7)) >();
}

