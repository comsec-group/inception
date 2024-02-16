#include <dirent.h>
#include <fcntl.h>

#define PAGE 4096

static struct {
    int nblocks;
    uint64_t block_size;
    int blocks[1024];
} phys_blocks;

static void phys_blocks_init() {
#define BLOCK_SIZE_BYTES "/sys/devices/system/memory/block_size_bytes"
#define PATH_MEM         "/sys/devices/system/memory"
    int fd_blocksz = open(BLOCK_SIZE_BYTES, O_RDONLY);
    char buf[1024];
    ssize_t len = read(fd_blocksz, buf, sizeof(buf) - 1);
    buf[len] = 0;
    phys_blocks.block_size = strtol(buf, NULL, 16);
    close(fd_blocksz);

    int fd_mem = open(PATH_MEM, O_RDONLY | O_DIRECTORY);
    DIR *memdir = opendir(PATH_MEM);
    struct dirent *dent;
    phys_blocks.nblocks = 0;
    while ((dent = readdir(memdir)) != NULL) {
        int index;
        if (sscanf(dent->d_name, "memory%d", &index) == 0)
            continue;
        phys_blocks.blocks[phys_blocks.nblocks++] = index;
    }

    for (int i = 0; i < phys_blocks.nblocks; i++) {
        int rnd = rand() % phys_blocks.nblocks;
        int tmp = phys_blocks.blocks[i];
        phys_blocks.blocks[i] = phys_blocks.blocks[rnd];
        phys_blocks.blocks[rnd] = tmp;
    }

    int phys_blocks_cpy[1024];
    for (int i = 0; i < phys_blocks.nblocks; i++) {
        phys_blocks_cpy[i] = phys_blocks.blocks[i];
    }

    close(fd_mem);
}
