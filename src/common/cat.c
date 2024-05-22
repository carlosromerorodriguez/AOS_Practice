#include "cat.h"
#include "../ext2/ext2_reader.h"
#include "../fat16/fat16_reader.h"
#include "tree.h"

/**
 * @brief Displays the contents of a file using the cat command.
 * 
 * @param fd File descriptor of the file system.
 * @param fileName Name of the file to display.
 * 
 * @return void
*/
void cat_command(int fd, char* fileName) {
    printf("---- Cat Command ----\n\n");

    //Check if the file system is ext2 or fat16
    if (is_ext2(fd)) {
        Ext2Superblock superblock;
        if (read_ext2_superblock(fd, &superblock) != 0) {
            perror("Error reading superblock");
            close(fd);
            exit(-1);
        }
        if (!cat_ext2(fd, 2, &superblock, fileName, 2, 2)) {
            printf("File not found.\n");
        }
    } else if (is_fat16(fd)) {
        BootSector bootSector;
        read_boot_sector(fd, &bootSector);

        fat16_recursion_tree(fd, bootSector, 0, fileName);
    } else {
        printf("Invalid file system.\n");
    }
}
