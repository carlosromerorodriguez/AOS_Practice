#include "tree.h"

void print_file_tree(int fd) {
    BootSector bootSector;
    read_boot_sector(fd, &bootSector);
    print_boot_sector(&bootSector);

    if (is_fat16(fd)) {
        read_root_dir(fd, &bootSector, process_dir_entry);
    } else {
        printf("Unknown file system\n");
    }
}