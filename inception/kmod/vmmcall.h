#ifndef _INCEPTION_KMOD_VMMCALL_H
#define _INCEPTION_KMOD_VMMCALL_H

#include "kmod.h"

#define TRIGGER_INCEPTION_VMMCALL(buffer) do {\
    asm volatile( \
        "movq %0, %%rbx\t\n" \
        "movq %1, %%rax\t\n" \
        "vmmcall\t\n" \
        :: "r"(buffer), "r"((uint64_t)REQ_TRIGGER_INCEPTION) \
        : "%rax", "%rbx"); \
} while(0)

#define READ_GADGET_OFFSETS_VMMCALL(buffer) do {\
    asm volatile( \
        "movq %0, %%rbx\t\n" \
        "movq %1, %%rax\t\n" \
        "vmmcall\t\n" \
        :: "r"(buffer), "r"((uint64_t)REQ_READ_GADGET_OFFSETS) \
        : "%rax", "%rbx"); \
} while(0)

#define READ_PHYS_ADDR_VMMCALL(buffer) do {\
    asm volatile( \
        "movq %0, %%rbx\t\n" \
        "movq %1, %%rax\t\n" \
        "vmmcall\t\n" \
        :: "r"(buffer), "r"((uint64_t)REQ_READ_PHYS_ADDR) \
        : "%rax", "%rbx"); \
} while(0)

#endif /*_INCEPTION_KMOD_VMMCALL_H */
