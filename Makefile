O      = o

ZODTTD=1

PREFIX  =
CC     	= ${PREFIX}gcc
CXX    	= ${PREFIX}g++
AS     	= ${PREFIX}g++
STRIP  	= ${PREFIX}strip
INC    	= 
INCS   	= ${INC}
LD     	= ${PREFIX}g++
M4      = ${PREFIX}m4
LDFLAGS	= -lpthread -lm -ldl -lz

#STD_INTERP=1
#NAME  = psx4all_std_int

STD_DYNAREC=1
NAME  = psx4m

PROG   = $(NAME)

USE_HLE=1
NEWSPU=1
#DRHELLGPU=1
#PEOPSGPU=1
#DUMMYGPU=1

#NULLSPU=1

DEFS = -DPSXDUMMYDEFINE
#DEFS  += -DUSE_OLD_COUNTERS
#DEFS  += -DNOSOUND

#DEFS += -DNO_RENDER

#DEFS += -DEMU_LOG -DLOG_STDOUT
#DEFS += -DPROFILER_PSX4ALL
#DEFS += -DPROFILER_SDL
#DEFS += -DAUTOEVENTS=4000
#DEFS += -DMAXFRAMES=4000
#DEFS += -DNOTHREADLIB 
#DEFS += -DGP2X_SDLWRAPPER_NODOUBLEBUFFER

DEFS += -DMAEMO_CHANGES

ifdef STD_INTERP
DEFS += -DINTERP
endif

ifdef STD_DYNAREC
DEFS += -DDYNAREC
endif

ifdef USE_HLE
DEFS += -DWITH_HLE
endif

DEFAULT_CFLAGS = -DPANDORA -DGP2X -DARM_ARCH \
 -DDATA_PREFIX=\"/home/user\" \
 -DROM_PREFIX=\"/home/user/MyDocs\" \
 -Isrc -I./maemo -I./iphone -I$(INCS) $(DEFS) -ggdb -Wno-write-strings

DEFAULT_CFLAGS += $(shell pkg-config --cflags hildon-1 libpulse)
LDFLAGS += $(shell pkg-config --libs hildon-1 libpulse)

MORE_CFLAGS = -march=armv7-a -mcpu=cortex-a8 -mtune=cortex-a8 -mfloat-abi=softfp -mfpu=neon \
 -O3 -ffast-math -ftemplate-depth-36 -fstrict-aliasing \
 -mstructure-size-boundary=32 -falign-functions=32 -falign-loops \
 -falign-labels -falign-jumps -finline -finline-functions -fno-builtin -fno-common \
 -funroll-loops -fstrength-reduce -fpeel-loops -fomit-frame-pointer -fsigned-char -DINLINE="inline"

#MORE_CFLAGS += -DMITSHM $(DEFS) -DUNZIP_SUPPORT -DZLIB -DHAVE_MKSTEMP -DHAVE_STRINGS_H 
#'-DACCEPT_SIZE_T=int'

CFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS)
CPPFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS)
ASFLAGS  = -c $(DEFAULT_CFLAGS) $(MORE_CFLAGS)

ASM_OBJS = pandora/memcmp.S pandora/memcpy.S pandora/memset.S pandora/strcmp.S pandora/strlen.S pandora/strncmp.S

OBJS =	\
	CdRom.o \
	Decode_XA.o \
	gte.o \
	LnxMain.o \
	Mdec.o \
	Misc.o \
	plugins.o \
	PsxBios.o \
	PsxCounters.o \
	PsxDma.o \
	PsxHLE.o \
	PsxHw.o \
	PsxMem.o \
	R3000A.o \
	Sio.o \
	Spu.o \
	interpreter/interpreter.o \
	iphone/minimal.o \
	maemo/hildon.o
	
OBJS += 	cdr/oldversion/cdriso.o 

#OBJS += 	cdr/CDDAData.o 
#OBJS += 	cdr/FileInterface.o 
#OBJS += 	cdr/Globals.o 
#OBJS += 	cdr/Open.o 
#OBJS += 	cdr/SubchannelData.o 
#OBJS += 	cdr/TrackParser.o 
#OBJS += 	cdr/unrar/unrarlib.o 
#OBJS += 	cdr/bzip/blocksort.o 
#OBJS += 	cdr/bzip/bzlib.o 
#OBJS += 	cdr/bzip/compress.o 
#OBJS += 	cdr/bzip/crctable.o 
#OBJS += 	cdr/bzip/decompress.o 
#OBJS += 	cdr/bzip/huffman.o 
#OBJS += 	cdr/bzip/randtable.o 

ifdef NULLSPU
OBJS += nullspu/spu_core.o 
else
ifdef NEWSPU
OBJS +=	\
	franspu/spu_callback.o \
	franspu/spu_dma.o \
	franspu/spu_registers.o \
	franspu/spu_output_pulseaudio.o
else
OBJS +=	\
	petespu/stdafx.o \
	petespu/adsr.o \
	petespu/spu_dma.o \
	petespu/freeze.o \
	petespu/alsa.o \
	petespu/registers.o \
	petespu/reverb.o \
	petespu/spu.o \
	petespu/xa.o 
endif
endif

ifdef STD_DYNAREC
OBJS += recompiler3/arm/recompiler.o 
OBJS += recompiler3/arm/disasm.o 
OBJS += recompiler3/arm/mips_disasm.o 
#ASM_OBJS += recompiler3/arm/arm.S 
endif

GPU_OBJS =  gpuAPI/gpuAPI.o


ifdef PEOPSGPU
GPU_OBJS += gpuAPI/peopsGPU/cfg.o  \
            gpuAPI/peopsGPU/draw.o \
            gpuAPI/peopsGPU/fps.o  \
            gpuAPI/peopsGPU/gpu.o  \
            gpuAPI/peopsGPU/key.o  \
            gpuAPI/peopsGPU/menu.o \
            gpuAPI/peopsGPU/prim.o \
            gpuAPI/peopsGPU/soft.o \
            gpuAPI/peopsGPU/texture.o \
            libmesagles.a
DEFAULT_CFLAGS += -I ./glesport/include -DPEOPS_GPU
else
ifdef DRHELLGPU
GPU_OBJS += gpuAPI/drhellGPU/GPU.o 
else
GPU_OBJS += gpuAPI/newGPU/newGPU.o 	      \
            gpuAPI/newGPU/fixed.o         \
            gpuAPI/newGPU/core_Command.o  \
            gpuAPI/newGPU/core_Dma.o      \
            gpuAPI/newGPU/core_Draw.o     \
            gpuAPI/newGPU/core_Misc.o     \
            gpuAPI/newGPU/raster_Sprite.o \
            gpuAPI/newGPU/raster_Poly.o   \
            gpuAPI/newGPU/raster_Line.o   \
            gpuAPI/newGPU/raster_Image.o  \
            gpuAPI/newGPU/inner.o
endif
endif

ASM_OBJS += gpuAPI/newGPU/ARM_asm.S

%.o: %.cpp
	${CXX} ${CFLAGS} -c -o $@ $<

#%.o: %.s
#	${CXX} ${CFLAGS} -c -o $@ $<

%.z: %.c
	${CC} ${CFLAGS} -c -o $@ $<

%.o: %.c
	${CXX} ${CFLAGS} -c -o $@ $<

all:	${OBJS} ${GPU_OBJS}
	${LD} -O3 ${ASM_OBJS} ${CFLAGS} ${GPU_OBJS} ${OBJS} -o ${PROG} ${LDFLAGS}  
#	${STRIP} ${PROG}


#$(PROG): $(OBJS) $(GPU_OBJS) $(ASM_OBJS)
#	$(LD) $(CFLAGS) -o $(PROG) $(OBJS) $(GPU_OBJS) $(ASM_OBJS) $(LDFLAGS) -s
#	$(STRIP) $(PROG)

run: $(PROG)
	./$(PROG) example

clean:
	$(RM) $(PROG) $(OBJS) $(GPU_OBJS)
	make -C glesport/src/mesa clean

libmesagles.a:
	make -C glesport/src/mesa gles-static
	cp -f glesport/src/mesa/libmesagles.a $@

test:
	$(CXX) $(CFLAGS) -S -o arm/arm_std_rec.S arm/arm_std_rec.cpp

test2:
	$(CXX) $(CFLAGS) -S -o misc.S misc.cpp

