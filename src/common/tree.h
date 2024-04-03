#ifndef _TREE_H
#define _TREE_H

#include "../ext2/ext2_reader.h"
#include "../fat16/fat16_reader.h"

void printFileInfo(char filename[]);
void printFileTree(char filename[]);
void readRootDir(FILE *fd, const BootSector *bootSector, void (*processEntry)(const DirEntry*, int));
void processDirEntry(const DirEntry *entry, int level);

#endif // !_TREE_H