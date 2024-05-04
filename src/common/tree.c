#include "tree.h"
#include "../ext2/ext2_reader.h"

/**
 * @brief Prints the tree representation of the directory structure of the file system.
 * 
 * @param fd File descriptor of the file system.
 * 
 * @return void
*/
void print_file_tree(int fd) {
    if (is_ext2(fd)) {
        Ext2Superblock superblock;
        read_ext2_superblock(fd, &superblock);
        dfs_ext2(fd, 2, &superblock, 0, 2, 2);
    } else if (is_fat16(fd)) {
        BootSector bootSector;
        read_boot_sector(fd, &bootSector);
        fat16_recursion_tree(fd, bootSector, 1, "");
    } else {
         printf("Unknown file system\n");
    }
}
