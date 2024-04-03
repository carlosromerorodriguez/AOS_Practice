#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include "common/tree.h"
/* includes */

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Wrong number of arguments.\n");
        return 1;
    }

    // Open the file system image file
    int fd = open(argv[2], O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    if (strcmp(argv[1], "--info") == 0) {
        //TODO: Implement info command
        printFileInfo(argv[2]);
    } else if (strcmp(argv[1], "--tree") == 0) {
        //TODO: Implement tree command
        printFileTree(argv[2]);

    } else if (strcmp(argv[1], "--cat") == 0) {
        //TODO: Implement cat command
        
    } else {
        printf("Invalid command.\n");
        close(fd);
        return 1;

    }

    close(fd);
    return 0;
}