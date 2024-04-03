#include "ext2_reader.h"

int is_ext2(int fd) {
    uint16_t magic;

    // Busquem el magic number de l'ext2
    if (lseek(fd, EXT2_SUPERBLOCK_OFFSET + EXT2_MAGIC_OFFSET, SEEK_SET) == -1) {
        perror("Error seeking to superblock");
        close(fd);
        exit(1);
    }

    // Llegim el magic number
    if (read(fd, &magic, sizeof(magic)) == -1) {
        perror("Error reading magic number");
        close(fd);
        exit(1);
    }

    // Mirem si el magic number és el de l'ext2
    if (magic == EXT2_MAGIC) {
        return 1;  // It's an EXT2 file system
    }
    
    return 0;  // It's not an EXT2 file system
}

int read_ext2_superblock(int fd, Ext2Superblock *superblock) {
    if (lseek(fd, EXT2_SUPERBLOCK_OFFSET, SEEK_SET) != EXT2_SUPERBLOCK_OFFSET) {
        return -1;
    }

    if (read(fd, superblock, sizeof(Ext2Superblock)) != sizeof(Ext2Superblock)) {
        return -1;
    }
    return 0;
}