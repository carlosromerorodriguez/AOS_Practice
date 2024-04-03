#ifndef _TREE_H
#define _TREE_H

#include "../ext2/ext2_reader.h"
#include "../fat16/fat16_reader.h"

void print_file_tree(int fd);
void read_root_dir(int fd, const BootSector *bootSector, void (*processEntry)(const DirEntry*, int));
void process_dir_entry(const DirEntry *entry, int level);

#endif // !_TREE_H