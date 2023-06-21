#include <string.h>
#include <sys/mman.h>
#include <err.h>
#include <stdlib.h>
#include "./memtools.h"

#define MMAP_FLAGS (MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED_NOREPLACE)
#define PROT_RW    (PROT_READ | PROT_WRITE)
#define PROT_RWX   (PROT_RW | PROT_EXEC)
#define PG_ROUND(n) (((((n)-1UL)>>12)+1)<<12)

static inline u64 rand47() {
	return (((unsigned long)rand())<<16) ^ rand();
}


int map_exec(struct j_malloc *m, u64 addr, u64 code_sz)
{
    m->ptr = (void *)addr;
    m->map_base = (void *)((u64)m->ptr & ~0xfff);
    m->map_sz = PG_ROUND(((u64)m->ptr & 0xfff) + code_sz);
    if (mmap(m->map_base, m->map_sz, PROT_RWX, MMAP_FLAGS, -1, 0) ==
        MAP_FAILED) {
        return -1;
    }
    return 0;
}

int map_code(struct j_malloc *m, u64 addr, void *code_tmpl, u64 code_sz)
{
    if (map_exec(m,addr,code_sz)) {
        return -1;
    }
    memcpy(m->ptr, code_tmpl, code_sz);
    return 0;
}

int map_code_force(struct j_malloc *m, u64 addr, void *code_tmpl, u64 code_sz) {
    m->ptr = (void *)addr;
    m->map_base = (void *)((u64)m->ptr & ~0xfff);
    m->map_sz = PG_ROUND(((u64)m->ptr & 0xfff) + code_sz);
    if (mmap(m->map_base, m->map_sz, PROT_RWX, MMAP_FLAGS, -1, 0) ==
        MAP_FAILED) {
        if (mprotect(m->map_base, m->map_sz, PROT_RWX)) {
            return -1;
        }
    }
    memcpy(m->ptr, code_tmpl, code_sz);
    return 0;
}

int map_code_rand(struct j_malloc *m, void *code_tmpl, u64 code_sz, u64 mask)
{
    for (int try = 200; try--;) {
        if (map_code(m, rand47() & mask, code_tmpl, code_sz) == -1) {
            continue;
        }
        return 0;
    }
    return -1;
}

int junmap(struct j_malloc *m)
{
    int res = munmap(m->map_base, m->map_sz);
    m->map_base = 0;
    m->map_sz = 0;
    m->ptr_u64 = 0;
    return res;
}

void code_poke(void *addr, char *code, int len)
{
    // could even be as fancy as looking up the current permissions via
    // /proc/self/maps. but no thanks.
    int result;
    void *addr_pg = (void *)((long)addr & ~0xfffUL);
    result = mprotect(addr_pg, len, PROT_RWX);
    if (result != 0) {
        /* err(result, "set rwx"); */
    }
    memcpy(addr, code, len);
    result |= mprotect(addr_pg, len, PROT_READ | PROT_EXEC);
    if (result != 0) {
        /* err(result, "set rx"); */
    }
}
