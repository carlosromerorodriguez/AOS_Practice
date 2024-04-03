/*void cat_command(int fd) {
    printf("---- Cat Command ----\n\n");

    //Check if the file system is ext2 or fat16
    if (is_ext2(fd)) {
        //Print the superblock information
        cat_ext2(fd);
    } else if (is_fat16()) {
        //Print the boot sector information
        print_fat16_boot_sector();
    } else {
        printf("Invalid file system.\n");
    }
}

int read_ext2_inode(int fd, Ext2Superblock *superblock, uint32_t inode_num, Ext2Inode *inode) {
    uint32_t group_num = (inode_num - 1) / superblock->inodes_per_group;
    uint32_t inode_table_block = superblock->first_data_block + superblock->blocks_per_group * group_num;
    uint32_t inode_table_offset = (inode_num - 1) % superblock->inodes_per_group;
    uint32_t inode_offset = inode_table_block * superblock->block_size + inode_table_offset * superblock->inode_size;

    if (lseek(fd, inode_offset, SEEK_SET) != inode_offset) {
        perror("Error seeking to inode");
        return -1;
    }

    if (read(fd, inode, sizeof(Ext2Inode)) != sizeof(Ext2Inode)) {
        perror("Error reading inode");
        return -1;
    }

    return 0;
}

int read_ext2_directory(int fd, Ext2Superblock *superblock, Ext2Inode *inode, Ext2DirectoryEntry *entries) {
    uint32_t block_size = 1024 << superblock->log_block_size;
    uint32_t num_blocks = (inode->size + block_size - 1) / block_size;
    uint32_t block_num = 0;
    uint32_t block_offset = 0;

    for (int i = 0; i < num_blocks; i++) {
        block_num = inode->block[block_offset];
        block_offset++;

        if (lseek(fd, block_num * block_size, SEEK_SET) != block_num * block_size) {
            perror("Error seeking to block");
            return -1;
        }

        if (read(fd, entries + i * block_size, block_size) != block_size) {
            perror("Error reading block");
            return -1;
        }
    }

    return 0;
}

void cat_ext2(fd) {
    // Read the superblock
    Ext2Superblock superblock;
    if (read_ext2_superblock(fd, &superblock) < 0) {
        perror("Error reading superblock");
        close(fd);
        exit(1);
    }

    // Read the root inode
    Ext2Inode inode;
    if (read_ext2_inode(fd, &superblock, EXT2_ROOT_INODE, &inode) < 0) {
        perror("Error reading root inode");
        close(fd);
        exit(1);
    }

    // Allocate memory and read the root directory
    char *directory_entries = malloc(inode.size);
    if (read_ext2_directory(fd, &superblock, &inode, &directory_entries) < 0) {
        perror("Error reading root directory");
        free(directory_entries);
        close(fd);
        exit(1);
    }

    // Iterate over the directory entries and print them
    char *entry_ptr = directory_entries;
    while (entry_ptr < directory_entries + inode.size) {
        Ext2DirectoryEntry *entry = (Ext2DirectoryEntry *)entry_ptr;
        printf("%.*s\n", entry->name_len, entry->name);
        entry_ptr += entry->size; // Advance to the next directory entry
    }

    free(directory_entries);
}*/