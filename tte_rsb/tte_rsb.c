#include <err.h>
#include <string.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <signal.h>
#include <setjmp.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define RB_PTR 0x13370000
#define RB_STRIDE_BITS 12
#define RB_STRIDE (0x1UL << RB_STRIDE_BITS)
#define RB_SLOTS 0x21
#define RSB_SIZE 0x20

#if defined(ZEN)
    #define PTRN 0x20100000000UL //Zen(+)
#else
    #define PTRN 0x100100000000UL //Zen 2
#endif

#define ROUNDS 1000

#define MMAP_FLAGS (MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE | MAP_FIXED_NOREPLACE)
#define PROT_RW    (PROT_READ | PROT_WRITE)

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

#define NOPS_str(n) ".rept " xstr(n) "\n\t"\
    "nop\n\t"\
    ".endr\n\t"

#define str(s) #s
#define xstr(s) str(s)

static inline __attribute__((always_inline)) uint64_t rdtsc(void) {
    uint64_t lo, hi;
    asm volatile ("CPUID\n\t"
            "RDTSC\n\t"
            "movq %%rdx, %0\n\t"
            "movq %%rax, %1\n\t" : "=r" (hi), "=r" (lo)::
            "%rax", "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}

static inline __attribute__((always_inline)) uint64_t rdtscp(void) {
    uint64_t lo, hi;
    asm volatile("RDTSCP\n\t"
            "movq %%rdx, %0\n\t"
            "movq %%rax, %1\n\t"
            "CPUID\n\t": "=r" (hi), "=r" (lo):: "%rax",
            "%rbx", "%rcx", "%rdx");
    return (hi << 32) | lo;
}


static inline __attribute__((always_inline)) void reload_range(long base, long stride, int n, uint64_t *results) {
    asm("lfence");
    asm("mfence");
	int done = 0;
    for (volatile int k = 0; k < n / 2; k += 2) {
        uint64_t c = (n / 2) - 1 - ((k*13+9)&((n / 2)-1));
        unsigned volatile char *p = (uint8_t *)base + (stride * c);
        uint64_t t0 = rdtsc();
        *(volatile unsigned char *)p;
        uint64_t dt = rdtscp() - t0;
        if (dt < 200) results[c]++;
        if(k == (n / 2) - 2 && !done){
            k = -1;
            done = 1;
        }
	}
    asm("lfence");
    asm("mfence");

    done = 0;
    for (volatile int k = 0; k < n / 2; k += 2) {
        uint64_t c = (n / 2) + (n / 2) - 1 - ((k*13+9)&((n / 2)-1));
	    unsigned volatile char *p = (uint8_t *)base + (stride * c);
        uint64_t t0 = rdtsc();
        *(volatile unsigned char *)p;
        uint64_t dt = rdtscp() - t0;
	    if (dt < 200) results[c]++;
	    if(k == (n / 2) - 2 && !done){
	    	k = -1;
	    	done = 1;
	    }
	}
    asm("lfence");
    asm("mfence");

    if(n % 2 == 1){
        unsigned volatile char *p = (uint8_t *)base + (stride * (n - 1));
        uint64_t t0 = rdtsc();
        *(volatile unsigned char *)p;
        uint64_t dt = rdtscp() - t0;
        if (dt < 200) results[n - 1]++;
    }
}

static inline __attribute__((always_inline)) void flush_range(long start, long stride, int n) {
    asm("lfence");
    asm("mfence");
    for (uint64_t k = 0; k < n; ++k) {
        volatile void *p = (uint8_t *)start + k * stride;
        __asm__ volatile("clflushopt (%0)\n"::"r"(p));
        __asm__ volatile("clflushopt (%0)\n"::"r"(p));
    }
    asm("lfence");
    asm("mfence");
}

#if defined(BTB)
void b();
void c();
uint64_t b_addr = (uint64_t)b;
uint64_t c_addr = (uint64_t)c;
#elif defined(PHT)
volatile uint64_t value[100] = {0};
#endif

int main(int argc, char *argv[]){
    if (mmap((void*)RB_PTR, ((RB_SLOTS + 1)<<RB_STRIDE_BITS), PROT_RW, MMAP_FLAGS, -1, 0) == MAP_FAILED) {
        err(1, "rb");
    }

    uint64_t *results_arr[RSB_SIZE] = {results1, results2, results3, results4, results5, results6, results7, results8, results9, results10, results11, results12, results13, results14, results15, results16, results17, results18, results19, results20, results21, results22, results23, results24, results25, results26, results27, results28, results29, results30, results31, results32};

    for(int k = 0; k < RSB_SIZE; k++){
        uint64_t *res = results_arr[k];
        for(int i = 0; i < RB_SLOTS; i++){
            res[i] = 0;
        }
    }

    for(int i = 0; i < ROUNDS; i++){
#if defined(PHT) || defined(BTB)
    //Evict BTB entry belonging to conditional- or indirect jump
	 asm(
            ".rept 200000000\n\t"
            "nop\n\t"
            ".endr\n\t"
	 );
#endif

    //Prime RSB
	asm(
            "mfence\n\t"
            "lfence\n\t"
            ".secret=0\n\t"
            ".rept "xstr(RSB_SIZE)"\n\t"
            "call 4f\n\t"
            "add $.secret, %%rdi\n\t" 
            "shl $" xstr(RB_STRIDE_BITS) ", %%rdi\n\t"
            "mov "xstr(RB_PTR) "(%%rdi), %%r8\n\t"
            "int3\n\t"
            "4: pop %%r8\n\t"
            ".secret=.secret+1\n\t"
            ".endr\n\t"
        ::: "r8");

	flush_range(RB_PTR, 1<<RB_STRIDE_BITS, RB_SLOTS);	

#if defined(PHT)
        asm(
            "clflush (%[cond])\n\t"
            "mfence\n\t"
            "mov (%[cond]), %%rdi\n\t"
            "test %%rdi, %%rdi\n\t"
            "je 1f\n\t"
            //"mov "xstr((RB_PTR + (0 * RB_STRIDE)))", %%r10\n\t" //Optional indication signal
                
            ".rept "xstr(CALLS_CNT)"\n\t"
            "call 2f\n\t"
            "mov "xstr((RB_PTR + (RSB_SIZE * RB_STRIDE)))", %%r8\n\t"
            "int3\n\t"
            "2:\n\t"
            ".endr\n\t"
        

            //"mov "xstr((RB_PTR + (1 * RB_STRIDE)))", %%r10\n\t" //Optional indication signal
            "int3\n\t"

            "1: lfence\n\t"
        :: [cond]"r"(value) : "r8", "rdi");
#elif defined(RSB)
        asm(
            "call 1f\n\t"
            // "mov "xstr((RB_PTR + (0 * RB_STRIDE)))", %%r10\n\t" //Optional indication signal
	    
            ".rept "xstr(CALLS_CNT)"\n\t"
            "call 2f\n\t"
            "mov "xstr((RB_PTR + (RSB_SIZE * RB_STRIDE)))", %%r8\n\t"
            "int3\n\t"
            "2:\n\t"
            ".endr\n\t"
            
            // "mov "xstr((RB_PTR + (1 * RB_STRIDE)))", %%r10\n\t" //Optional indication signal
            "int3\n\t"
            NOPS_str(50)
            "1: pop %%r8\n\t"
            "pushq $2f\n\t"
            NOPS_str(1000)
            "clflush (%%rsp)\n\t"
            "mfence\n\t"
            "ret\n\t"
            "2: lfence\n\t"
        ::: "r8");
#elif defined(BTB)
        

        asm(
            "mov $2, %%rdi\n\t"    
            "mov %[b_addr], %%r8\n\t"        
            "mov $c, %%r9\n\t"
            "start_btb:\n\t"

            "clflush (%%r8)\n\t"
            "mfence\n\t"
            "jmp *(%%r8)\n\t"

            "b:\n\t"
            "jmp *%%r9\n\t"
            NOPS_str(1000)
            "btb_leak:\n\t"
            // "mov "xstr((RB_PTR + (0 * RB_STRIDE)))", %%r10\n\t" //Optional indication signal
            
            ".rept "xstr(CALLS_CNT)"\n\t"
            "call 4f\n\t"
            "mov "xstr((RB_PTR + (RSB_SIZE * RB_STRIDE)))", %%r8\n\t"
            "lfence\n\t"
            "4: pop %%r10\n\t"
            ".endr\n\t"
	    
            // "mov "xstr((RB_PTR + (1 * RB_STRIDE)))", %%r10\n\t" //Optional indication signal
	        "int3\n\t"
            "c:\n\t"

            "mov $btb_leak, %%r9\n\t"
            "mov %[c_addr], %%r8\n\t"
            "dec %%rdi\n\t"
            "cmp $1, %%rdi\n\t"
            "je start_btb\n\t"
        :: [b_addr]"r"(&b_addr), [c_addr]"r"(&c_addr) : "r8", "r9");
#endif

        for(int k = 0; k < RSB_SIZE; k++){
            if(k > 0) flush_range(RB_PTR, 1<<RB_STRIDE_BITS, RB_SLOTS);
            asm(
                "mfence\n\t"
                "mov $1f, %%r9\n\t"
                "mov $0, %%rdi\n\t"
                "pushq $1f\n\t"
                "clflush (%%rsp)\n\t"

                "ret\n\t"

                "1:\n\t"
            ::: "r9", "r10", "rdi");
            uint64_t *res = results_arr[k % 32];
            reload_range(RB_PTR, 1<<RB_STRIDE_BITS, RB_SLOTS, res);
        }
    }

    //Print results
    printf("     Return: ");
    for(int i = 0; i < RSB_SIZE; i++){
        printf(" - %04d", i + 1);
    }
    printf("\n");
    
    for(int i = 0 ; i < RSB_SIZE; ++i){
        printf("RB entry %02d: ", i);
        for(int k = 0; k < RSB_SIZE; k++){
            uint64_t *res = results_arr[k];
            printf(" - %04ld", res[i]);
        }
        printf("\n");
    }

    printf("   Hijacked: ");
    for(int i = 0; i < RSB_SIZE; i++){
        uint64_t *res = results_arr[i];
        printf(" - %04ld", res[RSB_SIZE]);
    }

    printf("\n");

    return 0;
}



