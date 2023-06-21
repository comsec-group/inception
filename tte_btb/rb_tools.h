typedef unsigned long u64;
typedef unsigned char u8;
// ------------------------------------
// RB tools
// ------------------------------------

#ifndef RB_PTR
#define RB_PTR 0x13200000UL
#endif
static u8* rb = (u8 *)RB_PTR;
#ifndef RB_STRIDE
#define RB_STRIDE (1UL<<12)
#endif
#ifndef RB_SLOTS
#define RB_SLOTS 8
#endif
static u64 rb_hist[RB_SLOTS];

#define ROUND_2MB_UP(x) (((x) + 0x1fffffUL) & ~0x1fffffUL)
#define RB_SZ ROUND_2MB_UP((RB_SLOTS * RB_STRIDE) + 0x1000UL)
#define MMAP_FLAGS (MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED_NOREPLACE)


#include "sys/mman.h"
#include "err.h"
#include <string.h>

#define rb_reset() memset(rb_hist, 0, RB_SLOTS * sizeof(*rb_hist))

// start measure.
static __always_inline u64 rdtsc(void) {
    u64 lo, hi;
    asm volatile ("CPUID\n\t"
            "RDTSC\n\t"
            "movq %%rdx, %0\n\t"
            "movq %%rax, %1\n\t" : "=r" (hi), "=r" (lo)::
            "%rax", "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}

// stop meassure.
static __always_inline u64 rdtscp(void) {
    u64 lo, hi;
    asm volatile("RDTSCP\n\t"
            "movq %%rdx, %0\n\t"
            "movq %%rax, %1\n\t"
            "CPUID\n\t": "=r" (hi), "=r" (lo):: "%rax",
            "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}
static __always_inline void flush(void *p) {
    asm volatile(
            "mfence		\n"
            "clflush 0(%[p])	\n"
            : : [p] "c" (p) : "rax");
}
// it's generally better to hardcode this than to compute it>
// 1. threshold becomes an immediate value
// 2. calibrating requires a warmup to get real results => slow.
#define CACHE_MISS_THRES 120

static __always_inline void reload_one(long addr, u64 *results) {
    unsigned volatile char *p = (u8 *)addr;
    u64 t0 = rdtsc();
    *(volatile unsigned char *)p;
    u64 dt = rdtscp() - t0;
    if (dt < CACHE_MISS_THRES) results[0]++;
}

static inline __attribute__((always_inline)) void reload_range(long base, long stride, int n, u64 *results) {
    __asm__ volatile("mfence\n"); // all memory operations done.
#pragma clang loop unroll(full)
    for (u64 k = 0; k < n; ++k) {
        u64 c = (k*13+9)&(n-1);

        unsigned volatile char *p = (u8 *)base + (stride * c);
        u64 t0 = rdtsc();
        *(volatile unsigned char *)p;
        u64 dt = rdtscp() - t0;
        if (dt < CACHE_MISS_THRES) results[c]++;
    }
}

static __always_inline void flush_range(long start, long stride, int n) {
    asm("mfence");
    for (u64 k = 0; k < n; ++k) {
        volatile void *p = (u8 *)start + k * stride;
        // flush_prefL2((void *)p);
        __asm__ volatile("clflushopt (%0)\n"::"r"(p));
        __asm__ volatile("clflushopt (%0)\n"::"r"(p));
    }
    asm("lfence");
    // __asm__ volatile("clflush (%0)\n"::"r"(start+16*stride));
}

static __always_inline void rb_init() {
    if (mmap((void *)RB_PTR, RB_SZ, PROT_READ | PROT_WRITE, MMAP_FLAGS, -1, 0) == MAP_FAILED) {
        err(1, "rb");
    }
    madvise((void *)RB_PTR, RB_SZ, MADV_HUGEPAGE);
    memset((void *)RB_PTR, 0xcc, RB_SZ);
    // if we're root we can verify that huge
}
#define rb_flush() flush_range(RB_PTR, RB_STRIDE, RB_SLOTS);
#define rb_reload() reload_range(RB_PTR, RB_STRIDE, RB_SLOTS, rb_hist);
#define rb_reload_one(i) reload_one(RB_PTR+i*RB_STRIDE, &rb_hist[i]);

#define rb_print() do {\
    printf("rb[0:%d]: ",RB_SLOTS);\
    for (int i = 0; i < RB_SLOTS; ++i) {\
        printf("%ld ", rb_hist[i]);\
    }\
    printf("\n");\
} while(0)

// ------------------------------------
