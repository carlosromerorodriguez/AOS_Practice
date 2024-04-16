#include "info.h"
#include "../ext2/ext2_reader.h"
#include "../fat16/fat16_reader.h"

void print_ext2_superblock(int fd);

void print_fat16_boot_sector(int fd);

void print_time(const char *prefix, time_t timestamp);

void info_command(int fd) {
    printf("---- Filesystem Information ----\n\n");

    //Check if the file system is ext2 or fat16
    if (is_ext2(fd)) {
        //Print the superblock information
        print_ext2_superblock(fd);
    } else if (is_fat16(fd)) {
        //Print the boot sector information
        print_fat16_boot_sector(fd);
    } else {
        printf("Invalid file system.\n");
    }
}

void print_ext2_superblock(int fd) {
    Ext2Superblock superblock;

    superblock.volume_name[0] = '\0'; // Asegurar que el nom del volum està finalitzat en NULL
    if (read_ext2_superblock(fd, &superblock) < 0) {
        perror("Error reading superblock");
        close(fd);
        return;
    }

    char volume_name[17]; // 16 caracteres + 1 para el terminador nulo
    volume_name[16] = '\0'; // Asegura que la cadena esté terminada en NULL

    if (lseek(fd, 1024 + 120, SEEK_SET) < 0) { // Moverse al offset donde comienza el nombre del volumen dentro del superbloque
        perror("Error seeking to volume name");
        close(fd);
        return;
    }

    if (read(fd, volume_name, 16) < 0) { // Leer los 16 bytes del nombre del volumen
        perror("Error reading volume name");
        close(fd);
        return;
    }

    /* Com que EXT2 permet diferents mides de blocs hem d'aplicar la formula: 1024 << log_block_size
     * La mida base del bloc és 1024 bytes, per tant, per calcular la mida del bloc
     * hem de fer 1024 << log_block_size, on log_block_size és el camp de la superblock
     * que ens indica la mida del bloc en potències de 2. */
    uint32_t block_size_bytes = 1024 << superblock.log_block_size;

    printf("Filesystem: EXT2\n\n");
    printf("INODE INFO\n");
    printf("Inode Size: %u bytes\n", superblock.inode_size);
    printf("Num Inodes: %u\n", superblock.total_inodes);
    printf("First Inode: %u\n", superblock.first_non_reserved_inode);
    printf("Inodes per Group: %u\n", superblock.inodes_per_group);
    printf("Free Inodes: %u\n\n", superblock.free_inodes);

    printf("BLOCK INFO\n");
    printf("Block Size: %u bytes\n", block_size_bytes);
    printf("Reserved Blocks: %u\n", superblock.reserved_blocks);
    printf("Free Blocks: %u\n", superblock.free_blocks);
    printf("Total Blocks: %u\n", superblock.total_blocks);
    printf("Blocks per Group: %u\n", superblock.blocks_per_group);
    printf("Frags per Group: %u\n\n", superblock.frags_per_group);

    printf("VOLUME INFO\n");
    printf("Volume Name: %s\n", volume_name); // Asumim que volume_name és una cadena de text finalitzada en NULL
    print_time("Last Checked:", superblock.last_check);
    print_time("Last Mounted:", superblock.last_mount_time);
    print_time("Last Written:", superblock.last_written_time);

    return; 
}

void print_time(const char *prefix, time_t timestamp) {
    struct tm *time_info;
    char time_buffer[80];

    time_info = localtime(&timestamp);
    strftime(time_buffer, sizeof(time_buffer), "%c", time_info);

    printf("%s: %s\n", prefix, time_buffer);
}
/*
 * Print the boot sector information of a fat16 file system
 */
void print_fat16_boot_sector(int fd) {
    BootSector bootSector;
    read_boot_sector(fd, &bootSector);
    print_boot_sector(&bootSector);
}

