#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <cclfsr.h>
#include <math.h>
#include <stdint.h>
#ifdef GEM5_RV64
#include "gem5/m5ops.h"
#else
#include "roi_hooks.h"
#include "cpu_uarch.h"
#include "errordefs.h"
#endif

#define ITERS 1
#ifndef LEN
#define LEN 256
#endif

struct ll {
    struct ll *next;
    uint8_t arr[48];
} __attribute__((aligned(64)));

struct ll list[LEN];
uint64_t run_cycles;

#if (__amd64__) && (USE_PCM)
static struct __eco_roi_stats_struct res;
#endif

__attribute__ ((noinline, aligned(32)))
uint8_t loop(struct ll *n) {
    uint8_t t = 0;
    unsigned iter = 0;
    for (iter = 0; iter < ITERS; ++iter) {
        struct ll *cur = n;
        while (cur != NULL) {
            cur = cur->next;
        }
    }
    return t;
}

int main() {
    // Cache warmup and linking
    for (unsigned i = 0; i < LEN-1; i++) {
        list[i].next = &(list[i+1]);
    }
    uint8_t t;

    /** CRITICAL SECTION : START **/
    #if (__amd64__) && (USE_PCM)
    unsigned lproc_id = 5;
    core_counter_state_ptr_t start = __eco_roi_begin(lproc_id);
    #elif GEM5_RV64
    m5_reset_stats(0,0);
    #else
    uint64_t start_cycles = __eco_rdtsc(); //cc_get_cycles(clk_freq);
    #endif

    // Pointer Chase
    t = loop(&list[0]);

    /** CRITICAL SECTION : STOP **/
    #if (__amd64__) && (USE_PCM)
    core_counter_state_ptr_t stop = __eco_roi_end(lproc_id);
    res = __eco_counter_diff(stop, start);
    run_cycles = res.tsc;
    __eco_reset(lproc_id);
    #elif GEM5_RV64
    m5_dump_stats(0,0);
    #else
    uint64_t stop_cycles = __eco_rdtsc();
    run_cycles = stop_cycles - start_cycles;
    printf("Num cycles = %lu\n",run_cycles);
    #endif

    volatile int a = t;

    return a;
}