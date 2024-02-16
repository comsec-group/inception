#include "./kmod_ibpb_cost/ibpb_cost_ioc.h"
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

static int fd_ibpb_cost;
static struct ibpb_cost_ioc_msg msg;

#define T 1000000
unsigned long m[T];

int cmp(const void *a, const void *b) {
    return *(unsigned long *) a - *(unsigned long *) b;
}

unsigned long median(unsigned long *vals, int count) {
    qsort(m, count, 8, cmp);
    return m[count / 2];
}

unsigned long min(unsigned long *vals, int count) {
    qsort(m, count, 8, cmp);
    return m[0];
}
unsigned long max(unsigned long *vals, int count) {
    qsort(m, count, 8, cmp);
    return m[count - 1];
}

unsigned long avg(unsigned long *vals, int count) {
    unsigned long sum = 0;
    for (int i = 0; i < count; ++i) {
        sum += m[i];
    }
    return sum / count;
}

int main(int argc, char *argv[]) {
    fd_ibpb_cost = open("/proc/" PROC_IBPB_COST, O_RDONLY);
    if (fd_ibpb_cost <= 0) {
        err(fd_ibpb_cost, "/proc/" PROC_IBPB_COST " missing");
    }
    if (ioctl(fd_ibpb_cost, IBPB_COST_IOC_HELLO, &msg)) {
        err(1, "ioctl");
    }

    for (int i = 0; i < T; ++i) {
        if (ioctl(fd_ibpb_cost, IBPB_COST_IOC_DRY, &msg)) {
            err(1, "ioctl");
        }
        /* printf("dry cycles: %ld\n", msg.result); */
        m[i] = msg.result;
    }

    unsigned long med = median(m, T);
    printf("measurement overhead: %ld\n", med);

    for (int i = 0; i < T; ++i) {
        if (ioctl(fd_ibpb_cost, IBPB_COST_IOC_MEASSURE, &msg)) {
            err(1, "ioctl");
        }
        /* printf("ibpb cycles: %ld\n", msg.result); */
        m[i] = msg.result;
    }

    printf("ibpb cost (median): %ld\n", median(m, T) - med);
    printf("ibpb cost (avg): %ld\n", avg(m, T) - med);
    printf("ibpb cost (min): %ld\n", min(m, T) - med);
    printf("ibpb cost (max): %ld\n", max(m, T) - med);
    return 0;
}
