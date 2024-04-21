#ifndef _FAT16_READER_H
#define _FAT16_READER_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

// Estructura para el sector de arranque
typedef struct {
    uint8_t jmp[3];
    char oem[8];
    uint16_t sector_size;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t number_of_fats;
    uint16_t root_dir_entries;
    uint16_t total_sectors_short; // if zero, later field is used
    uint8_t media_descriptor;
    uint16_t fat_size_sectors;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_long;

    // Extended Boot Record fields
    uint8_t drive_number;
    uint8_t current_head;
    uint8_t boot_signature;
    uint32_t volume_id;
    char volume_label[11];
    char fs_type[8];
    uint8_t boot_code[448];
    uint16_t boot_sector_signature;
} __attribute__((packed)) BootSector;

// Estructura para una entrada de directorio
typedef struct {
    char filename[8];
    char ext[3];
    char attributes;
    char reservedNT;
    char creationTimeTenth;
    unsigned short createTime;
    unsigned short createDate;
    unsigned short accessDate;
    unsigned short reservedFAT32;
    unsigned short writeTime;
    unsigned short writeDate;
    unsigned short startCluster;
    unsigned int fileSize;
} __attribute__((packed)) DirEntry;

/**
 * Checks if the file system is FAT16 by reading the boot sector.
 * 
 * @param fd File descriptor of the file system.
 * 
 * @return true if the file system is FAT16, false otherwise.
*/
bool is_fat16(int fd);

/**
 * Reads the boot sector of the file system. 
 * 
 * @param fd File descriptor of the file system.
 * @param bootSector Pointer to the boot sector structure to store the boot sector information.
 * 
 * @return void
*/
void read_boot_sector(int fd, BootSector *bootSector);

/**
 * Prints the boot sector information.
 * 
 * @param bootSector Pointer to the boot sector structure.
 * 
 * @return void
*/
void print_boot_sector(const BootSector *bootSector);

/**
 * Hets the root directory offset of the file system.
 * 
 * @param bootSector Boot sector of the file system.
 * 
 * @return the root directory offset of the file system.
*/
int get_root_dir_offset(BootSector bootSector);

/**
 * Gets the first cluster of the file.
 * 
 * @param bootSector Boot sector of the file system.
 * 
 * @return the first cluster of the file.
*/
unsigned int get_first_cluster(const BootSector bootSector);

/**
 * Gets the next cluster of the file. 
 * 
 * @param fd File descriptor of the file system.
 * @param current_cluster Current cluster of the file.
 * @param bootSector Boot sector of the file system.
 * 
 * @return the next cluster of the file.
*/
unsigned int get_next_cluster(int fd, unsigned int cluster, const BootSector bootSector);

#endif // !_FAT16_READER_H