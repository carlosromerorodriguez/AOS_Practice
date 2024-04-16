#ifndef _TREE_H
#define _TREE_H

#include "../ext2/ext2_reader.h"
#include "../fat16/fat16_reader.h"

void print_file_tree(int fd);
void print_directory(int fd, const BootSector *bootSector, int level, void (*processEntry)(DirEntry*, int));
void process_dir_entry(DirEntry *entry, int level);

#endif // !_TREE_H