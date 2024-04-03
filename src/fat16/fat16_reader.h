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
    unsigned char filename[8];
    char ext[3];
    uint8_t attributes;
    uint8_t reserved[10];
    uint16_t modify_time;
    uint16_t modify_date;
    uint16_t starting_cluster;
    uint32_t file_size;
} __attribute__((packed)) DirEntry;


bool is_fat16(int fd);

void read_boot_sector(int fd, BootSector *bootSector);

void print_boot_sector(const BootSector *bootSector);

#endif // !_FAT16_READER_H