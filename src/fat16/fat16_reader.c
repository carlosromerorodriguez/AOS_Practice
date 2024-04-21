#include "fat16_reader.h"

/**
 * Checks if the file system is FAT16 by reading the boot sector.
 * 
 * @param fd File descriptor of the file system.
 * 
 * @return true if the file system is FAT16, false otherwise.
*/
bool is_fat16(int fd) {
    BootSector bs;
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1 || read(fd, &bs, sizeof(BootSector)) != sizeof(BootSector)) {
        perror("Error reading boot sector");
        return false;
    }

    uint32_t total_sectors = bs.total_sectors_short != 0 ? bs.total_sectors_short : bs.total_sectors_long;
    uint32_t fat_size = bs.fat_size_sectors != 0 ? bs.fat_size_sectors : bs.total_sectors_long;
    uint32_t root_dir_sectors = ((bs.root_dir_entries * 32) + (bs.sector_size - 1)) / bs.sector_size;
    uint32_t data_sectors = total_sectors - (bs.reserved_sectors + (bs.number_of_fats * fat_size) + root_dir_sectors);
    uint32_t total_clusters = data_sectors / bs.sectors_per_cluster;

    return total_clusters >= 4086 && total_clusters < 65526;    
}

/**
 * Reads the boot sector of the file system. 
 * 
 * @param fd File descriptor of the file system.
 * @param bootSector Pointer to the boot sector structure to store the boot sector information.
 * 
 * @return void
*/
void read_boot_sector(int fd, BootSector *bootSector) {
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("Error seeking to start of the file");
        exit(EXIT_FAILURE);
    }

    if (read(fd, bootSector, sizeof(BootSector)) != sizeof(BootSector)) {
        perror("Error reading boot sector");
        exit(EXIT_FAILURE);
    }
}

/**
 * Prints the boot sector information.
 * 
 * @param bootSector Pointer to the boot sector structure.
 * 
 * @return void
*/
void print_boot_sector(const BootSector *bootSector) {
    printf("Filesystem: FAT16\n\n");
    printf("System name: %.8s\n", bootSector->oem);
    printf("Sector size: %u bytes\n", bootSector->sector_size);
    printf("Sectors per cluster: %u\n", bootSector->sectors_per_cluster);
    printf("Reserved Sectors: %u\n", bootSector->reserved_sectors);
    printf("# of FATs: %u\n", bootSector->number_of_fats);
    printf("Max root entries: %u\n", bootSector->root_dir_entries);
    printf("Sector per FAT: %u\n", bootSector->fat_size_sectors);
    printf("Label: %.11s\n", bootSector->volume_label);
}

/**
 * Hets the root directory offset of the file system.
 * 
 * @param bootSector Boot sector of the file system.
 * 
 * @return the root directory offset of the file system.
*/
int get_root_dir_offset(BootSector bootSector) {
    return (bootSector.reserved_sectors * bootSector.sector_size) + (bootSector.number_of_fats * bootSector.fat_size_sectors) * bootSector.sector_size;
}

/**
 * Gets the next cluster of the file. 
 * 
 * @param fd File descriptor of the file system.
 * @param current_cluster Current cluster of the file.
 * @param bootSector Boot sector of the file system.
 * 
 * @return the next cluster of the file.
*/
unsigned int get_next_cluster(int fd, unsigned int current_cluster, const BootSector bootSector) {
    unsigned int fat_offset = bootSector.reserved_sectors * bootSector.sector_size + current_cluster * 2;
    uint16_t next_cluster;
    lseek(fd, fat_offset, SEEK_SET);
    read(fd, &next_cluster, sizeof(next_cluster));

    if (next_cluster >= 0xFFF8) { // Final de la cadena de clústeres
        return 0xFFFF;
    }
    return next_cluster;
}


/**
 * Gets the first cluster of the file.
 * 
 * @param bootSector Boot sector of the file system.
 * 
 * @return the first cluster of the file.
*/
unsigned int get_first_cluster(const BootSector bootSector) {
     return get_root_dir_offset(bootSector) + bootSector.root_dir_entries * sizeof(DirEntry);
}

/**
 * Reads a directory entry from the file system.
 * 
 * @param fd File descriptor of the file system.
 * @param offset Offset of the directory entry.
 * 
 * @return the directory entry.
*/
DirEntry read_directory_entry(int fd, int offset) {
    DirEntry entry;
    if (lseek(fd, offset, SEEK_SET) < 0) {
        perror("Error seeking to directory entry");
        exit(EXIT_FAILURE);
    }

    if (read(fd, &entry, sizeof(DirEntry)) != sizeof(DirEntry)) {
        perror("Error reading directory entry");
        exit(EXIT_FAILURE);
    }

    return entry;
}

/**
 * Checks if the directory entry is a directory.
 * 
 * @param entry Directory entry to check.
 * 
 * @return 1 if the directory entry is a directory, 0 otherwise.
*/
unsigned char it_is_a_directory(DirEntry entry) {
    if (entry.filename[0] == '.' &&
        (entry.filename[1] == ' ' || entry.filename[1] == '\0' || entry.filename[1] == '.')) {
        return 0;
    }

    return entry.attributes == 0x10;
}

/**
 * Removes the characters from the filename until the character c is found.
 * 
 * @param filename Filename to remove the characters.
 * @param c Character to find.
 * 
 * @return void
*/
void remove_until_char(char *filename, char c) {
    for (int i = 0; i < 8; i++) {
        if (filename[i] == c) {
            filename[i] = '\0';
            break;
        }
    }
}

/**
 * Recursively reads the directory structure of the file system and prints it in a tree format.
 * 
 * @param fd File descriptor of the file system.
 * @param bootSector Boot sector of the file system.
 * @param offset Offset of the directory entry.
 * @param depth Depth of the directory.
 * @param tree_not_cat Flag to print the tree or to find a file.
 * @param filename_to_find Filename to find.
 * 
 * @return the directory entry of the file.
*/
DirEntry fat16_recursion_tree(int fd, const BootSector bootSector, int offset, int depth, int tree_not_cat, char *filename_to_find) {
    int new_offset;
    DirEntry entry;
    char fullname[14];
    char indent[10] = {0};

    for (int i = 0; i < depth; i++) {
        strcat(indent, "│  ");
    }

    for (int i = 0; i < bootSector.root_dir_entries; i++) {
        lseek(fd, offset, SEEK_SET);
        entry = read_directory_entry(fd, offset);
        offset += sizeof(DirEntry);

        // Se salta el nombre del directorio raiz
        if (i == 0) {
            continue;
        }

        if (entry.attributes == 0x0) {
            break;
        }

        // Si la entrada del directorio habia sido eliminada previamente o es un archivo de sistema, se ignora
        if (entry.attributes == 0xF || entry.filename[0] == (char)229) {
            continue;
        }    

        if (entry.filename[0] == '.' || (strlen(entry.filename) == 2 && (entry.filename[0] == '.' && entry.filename[1] == '.')) || entry.filename[0] == ' ') {
            continue;
        }

        remove_until_char(entry.filename, ' ');
        remove_until_char(entry.filename, '~');

        // If not found, check if directory
        if (it_is_a_directory(entry)) {
            if (tree_not_cat) {
                printf("%s├── [%.8s]\n", indent, entry.filename);
            } else {
                if (strcmp(entry.filename, filename_to_find) == 0) {
                    return entry;
                }
            }

            // Read Inside Directory
            new_offset = get_first_cluster(bootSector) + (entry.startCluster - 2) * bootSector.sectors_per_cluster * bootSector.sector_size;
            DirEntry foundEntry = fat16_recursion_tree(fd, bootSector, new_offset, depth + 1, tree_not_cat, filename_to_find);
            remove_until_char(foundEntry.filename, ' ');
            remove_until_char(foundEntry.filename, '~');

            if (!tree_not_cat) {
                if (entry.ext[0] == ' ' || entry.ext[0] == '8') {
                    if (!strcmp(foundEntry.filename, filename_to_find)) {
                        return foundEntry;
                    }
                } else {
                    remove_until_char(foundEntry.ext, ' ');
                    if (foundEntry.ext[0] == '\0') {
                        snprintf(fullname, 14, "%.8s", foundEntry.filename);
                    } else {
                        snprintf(fullname, 14, "%.8s.%.3s", foundEntry.filename, foundEntry.ext);
                    }
                    if (!strcmp(fullname, filename_to_find)) {
                        return foundEntry;
                    }
                }
            }
        } else {
            // It is a file
            if (tree_not_cat) {
                if (entry.ext[0] == ' ' || entry.ext[0] == '8') {
                    printf("%s├── %.8s\n", indent, entry.filename);
                } else {
                    printf("%s├── %.8s.%s\n", indent, entry.filename, entry.ext);
                }
                continue;
            } 

            if (entry.ext[0] == ' ' || entry.ext[0] == '8') {
                if (!strcmp(entry.filename, filename_to_find)) {
                    return entry;
                }
            } else {
                remove_until_char(entry.ext, ' ');
                if (entry.ext[0] == '\0') {
                        snprintf(fullname, 14, "%.8s", entry.filename);
                    } else {
                        snprintf(fullname, 14, "%.8s.%.3s", entry.filename, entry.ext);
                    }
                if (!strcmp(fullname, filename_to_find)) {
                    return entry;
                }
            }
        }
    }

    return entry;
}
