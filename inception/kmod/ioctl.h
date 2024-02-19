#ifndef _INCEPTION_KMOD_IOCTL_H
#define _INCEPTION_KMOD_IOCTL_H

#include "kmod.h"

#define PROC_INCEPTION "inception_test_kmod"

#ifndef __KERNEL__
static int fd_inception;
#define inception_kmod_init() do {\
    fd_inception = open("/proc/" PROC_INCEPTION, O_RDONLY);\
    if (fd_inception <= 0) {\
        err(fd_inception, "inception_kmod module missing");\
    }\
} while(0)

#define TRIGGER_INCEPTION_IOCTL(buffer) do {\
    if (ioctl(fd_inception, REQ_TRIGGER_INCEPTION, buffer) < 0) { err(1, "trigger_inception failed\n\t"); }\
} while(0)

#define READ_GADGET_OFFSETS_IOCTL(buffer) do {\
    if (ioctl(fd_inception, REQ_READ_GADGET_OFFSETS, buffer) < 0) { err(1, "read_gadget_offsets failed\n\t"); }\
} while(0)

#define READ_PHYS_ADDR_IOCTL(buffer) do {\
    if (ioctl(fd_inception, REQ_READ_PHYS_ADDR, buffer) < 0) { err(1, "read_phys_addr failed\n\t"); }\
} while(0)

#define TRIGGER_INCEPTION_SELFTEST_IOCTL(buffer) do {\
    if (ioctl(fd_inception, REQ_TRIGGER_INCEPTION_SELFTEST, buffer) < 0) { err(1, "trigger_inception failed\n\t"); }\
} while(0)

#endif

#endif /*_INCEPTION_KMOD_IOCTL_H */
