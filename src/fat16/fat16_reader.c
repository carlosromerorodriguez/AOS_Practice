#include "fat16_reader.h"

#define FAT16_OFFSET 54
#define FAT16_LENGTH 8
#define FAT16_ID "FAT16   "

#define ENTRY_SIZE 32
#define ATTR_DIRECTORY 0x10
#define END_OF_DIRECTORY 0x00

bool isFat16(FILE *fd) {
    char type_fat16[FAT16_OFFSET + 1]; // + 1 -> '\0'

    fseek(fd, FAT16_OFFSET, SEEK_SET);

    if (fread(type_fat16, FAT16_LENGTH, 1, fd) <= 0) {
        return false;
    }

    type_fat16[FAT16_LENGTH] = '\0'; 
    
    if (!strcmp(FAT16_ID, type_fat16)) {
        return true;
    }

    return false;
}

void readBootSector(const char *filename, BootSector *bootSector) {
    FILE *disk = fopen(filename, "rb");
    if (disk == NULL) {
        fprintf(stderr, "Error opening the disk image file.\n");
        exit(EXIT_FAILURE);
    }

    if (fread(bootSector, sizeof(BootSector), 1, disk) != 1) {
        fprintf(stderr, "Error reading boot sector.\n");
        fclose(disk);
        exit(EXIT_FAILURE);
    }

    fclose(disk);
}


void printBootSector(const BootSector *bootSector) {
    printf("------ File System Information ------\n\nFilesystem: FAT16\n\n");
    printf("System name: %.8s\n", bootSector->oem);
    printf("Sector size: %u bytes\n", bootSector->sector_size);
    printf("Sectors per cluster: %u\n", bootSector->sectors_per_cluster);
    printf("Reserved Sectors: %u\n", bootSector->reserved_sectors);
    printf("# of FATs: %u\n", bootSector->number_of_fats);
    printf("Max root entries: %u\n", bootSector->root_dir_entries);
    printf("Sector per FAT: %u\n", bootSector->fat_size_sectors);
    printf("Label: %.11s\n", bootSector->volume_label);
}

// Calcula la dirección del directorio raíz
unsigned int rootDirAddress(const BootSector *bootSector) {
    return (bootSector->reserved_sectors + bootSector->number_of_fats * bootSector->fat_size_sectors) * bootSector->sector_size;
}

// Función para leer las entradas del directorio raíz
void readRootDir(FILE *fd, const BootSector *bootSector, void (*processEntry)(const DirEntry*, int)) {
    unsigned int address = rootDirAddress(bootSector);
    fseek(fd, address, SEEK_SET);

    for (int i = 0; i < bootSector->root_dir_entries; i++) {
        DirEntry dir;
        if (fread(&dir, sizeof(DirEntry), 1, fd) != 1) {
            break; // Error de lectura o final del directorio
        }

        if (dir.filename[0] == 0x00) {
            break; // Final de las entradas del directorio
        }

        if (dir.filename[0] == 0xE5) {
            continue; // Entrada de directorio borrada
        }

        processEntry(&dir, 0); // Procesa cada entrada del directorio
    }
}

// Procesa cada entrada de directorio
void processDirEntry(const DirEntry *entry, int level) {
    if (entry->attributes & ATTR_DIRECTORY) {
        // Es un directorio
        printf("%*s[%s]\n", level * 2, "", entry->filename);
    } else {
        // Es un archivo
        printf("%*s%s\n", level * 2, "", entry->filename);
    }
}