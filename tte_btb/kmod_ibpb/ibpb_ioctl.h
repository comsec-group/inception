#define REQ_IBPB 1340
#define PROC_IBPB "ibpb"

#ifndef __KERNEL__
static int fd_ibpb;
#define ibpb_init() do {\
    fd_ibpb = open("/proc/" PROC_IBPB, O_RDONLY);\
    if (fd_ibpb <= 0) {\
        err(fd_ibpb, "ibpb module missing");\
    }\
} while(0)

#define IBPB() do {\
    if (ioctl(fd_ibpb, REQ_IBPB) < 0) { err(1, "ibpb failed\n\t"); }\
} while(0)

#endif
