#include "cat.h"
#include "../ext2/ext2_reader.h"
#include "../fat16/fat16_reader.h"
#include "tree.h"

// Function prototypes
void read_and_print_file_content(int fd, const BootSector bootSector, DirEntry entry);


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
        cat_ext2(fd, 2, &superblock, fileName, 2, 2);

    } else if (is_fat16(fd)) {
        BootSector bootSector;
        read_boot_sector(fd, &bootSector);

        DirEntry entry = fat16_recursion_tree(fd, bootSector, get_root_dir_offset(bootSector), 0, 0, fileName);
        if (strcmp(entry.filename, "") != 0) {
            printf("File found: '%.8s.%.3s'\n", entry.filename, entry.ext);
            read_and_print_file_content(fd, bootSector, entry);
        } else {
            printf("File not found.\n");
        }
    } else {
        printf("Invalid file system.\n");
    }
}

/**
 * @brief Reads and prints the content of a file.
 * 
 * @param fd File descriptor of the file system.
 * @param bootSector Boot sector of the file system.
 * @param entry Directory entry of the file.
 * 
 * @return void
*/
void read_and_print_file_content(int fd, const BootSector bootSector, DirEntry entry) {
    unsigned int offset = get_first_cluster(bootSector) + (entry.startCluster - 2) * bootSector.sectors_per_cluster * bootSector.sector_size;
    char buffer[513];
    int bytesRead, bytesToRead;
    unsigned int totalBytesRead = 0;

    while (totalBytesRead < entry.fileSize && entry.startCluster < 0xFFF8) { // 0xFFF8 is the last cluster
        lseek(fd, offset, SEEK_SET);
        bytesToRead = 512;
        bytesRead = read(fd, buffer, bytesToRead);
        if (bytesRead <= 0) break;

        buffer[bytesRead] = '\0';
        printf("%s", buffer);
        totalBytesRead += bytesRead;

        // Get the next cluster
        if (bytesRead == bytesToRead && totalBytesRead <= entry.fileSize) {
            entry.startCluster = get_next_cluster(fd, entry.startCluster, bootSector);
            if (entry.startCluster < 0xFFF8) {
                offset = get_first_cluster(bootSector) + (entry.startCluster - 2) * bootSector.sectors_per_cluster * bootSector.sector_size;
            }
        }
    }

    if (totalBytesRead < entry.fileSize) {
        // Read the last bytes and print them
        lseek(fd, offset, SEEK_SET);
        bytesRead = read(fd, buffer, entry.fileSize - totalBytesRead);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            printf("%s", buffer);
            totalBytesRead += bytesRead;
        }
    }
}
