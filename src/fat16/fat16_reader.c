#include "fat16_reader.h"

#define FAT16_OFFSET 54
#define FAT16_LENGTH 8
#define FAT16_ID "FAT16   "

#define ENTRY_SIZE 32
#define ATTR_DIRECTORY 0x10
#define END_OF_DIRECTORY 0x00

bool is_fat16(int fd) {
    char type_fat16[FAT16_LENGTH + 1];  // + 1 -> '\0'

    // lseek para mover el offset al lugar correcto
    if (lseek(fd, FAT16_OFFSET, SEEK_SET) == (off_t)-1) {
        perror("Error seeking in file");
        return false;
    }

    // read para leer los datos del archivo
    ssize_t bytes_read = read(fd, type_fat16, FAT16_LENGTH);
    if (bytes_read < FAT16_LENGTH) {
        // Error de lectura o no hay suficientes bytes (puede ser el final del archivo)
        return false;
    }

    type_fat16[FAT16_LENGTH] = '\0';

    if (strcmp(FAT16_ID, type_fat16) == 0) {
        return true;
    }

    return false;
}

void read_boot_sector(int fd, BootSector *bootSector) {
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("Error seeking to start of the file");
        exit(EXIT_FAILURE);
    }

    // Leer el sector de arranque usando read en lugar de fread
    if (read(fd, bootSector, sizeof(BootSector)) != sizeof(BootSector)) {
        perror("Error reading boot sector");
        exit(EXIT_FAILURE);
    }
}


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

// Calcula la dirección del directorio raíz
unsigned int root_dir_address(const BootSector *bootSector) {
    return (bootSector->reserved_sectors + bootSector->number_of_fats * bootSector->fat_size_sectors) * bootSector->sector_size;
}

// Función para leer las entradas del directorio raíz
void read_root_dir(int fd, const BootSector *bootSector, void (*processEntry)(const DirEntry*, int)) {
    unsigned int address = root_dir_address(bootSector);
    if (lseek(fd, address, SEEK_SET) == (off_t)-1) {
        perror("Error seeking to root directory");
        exit(EXIT_FAILURE);
    }

    DirEntry dir;
    for (int i = 0; i < bootSector->root_dir_entries; i++) {
        ssize_t bytes_read = read(fd, &dir, sizeof(DirEntry));
        if (bytes_read < 0) {
            perror("Error reading directory entry");
            exit(EXIT_FAILURE);
        }
        if (bytes_read == 0 || dir.filename[0] == 0x00) {
            break; // Final de las entradas del directorio
        }
        if (dir.filename[0] == 0xE5) {
            continue; // Entrada de directorio borrada
        }
        processEntry(&dir, 0); // Procesa cada entrada del directorio
    }
}

// Procesa cada entrada de directorio
void process_dir_entry(const DirEntry *entry, int level) {
    if (entry->attributes & ATTR_DIRECTORY) {
        // Es un directorio
        printf("%*s[%s]\n", level * 2, "", entry->filename);
    } else {
        // Es un archivo
        printf("%*s%s\n", level * 2, "", entry->filename);
    }
}