#include "tree.h"
#include "../ext2/ext2_reader.h"

/*
    * @brief Prints a line of the tree representation of the directory structure.
    * @param level Level of the tree where the line will be printed.
    * @param name Name of the entry to print.
    * @param is_last_entry Flag indicating if the entry is the last one in the directory.
 */
void print_tree_line(int level, const char* name, int is_last_entry) {
    static int levels[100] = {0}; // Array per rastrejar els nivells actius
    
    for (int i = 0; i < level; ++i) {
        printf("%s", levels[i] ? "│   " : "    ");
    }

    printf("%s── %s\n", is_last_entry ? "└" : "├", name);

    levels[level] = !is_last_entry; // Establim el nivell actual com a actiu o no
    
}

/*
    * @brief Performs a depth-first search of the EXT2 file system.
    * @param fd File descriptor of the EXT2 file system.
    * @param inode_num Number of the inode to start the search from.
    * @param superblock Superblock of the EXT2 file system.
    * @param level Level of the tree where the search is currently at.
 */
void dfs_ext2(int fd, uint32_t inode_num, Ext2Superblock *superblock, int level) {
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

                uint32_t next_offset = offset + entry->rec_len;
                uint32_t is_current_last_entry = (next_offset >= block_size) || (((Ext2DirectoryEntry *)((char *)entries + next_offset))->inode == 0);

                if (strcmp(entry->name, ".") != 0 && strcmp(entry->name, "..") != 0 && strcmp(entry->name, "lost+found") != 0) {
                    print_tree_line(level, entry->name, is_current_last_entry);
                }

                // Explorem recursivament si és un directori i no és '.' ni '..'
                if (entry->file_type == 2 && strcmp(entry->name, ".") != 0 && strcmp(entry->name, "..") != 0) {
                    dfs_ext2(fd, entry->inode, superblock, level + 1);
                }
            }
            offset += entry->rec_len; // Ens movem a la següent entrada
        }

        free(entries); // Alliberem la memòria de les entrades
    }
}

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
        dfs_ext2(fd, 2, &superblock, 0);
    } else if (is_fat16(fd)) {
        BootSector bootSector;
        read_boot_sector(fd, &bootSector);
        DirEntry entry = fat16_recursion_tree(fd, bootSector, get_root_dir_offset(bootSector), 0, 1, "");
        printf("%s└── End of directory\n", entry.filename);
    } else {
         printf("Unknown file system\n");
    }
}