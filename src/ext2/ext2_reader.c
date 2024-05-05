#include "ext2_reader.h"

/**
 * @brief Checks if the file system is an EXT2 file system.
 * 
 * @param fd File descriptor of the file system.
 * 
 * @return 1 if the file system is EXT2, 0 otherwise.
*/
int is_ext2(int fd) {
    uint16_t magic;

    // Busquem el magic number de l'ext2
    if (lseek(fd, EXT2_SUPERBLOCK_OFFSET + EXT2_MAGIC_OFFSET, SEEK_SET) == -1) {
        perror("Error seeking to superblock");
        close(fd);
        exit(1);
    }

    // Llegim el magic number
    if (read(fd, &magic, sizeof(magic)) == -1) {
        perror("Error reading magic number");
        close(fd);
        exit(1);
    }

    // Mirem si el magic number és el de l'ext2
    if (magic == EXT2_MAGIC) {
        return 1;  // It's an EXT2 file system
    }
    
    return 0;  // It's not an EXT2 file system
}

/*
 * @brief Reads the superblock of an EXT2 file system.
 * @param fd File descriptor of the EXT2 file system.
 * @param superblock Pointer to the superblock structure to fill.
 */
int read_ext2_superblock(int fd, Ext2Superblock *superblock) {
    /*
     * Tenim definida la posició del superblock a EXT2_SUPERBLOCK_OFFSET que té el valor 1024 ja que la mida del bloc és de 1024 bytes
     * Fem servir el SEEK_SET per indicar que volem moure el cursor al principi del fitxer i lsseek per moure el cursor
     */
    if (lseek(fd, EXT2_SUPERBLOCK_OFFSET, SEEK_SET) != EXT2_SUPERBLOCK_OFFSET) { // Ens movem al superblock
        return -1; // Si no podem moure'ns al superblock retornem -1
    }

    // Llegim un superblock mitjançant el struct Ext2Superblock
    if (read(fd, superblock, sizeof(Ext2Superblock)) != sizeof(Ext2Superblock)) { // Llegim el superblock
        return -1; // Si no podem llegir el superblock retornem -1
    }
    return 0;
}

/*
    * @brief Reads a group descriptor from the block group descriptor table.
    * @param fd File descriptor of the EXT2 file system.
    * @param superblock Superblock of the EXT2 file system.
    * @param group_num Number of the group descriptor to read.
    * @param group_desc Pointer to the group descriptor structure to fill.
 */
int read_ext2_group_desc(int fd, Ext2Superblock *superblock, uint32_t group_num, Ext2GroupDesc *group_desc) {
    /*
     * Calculem el block_size de la seguent manera: 1024 << log_block_size.
     * Es fa aixi perque en ext2 la mida del bloc donada pel superblock es en potencies de 2
     */
    uint32_t block_size = 1024 << superblock->log_block_size;

    // Calculem el bloc on comença la taula de descriptors de grup, sumem 1 perque el superblock ocupa el primer bloc 
    uint32_t bgdt_block = superblock->first_data_block + 1;

    /*
     * Ens movem al descriptor de grup que volem llegir, el descriptor de grup es troba a la posició
     * fem servir la formula: (bgdt_block * block_size) + (group_num * sizeof(Ext2GroupDesc))
     * per calcular la posició del descriptor de grup que volem llegir
     * Ho fem amb aquesta formula ja que el descriptor del grup es troba a la posició 
     */
    lseek(fd, (bgdt_block * block_size) + (group_num * sizeof(Ext2GroupDesc)), SEEK_SET);

    // Llegim el descriptor de grup
    read(fd, group_desc, sizeof(Ext2GroupDesc));

    return 0;
}


/*
    * @brief Reads an inode from the inode table.
    * @param fd File descriptor of the EXT2 file system.
    * @param superblock Superblock of the EXT2 file system.
    * @param inode_num Number of the inode to read.
    * @param inode Pointer to the inode structure to fill.
 */
int read_ext2_inode(int fd, Ext2Superblock *superblock, uint32_t inode_num, Ext2Inode *inode) {
    // Calculem el número de grup per l'inode donat. Cada grup de blocs conté un nombre fix d'inodes com definit en el superblock
    uint32_t group_num = (inode_num - 1) / superblock->inodes_per_group;

    Ext2GroupDesc group_desc;
    // Llegim el descriptor del grup per obtenir les ubicacions de les taules d'inodes entre d'altres
    read_ext2_group_desc(fd, superblock, group_num, &group_desc); 

    // La ubicació de la taula d'inodes dins del grup, ens dona el bloc inicial on comencen els inodes del grup
    uint32_t inode_table_start = group_desc.inode_table;

    // Calculem l'índex local de l'inode dins del seu grup de blocs, utilitzant mòdul amb el nombre d'inodes per grup
    uint32_t index = (inode_num - 1) % superblock->inodes_per_group;

    // Calculem la mida del bloc utilitzant el valor de desplaçament bit a bit com definit en el superblock ja que la mida del bloc es en potencies de 2    
    uint32_t block_size = 1024 << superblock->log_block_size; 

    // Utilitzem la mida de l'inode com a especificat en el superblock per garantir que el càlcul de la posició sigui correcte
    uint32_t inode_size = superblock->inode_size;

    // Calculem el bloc dins del grup que conté l'inode específic, basant-nos en la mida de l'inode
    uint32_t containing_block = (index * inode_size) / block_size;

    // Calculem l'offset dins del bloc on comença l'inode específic
    uint32_t offset_within_block = (index * inode_size) % block_size;

    // Combinem l'ubicació del bloc inicial, l'offset del bloc contenidor i l'offset dins del bloc per obtenir l'offset complet en el fitxer
    uint32_t read_offset = (inode_table_start + containing_block) * block_size + offset_within_block;

    // Posicionem el descriptor del fitxer al començament de l'inode que volem llegir
    if (lseek(fd, read_offset, SEEK_SET) != read_offset) {
        perror("Error seeking to inode");
        return -1;
    }

    // Creem un buffer per llegir l'inode i llegim la quantitat de bytes que ocupa un inode.
    char inode_buffer[256]; // Utilitzem 256 bytes ja que el superblock indica que la mida de l'inode es de 256 bytes
    if (read(fd, inode_buffer, sizeof(inode_buffer)) != sizeof(inode_buffer)) {
        perror("Error reading inode");
        return -1;
    }

    // Copiem les dades llegides al buffer de l'estructura d'inode.
    memcpy(inode, inode_buffer, sizeof(Ext2Inode));

    return 0;
}

/*
    * @brief Reads a directory from the inode.
    * @param fd File descriptor of the EXT2 file system.
    * @param superblock Superblock of the EXT2 file system.
    * @param inode Inode of the directory to read.
    * @param entries Pointer to the directory entries structure to fill.
 */
int read_ext2_directory(int fd, Ext2Superblock *superblock, Ext2Inode *inode, Ext2DirectoryEntry *entries) {
    // Calculem la mida del bloc utilitzant el desplaçament bit a bit. 
    // Ext2 fa servir una base de 1024 bytes, i 'log_block_size' indica quantes vegades aquesta base ha de ser desplaçada a l'esquerra (multiplicada per 2 a la potència de 'log_block_size').
    uint32_t block_size = 1024 << superblock->log_block_size;

    // Calculem el nombre total de blocs necessaris per emmagatzemar les dades de l'inode
    // Això s'aconsegueix sumant 'block_size - 1' a la mida total de l'inode abans de dividir per la mida del bloc.
    uint32_t num_blocks = (inode->size + block_size - 1) / block_size;

    uint32_t block_num = 0;
    uint32_t block_offset = 0;

    // Per cada bloc de dades de l'inode
    for (uint32_t i = 0; i < num_blocks; i++) {
        // Obtenim el número del bloc de dades des de l'array de blocs de l'inode
        // 'block_offset' serveix com un índex a l'array 'block' dins de l'estructura de l'inode, que conté les adreces dels blocs físics on estan emmagatzemades les dades
        block_num = inode->block[block_offset];
        block_offset++;

        // Ens posicionem al començament del bloc de dades corresponent en el fitxer o dispositiu
        // Multipliquem 'block_num' per 'block_size' ja que ens dona la posició en bytes des del començament del fitxer
        // Fem servir 'lseek'per moure el cursor del fitxer a aquesta posició.
        if (lseek(fd, block_num * block_size, SEEK_SET) != block_num * block_size) {
            perror("Error seeking to block");
            return -1;
        }

        // Llegim un bloc sencer de dades del fitxer, fem serivr entrada + i * block_size per llegir el bloc sencer ja que la mida del bloc es de block_size i li sumem i * block_size per llegir el següent bloc
        if (read(fd, ((char*)entries) + i * block_size, block_size) != block_size) {
            perror("Error reading block");
            return -1;
        }
    }

    return 0;
}


#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"

/*
    * @brief Prints a line of the tree representation of the directory structure.
    * @param level Level of the tree where the line will be printed.
    * @param name Name of the entry to print.
    * @param is_last_entry Flag indicating if the entry is the last one in the directory.
    * @param is_directory Flag indicating if the entry is a directory.
 */
void print_tree_line(int level, const char* name, int is_last_entry, int is_directory) {
    static int levels[100] = {0}; // Array per rastrejar els nivells actius
    
    for (int i = 0; i < level; ++i) {
        printf("%s", levels[i] ? "│   " : "    ");
    }

    if (is_directory) {
        printf("%s%s── %s%s\n", is_last_entry ? "└" : "├", ANSI_COLOR_BLUE, name, ANSI_COLOR_RESET);
    } else {
        printf("%s%s── %s%s\n", is_last_entry ? "└" : "├", ANSI_COLOR_WHITE, name, ANSI_COLOR_RESET);
    }

    levels[level] = !is_last_entry; // Establim el nivell actual com a actiu o no
    
}

/*
    * @brief Performs a depth-first search of the EXT2 file system.
    * @param fd File descriptor of the EXT2 file system.
    * @param inode_num Number of the inode to start the search from.
    * @param superblock Superblock of the EXT2 file system.
    * @param level Level of the tree where the search is currently at.
 */
void dfs_ext2(int fd, uint32_t inode_num, Ext2Superblock *superblock, int level, uint32_t current_inode, uint32_t parent_inode) {
    Ext2Inode inode; // Ínode actual en el que estem
    Ext2DirectoryEntry *entries; // Entrades del directori
    // Mida del bloc, shiftem 1024 a l'esquerra per obtenir la mida del bloc ja que el superblock ens dona la mida del bloc en potencies de 2
    uint32_t block_size = 1024 << superblock->log_block_size; 

    // LLegim l'ínode
    read_ext2_inode(fd, superblock, inode_num, &inode);
    uint32_t num_blocks = (inode.size + block_size - 1) / block_size;
    
    // Comprovem si l'ínode és un directori
    if (inode.mode & 0x4000) { 
        // Llegim les entrades del directori
        entries = (Ext2DirectoryEntry *) malloc(num_blocks * block_size);
        read_ext2_directory(fd, superblock, &inode, entries); // Llegim les entrades del directori

        // Per cada entrada del directori
        for (uint32_t offset = 0; offset < block_size; ) {
            Ext2DirectoryEntry *entry = (Ext2DirectoryEntry *)((char *)entries + offset);
            if (entry->inode != 0) { // Si l'entrada no és buida
                // Mostrem el nom de l'entrada

                uint32_t next_offset = offset + entry->rec_len;
                uint32_t is_current_last_entry = (next_offset >= block_size) || (((Ext2DirectoryEntry *)((char *)entries + next_offset))->inode == 0);

                if (entry->inode != current_inode && entry->inode != parent_inode && strcmp(entry->name, "lost+found") != 0) {
                    print_tree_line(level, entry->name, is_current_last_entry, entry->file_type == 2);
                }

                // Explorem recursivament si és un directori i no és '.' ni '..'
                if (entry->file_type == 2 && entry->inode != current_inode && entry->inode != parent_inode) {
                    dfs_ext2(fd, entry->inode, superblock, level + 1, entry->inode, current_inode);
                }
            }
            offset += entry->rec_len; // Ens movem a la següent entrada
        }

        free(entries); // Alliberem la memòria de les entrades
    }
}

/*
    * @brief Displays the contents of a file.
    * @param fd File descriptor of the EXT2 file system.
    * @param inode Inode of the file to display.
    * @param block_size Size of the blocks in the file system.
 */
void cat_ext2_file(int fd, Ext2Inode *inode, uint32_t block_size) {
    // Calculem el nombre total de blocs de dades necessaris per emmagatzemar el fitxer,
    // arrodonint cap amunt si la mida total del fitxer no és múltiple de block_size
    uint32_t num_blocks = (inode->size + block_size - 1) / block_size;

    // Per cada bloc de dades del fitxers
    for (uint32_t i = 0; i < num_blocks; i++) {
        // Si el bloc de dades està buit saltem
        if (inode->block[i] == 0) continue;

        // Posiciona el descriptor del fitxer al començament del bloc de dades actual, ho fem multiplican l'index del bloc per la mida del bloc
        if (lseek(fd, inode->block[i] * block_size, SEEK_SET) != inode->block[i] * block_size) {
            perror("Error seeking to block");
            return;  // Si lseek falla mostrem un missatge d'error i retornem
        }

        // Creem un buffer per llegir el bloc de dades
        char buffer[block_size];
        // Llegim el bloc de dades sencer
        ssize_t bytes_read = read(fd, buffer, block_size);
        if (bytes_read == -1) {
            perror("Error reading block");
            return;  // Si la lectura falla, mostrem un missatge d'error i retornem
        }

        // Escrivim els bytes llegits
        printf("%.*s", (int)bytes_read, buffer);
    }
}


/*
    * @brief Displays the contents of a file.
    * @param fd File descriptor of the EXT2 file system.
    * @param inode_num Number of the inode to display.
    * @param superblock Superblock of the EXT2 file system.
    * @param filename Name of the file to display.
 */
void cat_ext2(int fd, uint32_t inode_num, Ext2Superblock *superblock, char* filename, uint32_t current_inode, uint32_t parent_inode) {
    Ext2Inode inode; // Ínode actual en el que estem
    Ext2DirectoryEntry *entries; // Entrades del directori
    // Mida del bloc, shiftem 1024 a l'esquerra per obtenir la mida del bloc ja que el superblock ens dona la mida del bloc en potencies de 2
    uint32_t block_size = 1024 << superblock->log_block_size; 

    // LLegim l'ínode
    read_ext2_inode(fd, superblock, inode_num, &inode);
    uint32_t num_blocks = (inode.size + block_size - 1) / block_size;

    // Comprovem si l'ínode és un directori
    if (inode.mode & 0x4000) { 
        // Llegim les entrades del directori
        entries = (Ext2DirectoryEntry *) malloc(num_blocks * block_size); // Reservem memòria per les entrades
        read_ext2_directory(fd, superblock, &inode, entries); // Llegim les entrades del directori

        // Per cada entrada del directori
        for (uint32_t offset = 0; offset < block_size; ) {
            Ext2DirectoryEntry *entry = (Ext2DirectoryEntry *)((char *)entries + offset);
            if (entry->inode != 0) { // Si l'entrada no és buida
                char entry_name[entry->name_len + 1];
                memcpy(entry_name, entry->name, entry->name_len);
                entry_name[entry->name_len] = '\0';

                if(strcmp(entry_name, filename) == 0){
                    Ext2Inode file_inode; // Inode del fitxer
                    read_ext2_inode(fd, superblock, entry->inode, &file_inode); // Llegim l'ínode del fitxer
                    cat_ext2_file(fd, &file_inode, block_size); // Mostrem el contingut del fitxer
                }

                // Explorem recursivament si és un directori i no és el directori actual ni el directori pare
                if (entry->file_type == 2 && entry->inode != current_inode && entry->inode != parent_inode) {
                    cat_ext2(fd, entry->inode, superblock, filename, entry->inode, current_inode);
                }
            }
            offset += entry->rec_len; // Ens movem a la següent entrada
        }

        free(entries); // Alliberem la memòria de les entrades
    }
}