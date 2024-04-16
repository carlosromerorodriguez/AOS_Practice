#include "tree.h"

void print_file_tree(int fd) {
    BootSector bootSector;
    read_boot_sector(fd, &bootSector);
    //print_boot_sector(&bootSector);

    if (is_fat16(fd)) {
        print_directory(fd, &bootSector, 0, process_dir_entry);
    } else {
        printf("Unknown file system\n");
    }
}