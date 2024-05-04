#ifndef _FAT16_READER_H
#define _FAT16_READER_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>

// Marcadores de inicio y final de nombre de archivo 
#define DIR_ENTRY_FREE   0xE5
#define DIR_ENTRY_EMPTY  0x00

// Entrada de directorio actual
#define CURRENT_DIR_ENTRY 0x2E

// Caracteres especiales
#define KANJI_SPECIAL_CASE 0x05

// Espacio de relleno
#define SPACE_PAD 0x20

// Lista de caracteres prohibidos
#define ILLEGAL_CHARS "\x22\x2A\x2B\x2C\x2E\x2F\x3A\x3B\x3C\x3D\x3E\x3F\x5B\x5C\x5D\x7C"

// Atributos de archivo
#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

// Colores
#define ANSI_COLOR_YELLOW   "\x1b[33m"
#define ANSI_COLOR_RESET    "\x1b[0m"


// Estructura para el sector de arranque
typedef struct {
    uint8_t jmp[3];
    char oem[8];
    uint16_t sector_size;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t number_of_fats;
    uint16_t root_dir_entries;
    uint16_t total_sectors_16;
    uint8_t media_descriptor;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;

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
    unsigned char filename[11];
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
 * @return 1 if the file system is FAT16, 0 otherwise.
*/
int is_fat16(int fd);

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

#endif // !_FAT16_READER_H