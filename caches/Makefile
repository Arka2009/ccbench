SOURCES:=caches.c 
SOURCES+=../common/cctimer.c ../common/barrier.c ../common/cclfsr.c
OBJECTS?=$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(patsubst %.cc,%.o,$(SOURCES))))

CC_AMD64=/usr/bin/gcc
CC_RISCV?=/usr/bin/riscv64-linux-gnu-gcc
CC_ARM?=/usr/bin/arm-linux-gnueabi-gcc

IFLAGS_PMU_AMD64=-I../common -I$(HOME)/.local/include
IFLAGS_GEM5_RV64=-I../common -I$(GEM5_HOME)/include
LFLAGS_GEM5_RV64=-lpthread -lm
LFLAGS_PMU_AMD64=-lpthread -lm -lecotools
LDFLAGS_PMU_AMD64=-L$(HOME)/.local/lib
TARGET_GEM5_RV64=caches.GEM5_RV64
TARGET_AARCH64=caches.AARCH64
TARGET_AMD64=caches.AMD64

OPT?=-O2
CFLAGS+=-DWITH_BARRIER -Wall $(OPT)

all: $(TARGET_GEM5_RV64) $(TARGET_AMD64)

$(TARGET_GEM5_RV64): $(SOURCES)
	$(CC_RISCV) $(CFLAGS) $(IFLAGS_GEM5_RV64) $(SOURCES) -DGEM5_RV64 --static --std=gnu99 -march=rv64gc -mabi=lp64d -mcmodel=medany -fno-common -fno-builtin-printf -Wno-implicit -fno-inline-small-functions $(LFLAGS_GEM5_RV64) -o $(TARGET_GEM5_RV64) $(GEM5_HOME)/util/m5/src/abi/riscv/m5op.S

#$(TARGET_AMD64): $(SOURCES)
#	$(CC_AMD64) $(CFLAGS) $(IFLAGS_PMU_AMD64) $(SOURCES) $(LDFLAGS_PMU_AMD64) -o $(TARGET_AMD64) $(LFLAGS_PMU_AMD64)

clean:
	-rm -f *.o *.d ../common/*.o $(TARGET_AARCH64) $(TARGET_GEM5_RV64) $(TARGET_AMD64)
