#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define EXT2_SUPERBLOCK_OFFSET 1024
#define EXT2_SUPERBLOCK_SIZE 1024
#define EXT2_MAGIC_OFFSET 56
#define EXT2_MAGIC 0xEF53
#define EXT2_ROOT_INODE 2

#pragma pack(push, 1)
typedef struct {
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t reserved_blocks;
    uint32_t free_blocks;
    uint32_t free_inodes;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t last_mount_time;
    uint32_t last_written_time;
    uint16_t mnt_count;
    uint16_t max_mnt_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t last_check;
    uint32_t check_interval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;

    // Extended Superblock Fields
    uint32_t first_non_reserved_inode;
    uint16_t inode_size;
    uint16_t block_group_number;
    uint32_t feature_optional;
    uint32_t feature_required;
    uint32_t feature_ro_compat;
    char volume_name[16];
    char last_mounted_path[64];
    uint32_t compression_algorithms;
    uint8_t prealloc_blocks;
    uint8_t prealloc_dir_blocks;
    uint16_t unused;
    char filesystem_id[16];
    char journal_id[16];
    uint32_t journal_inode;
    uint32_t journal_device;
    uint32_t orphan_inode_list_head;
} Ext2Superblock;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct 
{
    uint16_t mode;
    uint16_t uid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t flags;
    uint32_t osd1;
    uint32_t block[15];
    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint32_t osd2[3];
} 
Ext2Inode;
#pragma pack(pop)

#pragma pack(push, 1)  
typedef struct 
{
        uint32_t inode;
        uint16_t rec_len;
        uint8_t name_len;
        uint8_t file_type;
        char name[];
} 
Ext2DirectoryEntry;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct 
{
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint16_t pad;
    uint32_t reserved[3];
} 
Ext2GroupDesc;
#pragma pack(pop)

/**
 * @brief Checks if the file system is an EXT2 file system.
 * 
 * @param fd File descriptor of the file system.
 * 
 * @return 1 if the file system is EXT2, 0 otherwise.
*/
int is_ext2(int fd);

/*
 * @brief Reads the superblock of an EXT2 file system.
 * @param fd File descriptor of the EXT2 file system.
 * @param superblock Pointer to the superblock structure to fill.
 */
int read_ext2_superblock(int fd, Ext2Superblock *superblock);

/*
    * @brief Reads a group descriptor from the block group descriptor table.
    * @param fd File descriptor of the EXT2 file system.
    * @param superblock Superblock of the EXT2 file system.
    * @param group_num Number of the group descriptor to read.
    * @param group_desc Pointer to the group descriptor structure to fill.
 */
int read_ext2_group_desc(int fd, Ext2Superblock *superblock, uint32_t group_num, Ext2GroupDesc *group_desc);

/*
    * @brief Reads an inode from the inode table.
    * @param fd File descriptor of the EXT2 file system.
    * @param superblock Superblock of the EXT2 file system.
    * @param inode_num Number of the inode to read.
    * @param inode Pointer to the inode structure to fill.
 */
int read_ext2_inode(int fd, Ext2Superblock *superblock, uint32_t inode_num, Ext2Inode *inode);

/*
    * @brief Reads a directory from the inode.
    * @param fd File descriptor of the EXT2 file system.
    * @param superblock Superblock of the EXT2 file system.
    * @param inode Inode of the directory to read.
    * @param entries Pointer to the directory entries structure to fill.
 */
int read_ext2_directory(int fd, Ext2Superblock *superblock, Ext2Inode *inode, Ext2DirectoryEntry *entries);

/*
    * @brief Displays the contents of a file.
    * @param fd File descriptor of the EXT2 file system.
    * @param inode_num Number of the inode to display.
    * @param superblock Superblock of the EXT2 file system.
    * @param filename Name of the file to display.
 */
void cat_ext2(int fd, uint32_t inode_num, Ext2Superblock *superblock, char* filename, uint32_t current_inode, uint32_t parent_inode);


/*
    * @brief shows the tree representation of the directory structure of the file system.
    * @param fd File descriptor of the EXT2 file system.
    * @param inode_num Inode number of the directory to display.
    * @param superblock Superblock of the EXT2 file system.
    * @param level Level of the directory in the tree.
 */
void dfs_ext2(int fd, uint32_t inode_num, Ext2Superblock *superblock, int level, uint32_t current_inode, uint32_t parent_inode);