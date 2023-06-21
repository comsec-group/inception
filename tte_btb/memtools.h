typedef unsigned long u64;

// memory mapping
struct j_malloc {
    union {
        void *ptr;
        u64 ptr_u64;
        void (*fptr)();
    };
    // start page
    void *map_base;
    u64 map_sz;
    // map_base           map_base+map_sz
    // |----------------------|
    //        <-------->
    //        ptr     ptr+code_sz
};
#define J_MALLOC_SIZE 0x18

/**
 * makes an executable mapping.
 * returns 0 on success, -1 otherwise
 */
int map_exec(struct j_malloc *m, u64 addr, u64 code_sz);

/**
 * returns 0 on success, -1 otherwise
 */
int map_code(struct j_malloc *m, u64 addr, void *code_tmpl, u64 code_sz);

/**
 * Same as above, but if the area already is mapped it turns it into RWX and
 * copies code there anyway. Likely to break the whole thing.
 * returns 0 on success, -1 otherwise
 */
int map_code_force(struct j_malloc *m, u64 addr, void *code_tmpl, u64 code_sz);


/**
 * @param u64 mask To give the code a certain alignment. Use mask = -1 for any
 * alignment, ~0xfUL for 16 byte alignment, etc,
 * returns 0 on success, -1 otherwise
 */
int map_code_rand(struct j_malloc *m, void *code_tmpl, u64 code_sz, u64 mask);

/**
 * returns 0 on success, -1 otherwise
 */
int junmap(struct j_malloc *m);

void code_poke(void *addr, char *code, int len);

// ----------------------------------------------------------------------------
// - code templating ----------------------------------------------------------
// ----------------------------------------------------------------------------
// labels right next to each other are made into one..
#define mk_tmpl(name, str)\
    extern char name##__tmpl[]; \
    extern char e_##name##__tmpl[]; \
    asm(".align 0x1000           \n"\
        #name"__tmpl:            \n"\
        str                         \
        "\ne_"#name"__tmpl: nop"    \
        )\

#define tmpl_sz(name) (unsigned long) (e_##name##__tmpl - name##__tmpl)

#define alloc_tmpl(c, name, adr)\
    (adr) \
        ? map_code(c, (adr), name##__tmpl, tmpl_sz(name)) \
        : map_code_rand(c, name##__tmpl, tmpl_sz(name), ~0x3fUL)

