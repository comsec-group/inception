#ifndef _INCEPTION_KMOD_KMOD_H
#define _INCEPTION_KMOD_KMOD_H

#define REQ_TRIGGER_INCEPTION 1340
#define REQ_READ_GADGET_OFFSETS 1341
#define REQ_READ_PHYS_ADDR 1342
#define REQ_TRIGGER_INCEPTION_SELFTEST 1343
#define REQ_READ_RELOAD_BUFFER_ADDR 1344

typedef struct
{
    uint64_t gadget_leak_text;
    uint64_t gadget_leak_phys;
    uint64_t gadget_leak_phys2;
    uint64_t gadget_leak_phys_total;
    uint64_t gadget_leak_physmap;
    uint64_t gadget_leak_data;
    uint64_t gadget_leak_data_selftest;
    uint64_t phantom_jump1;
    uint64_t phantom_jump2;
    uint64_t distance_leak_text;
    uint64_t distance_leak_phys;
    uint64_t distance_leak_phys2;
    uint64_t distance_leak_phys_total;
    uint64_t distance_leak_physmap;
    uint64_t distance_leak_data;
    uint64_t distance_leak_data_selftest;
    uint64_t distance_phantom_jump1;
    uint64_t distance_phantom_jump2;
} inception_gadget_offsets;

#endif /*_INCEPTION_KMOD_KMOD_H */
