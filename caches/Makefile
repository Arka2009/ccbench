TARGET=caches
SOURCES:=caches.c 

<<<<<<< Updated upstream
# <Num Cores. <Num Elements Per Core> <Num Iterations>
=======
ifeq ($(ARCH),riscv64)
	CC=riscv64-unknown-elf-gcc
	INCLUDEFLAGS=-I../common -I$(HOME)/.local/include -L$(HOME)/.local/lib
	CFLAGS=-std=gnu99 -O2 -ffast-math -funroll-loops $(INCLUDEFLAGS) -mcmodel=medany -g
	LD_FLAGS=-ffast-math -static
	LD_LIBS=-lm -lc -lecotools
else ifeq ($(ARCH),x86_64)
	CC=gcc
	INCLUDEFLAGS=-I../common -I$(HOME)/.local/include -L$(HOME)/.local/lib
	CFLAGS=-std=c99 -mtune=native -march=native -mssse3 -O3 -funroll-loops $(INCLUDEFLAGS) -DWITH_BARRIER -Wall
	LD_FLAGS=
	LD_LIBS=-lpthread -lm -lecotools
endif

# <Num Cores> <Num Elements Per Core> <Num Iterations>
>>>>>>> Stashed changes
RUN_TYPE = 0
ARRAY_SIZE = 16
ARRAY_SIZE = 128
NUM_ITERATIONS = 10
APP_FLAGS ?=$(ARRAY_SIZE) $(NUM_ITERATIONS) $(RUN_TYPE)

include ../common/Makefile.tests
