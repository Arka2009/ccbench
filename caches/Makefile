UBENCH=caches2.c
SOURCES:=$(UBENCH)
NUM_ELEMS:=512
CFLAGS=-DPRINT_SCRIPT_FRIENDLY -DLEN=$(NUM_ELEMS)

TARGET_GEM5_RV64=$(addprefix build/, $(patsubst %.c, %.$(NUM_ELEMS).GEM5_RV64, $(UBENCH)))
TARGET_AARCH64=$(addprefix build/, $(patsubst %.c, %.$(NUM_ELEMS).AARCH64, $(UBENCH)))
TARGET_AMD64=$(addprefix build/, $(patsubst %.c, %.$(NUM_ELEMS).AMD64, $(UBENCH)))

include ../Makefile.common