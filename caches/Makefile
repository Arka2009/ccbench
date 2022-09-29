TARGET=caches
SOURCES:=caches.c 
SOURCES+=../common/cctimer.c ../common/barrier.c ../common/cclfsr.c
OBJECTS?=$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(patsubst %.cc,%.o,$(SOURCES))))
ARCH:=$(shell uname -p)

ifeq ($(ARCH),riscv64)
	CC=riscv64-unknown-elf-gcc
	INCLUDEFLAGS=-I../common -I$(HOME)/.local/include
	CFLAGS=-std=gnu99 -O2 -ffast-math -funroll-loops $(INCLUDEFLAGS) -mcmodel=medany -g
	LD_FLAGS=-ffast-math -static
	LD_LIBS=-lm -lc -lecotools
else ifeq ($(ARCH),x86_64)
	CC=gcc
	INCLUDEFLAGS=-I../common -I$(HOME)/.local/include
	CFLAGS=-std=c99 -mtune=native -march=native -mssse3 -O3 -funroll-loops $(INCLUDEFLAGS) -DWITH_BARRIER -Wall
	LD_FLAGS=
	LD_LIBS=-lpthread -lm -lecotools
endif

# <Num Cores> <Num Elements Per Core> <Num Iterations>
RUN_TYPE = 0
ARRAY_SIZE = 128
NUM_ITERATIONS = 10
APP_FLAGS?=$(ARRAY_SIZE) $(NUM_ITERATIONS) $(RUN_TYPE)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LD_FLAGS) $(LD_LIBS)

run: $(TARGET)
	./$(TARGET) $(APP_FLAGS)

clean:
	-rm -f *.o *.d ../common/*.o $(TARGET)