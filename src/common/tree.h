#ifndef _TREE_H
#define _TREE_H

#include "../fat16/fat16_reader.h"

/**
 * @brief Prints the tree representation of the directory structure of the file system.
 * 
 * @param fd File descriptor of the file system.
 * 
 * @return void
*/
void print_file_tree(int fd);
void fat16_recursion_tree(int fd, const BootSector bootSector, int tree_not_cat, char *filename_to_find);
void process_dir_entry(const DirEntry *entry, int level);

#endif // !_TREE_H