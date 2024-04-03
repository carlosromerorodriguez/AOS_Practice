#include "tree.h"

void printFileInfo(char filename[]) {
    FILE *fileSystem = fopen(filename, "rb");
    if (!fileSystem) {
        perror("Error opening file");
        return;
    }
    /*if (isExt2(filename)) {
        printExt2FileTree(filename);
    } else*/ if (isFat16(fileSystem)) {
        BootSector bootSector;
        readBootSector(filename, &bootSector);
        printBootSector(&bootSector);
    } else {
        printf("Unknown file system\n");
    }
}

void printFileTree(char filename[]) {
    FILE *fileSystem = fopen(filename, "rb");
    if (!fileSystem) {
        perror("Error opening file");
        return;
    }

    BootSector bootSector;
    readBootSector(filename, &bootSector);
    printBootSector(&bootSector);

    if (isFat16(fileSystem)) {
        readRootDir(fileSystem, &bootSector, processDirEntry);
    } else {
        printf("Unknown file system\n");
    }

    fclose(fileSystem);
}