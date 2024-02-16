#include <pthread.h>
#include <sys/prctl.h>

pid_t run_sibling_noise(char path[], int core) {
    pid_t child = fork();
    if (child == 0) {
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        char core_str[2];
        sprintf(core_str, "%d", core);
        execl("/usr/bin/taskset", "taskset", "-c", core_str, path, NULL);
    }

    return child;
}

void set_cpu_affinity(int core) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core, &mask);
    if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
        printf("Unable to pin at core %d\n", core);
    }
}

void set_cpu_affinity2(int core, int core2) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core, &mask);
    CPU_SET(core2, &mask);
    if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
        printf("Unable to pin at core %d\n", core);
    }
}