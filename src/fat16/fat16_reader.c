#include "fat16_reader.h"

#define FAT16_OFFSET 54
#define FAT16_LENGTH 8
#define FAT16_ID "FAT16   "

#define ENTRY_SIZE 32
#define ATTR_DIRECTORY 0x10
#define END_OF_DIRECTORY 0x00

bool is_fat16(int fd) {
    char type_fat16[FAT16_LENGTH + 1];  // + 1 -> '\0'

    if (lseek(fd, FAT16_OFFSET, SEEK_SET) == (off_t)-1) {
        perror("Error seeking in file");
        return false;
    }

    ssize_t bytes_read = read(fd, type_fat16, FAT16_LENGTH);
    if (bytes_read < FAT16_LENGTH) {
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

// Función para imprimir un directorio y su contenido recursivamente
void print_directory(int fd, const BootSector *bootSector, int level, void (*processEntry)(DirEntry*, int)) {
    unsigned int address = root_dir_address(bootSector);
    lseek(fd, address, SEEK_SET);  // Mover el fd al inicio del directorio 'root'

    DirEntry dir;
    for (int i = 0; i < bootSector->root_dir_entries; i++) {
        ssize_t bytes_read = read(fd, &dir, sizeof(DirEntry));
        if (bytes_read < (ssize_t)sizeof(DirEntry)) {
            break; // Error de lectura o final del directorio
        }

        if (dir.filename[0] == 0x00) {
            break; // Final de las entradas del directorio
        }

        if (dir.filename[0] == 0xE5) {
            continue; // Entrada de directorio borrada
        }

        processEntry(&dir, level);
    }
}

void process_dir_entry(DirEntry *entry, int level) {  
    if (strlen((const char *)entry->filename) <= 2) {
        return;
    }

    // Convertir el nombre y la extensión del archivo a una cadena completa
    char fullname[13];
    snprintf(fullname, sizeof(fullname), "%s.%s", entry->filename, entry->ext);

    // Eliminar espacios de relleno del nombre del archivo y la extensión
    char *end = fullname + strlen(fullname);
    while (end > fullname && end[-1] == ' ') --end;
    *end = '\0';

    // Imprimir la entrada con la indentación basada en el nivel de profundidad
    printf("%*s%s\n", level * 4, "", entry->filename);  // Se usan 4 espacios por nivel de profundidad
}
