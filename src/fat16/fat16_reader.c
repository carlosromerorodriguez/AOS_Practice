#include "fat16_reader.h"

int fat16_recursion_tree_helper(int fd, BootSector bs, int current_sector, int depth, int wasLast, int tree_not_cat, char *file_name);
void print_directory_tree_entry(unsigned char entry_filename[], int depth, int is_last_entry, int prev_last_entry, int is_directory);
uint32_t calculate_root_dir_sectors(BootSector bpb);
void get_filename_processed(unsigned char entry_filename[], char filename[], int is_directory);
void print_directory_cat_entry(int fd, DirEntry entry, BootSector bs);

/**
 * Checks if the file system is FAT16 by reading the boot sector.
 * 
 * @param fd File descriptor of the file system.
 * 
 * @return 1 if the file system is FAT16, 0 otherwise.
*/
int is_fat16(int fd) {
    BootSector bpb;
    read_boot_sector(fd, &bpb);

    // Determine the count of sectors in the data region of the volume
    uint32_t fat_size = bpb.fat_size_16 != 0 ? bpb.fat_size_16 : bpb.total_sectors_32;
    uint32_t total_sectors = bpb.total_sectors_16 != 0 ? bpb.total_sectors_16 : bpb.total_sectors_32;
    uint32_t root_dir_sectors = calculate_root_dir_sectors(bpb);
    uint32_t data_sectors = total_sectors - (bpb.reserved_sectors + (bpb.number_of_fats * fat_size) + root_dir_sectors);
    uint32_t count_of_clusters = data_sectors / bpb.sectors_per_cluster;

    return count_of_clusters >= 4085 && count_of_clusters < 65525;    
}

/**
 * Reads the boot sector of the file system. 
 * 
 * @param fd File descriptor of the file system.
 * @param bootSector Pointer to the boot sector structure to store the boot sector information.
 * 
 * @return void
*/
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

/**
 * Prints the boot sector information.
 * 
 * @param bootSector Pointer to the boot sector structure.
 * 
 * @return void
*/
void print_boot_sector(const BootSector *bootSector) {
    printf("Filesystem: FAT16\n\n");
    printf("System name: %.8s\n", bootSector->oem);
    printf("Sector size: %u bytes\n", bootSector->sector_size);
    printf("Sectors per cluster: %u\n", bootSector->sectors_per_cluster);
    printf("Reserved Sectors: %u\n", bootSector->reserved_sectors);
    printf("# of FATs: %u\n", bootSector->number_of_fats);
    printf("Max root entries: %u\n", bootSector->root_dir_entries);
    printf("Sectors per FAT: %u\n", bootSector->fat_size_16);
    printf("Label: %.11s\n", bootSector->volume_label);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                       //
// FASE 2 FUNCTIONS                                                                                                      //
//                                                                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t calculate_root_dir_sectors(BootSector bpb) 
{
    return ((bpb.root_dir_entries * 32) + (bpb.sector_size - 1)) / bpb.sector_size;
}

uint32_t calculate_first_root_dir_sector_number(BootSector bpb) 
{
    return bpb.reserved_sectors + (bpb.number_of_fats * bpb.fat_size_16);
}

uint32_t calculate_first_data_sector(BootSector bpb, uint32_t root_dir_sectors) 
{
    return calculate_first_root_dir_sector_number(bpb) + root_dir_sectors;
}

uint32_t calculate_first_sector_of_cluster(uint16_t cluster, BootSector bs) 
{
    uint32_t first_sector_of_cluster = (cluster - 2) * bs.sectors_per_cluster + calculate_first_data_sector(bs, calculate_root_dir_sectors(bs));
    return first_sector_of_cluster;
}

off_t calculate_dir_entry_offset(uint32_t current_sector, uint16_t idx, const BootSector bs) 
{
    return current_sector * bs.sector_size + idx * sizeof(DirEntry);
}

int is_last_active_entry(int fd, uint32_t current_sector, uint16_t idx, BootSector bs) 
{
    off_t start_offset = calculate_dir_entry_offset(current_sector, idx + 1, bs);
    DirEntry next_entry;

    for (uint16_t i = idx + 1; i < bs.sector_size / sizeof(DirEntry); i++, start_offset += sizeof(DirEntry)) {
        if (lseek(fd, start_offset, SEEK_SET) == -1 || read(fd, &next_entry, sizeof(DirEntry)) != sizeof(DirEntry)) {
            perror("Error reading directory entry");
            exit(EXIT_FAILURE);
        }
        
        // Si se encuentra otra entrada válida, no es la última
        if (next_entry.filename[0] != DIR_ENTRY_FREE && next_entry.filename[0] != DIR_ENTRY_EMPTY) {
            return 0; 
        }
    }

    return 1; 
}

uint16_t read_fat_entry(int fd, BootSector bpb, uint16_t cluster) 
{
    off_t fat_offset = bpb.reserved_sectors * bpb.sector_size + cluster * 2;
    uint16_t next_cluster;

    if (lseek(fd, fat_offset, SEEK_SET) == -1 || read(fd, &next_cluster, sizeof(uint16_t)) != sizeof(uint16_t)) {
        perror("Error reading FAT entry");
        exit(EXIT_FAILURE);
    }

    return next_cluster;
}

void fat16_recursion_tree(int fd, const BootSector bpb, int tree_not_cat, char *filename_to_find) 
{
    int first_root_dir_sector_number = calculate_first_root_dir_sector_number(bpb);
    uint32_t root_dir_sectors = calculate_root_dir_sectors(bpb);
    
    for (uint32_t i = 0; i < root_dir_sectors; i++) {
        if (fat16_recursion_tree_helper(fd, bpb, first_root_dir_sector_number + i, 0, 0, tree_not_cat, filename_to_find)) {
            return;
        }
    }
    printf("File not found.\n");
}

int fat16_recursion_tree_helper(int fd, BootSector bpb, int current_sector, int lvl, int prev_last_entry, int tree_not_cat, char *file_name) 
{
  for (size_t i = 0; i < (bpb.sector_size / sizeof(DirEntry)); i++) 
  {
    DirEntry entry;
    off_t offset = calculate_dir_entry_offset(current_sector, i, bpb);

    lseek(fd, offset, SEEK_SET);
    read(fd, &entry, sizeof(DirEntry));

    // skip "." + ".." + "deleted" / "empty" entries
    if (entry.filename[0] == CURRENT_DIR_ENTRY || entry.filename[0] == DIR_ENTRY_FREE || entry.filename[0] == DIR_ENTRY_EMPTY) {
        continue;
    }
    
    int is_last_entry = is_last_active_entry(fd, current_sector, i, bpb);
    if (entry.attributes == ATTR_DIRECTORY) 
    {
        if (tree_not_cat) {
            print_directory_tree_entry(entry.filename, lvl, is_last_entry, prev_last_entry, 1);
        }
        
        uint16_t current_cluster = entry.startCluster;
        while (current_cluster < 0xFFF8) { // 0xFFF8 is the end-of-cluster-chain marker for FAT16
            uint32_t first_sector_of_cluster = calculate_first_sector_of_cluster(current_cluster, bpb);
            fat16_recursion_tree_helper(fd, bpb, first_sector_of_cluster, lvl + 1, is_last_entry, tree_not_cat, file_name);
            current_cluster = read_fat_entry(fd, bpb, current_cluster);
        }
    } 
    else if (entry.attributes == ATTR_ARCHIVE) 
    {
        if (tree_not_cat) {
            print_directory_tree_entry(entry.filename, lvl, is_last_entry, prev_last_entry, 0);
        } else {
            char filename_processed[20];
            get_filename_processed(entry.filename, filename_processed, 0);

            if (!strcmp(file_name, (char *)filename_processed)) {
                print_directory_cat_entry(fd, entry, bpb);
                return 1;
            }
        }
    }
  }
  return 0;
}

void get_filename_processed(unsigned char entry_filename[], char filename[], int is_directory)
{
    int cont = 0;
    int processing_extension = 0;

    if (is_directory) filename[cont++] = '[';

    for (int i = 0; i < 11; i++) {
        if (i == 0 && entry_filename[i] == KANJI_SPECIAL_CASE) {
            filename[cont++] = (char)DIR_ENTRY_FREE;
            continue;
        }
        if (entry_filename[i] == SPACE_PAD) continue;
        if (entry_filename[i] == '~') break; 

        // Agregar punto antes de la extensión
        if (i == 8) {
            if (processing_extension) break;
            filename[cont++] = '.';
            processing_extension = 1;
        }
        filename[cont++] = tolower(entry_filename[i]);
    }
    if (is_directory) filename[cont++] = ']';
    filename[cont] = '\0';
}

void print_directory_tree_entry(unsigned char entry_filename[], int depth, int is_last_entry, int prev_last_entry, int is_directory)
{
    char filename[20]; // 8 + '.' + 3 + '\0'

    get_filename_processed(entry_filename, filename, is_directory);

    // Imprimir la indentación basada en la profundidad
    for (int i = 0; i < depth; i++) {
        printf(i == 0 && depth > 1 ? "│   " : (prev_last_entry ? "    " : "│   "));
    }
    
    if (is_directory) {
        printf(is_last_entry ? "└──" ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "\n" : "├──" ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "\n", filename);
    } else {
        printf(is_last_entry ? "└── %s\n" : "├── %s\n", filename);
    }
}

void print_directory_cat_entry(int fd, DirEntry entry, BootSector bpb)
{
    uint16_t current_cluster = entry.startCluster;
    int file_size = entry.fileSize;
    int bytes_read = 0;

    while (bytes_read < file_size && current_cluster < 0xFFF8) {
        uint32_t first_sector_of_cluster = calculate_first_sector_of_cluster(current_cluster, bpb);
        
        for (int i = 0; i < bpb.sectors_per_cluster && bytes_read < file_size; i++) {
            int bytes_to_read = bpb.sector_size;
            bytes_to_read = bytes_read + bytes_to_read > file_size ? file_size - bytes_read : bytes_to_read;

            char buffer[bytes_to_read];
            lseek(fd, (first_sector_of_cluster + i) * bpb.sector_size, SEEK_SET);
            read(fd, buffer, bytes_to_read);

            printf("%.*s", bytes_to_read, buffer);

            bytes_read += bytes_to_read;
        }

        current_cluster = read_fat_entry(fd, bpb, current_cluster);
    }
}