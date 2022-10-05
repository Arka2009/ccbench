#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

// inline __attribute__((always_inline))
uint64_t __eco_rdtsc() {
        uint64_t res = 0;
        #if __amd64__
        uint64_t a, d;
        __asm__ volatile("mfence \n\t rdtsc" : "=a" (a), "=d" (d));
        res = (d << 32) | a;
        #elif __riscv
        __asm__ __volatile__("rdtime %0" : "=r" (res));
        #else
        __asm__ __volatile__("mrs %0, cntvct_el0":"=r"(res));
        #endif
        return res;
}

int main(){
    uint64_t first,second;
    first = _eco_rdtsc();
    sleep(1);
    second = __eco_rdtsc();
    printf("Number of cycles: %lu \n",(second-first));
}