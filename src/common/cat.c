#include "cat.h"
#include "../ext2/ext2_reader.h"
#include "../fat16/fat16_reader.h"
#include "tree.h"

// Function prototypes
void read_and_print_file_content(int fd, const BootSector bootSector, DirEntry entry);

/*
    * @brief Displays the contents of a file.
    * @param fd File descriptor of the EXT2 file system.
    * @param inode_num Number of the inode to display.
    * @param superblock Superblock of the EXT2 file system.
    * @param filename Name of the file to display.
 */
void cat_ext2(int fd, uint32_t inode_num, Ext2Superblock *superblock, char* filename) {
    Ext2Inode inode; // Ínode actual en el que estem
    Ext2DirectoryEntry *entries; // Entrades del directori
    // Mida del bloc, shiftem 1024 a l'esquerra per obtenir la mida del bloc ja que el superblock ens dona la mida del bloc en potencies de 2
    uint32_t block_size = 1024 << superblock->log_block_size; 

    // LLegim l'ínode
    read_ext2_inode(fd, superblock, inode_num, &inode);

    // Comprovem si l'ínode és un directori
    if (inode.mode & 0x4000) { 
        // Llegim les entrades del directori
        entries = (Ext2DirectoryEntry *) malloc(block_size); // Reservem memòria per les entrades
        read_ext2_directory(fd, superblock, &inode, entries); // Llegim les entrades del directori

        // Per cada entrada del directori
        for (uint32_t offset = 0; offset < block_size; ) {
            Ext2DirectoryEntry *entry = (Ext2DirectoryEntry *)((char *)entries + offset);
            if (entry->inode != 0) { // Si l'entrada no és buida
                // Mostrem el nom de l'entrada

                char* entry_name = (char *)malloc(entry->name_len + 1);
                memcpy(entry_name, entry->name, entry->name_len);
                entry_name[entry->name_len] = '\0';

                if(strcmp(entry_name, filename) == 0){
                    printf("File found: '%s'\n", entry->name);
                    Ext2Inode file_inode; // Ínode del fitxer
                    read_ext2_inode(fd, superblock, entry->inode, &file_inode); // Llegim l'ínode del fitxer
                    cat_ext2_file(fd, &file_inode, block_size); // Mostrem el contingut del fitxer
                }

                // Explorem recursivament si és un directori i no és '.' ni '..'
                if (entry->file_type == 2 && strcmp(entry->name, ".") != 0 && strcmp(entry->name, "..") != 0) {
                    cat_ext2(fd, entry->inode, superblock, filename);
                }
            }
            offset += entry->rec_len; // Ens movem a la següent entrada
        }

        free(entries); // Alliberem la memòria de les entrades
    }
}

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
        cat_ext2(fd, 2, &superblock, fileName);

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
