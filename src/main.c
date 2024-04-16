#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include "common/tree.h"
#include "common/info.h"
#include "common/cat.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Wrong number of arguments.\n");
        return 1;
    }

    // Open the file system image file
    int fd = open(argv[2], O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "--info") == 0) {
        //TODO: Implement info command
        info_command(fd);
    } else if (strcmp(argv[1], "--tree") == 0) {
        //TODO: Implement tree command
        print_file_tree(fd);

    } else if (strcmp(argv[1], "--cat") == 0) {
        //TODO: Implement cat command
        char* fileName = argv[3];
        fileName = strcat(fileName, "\0");
        cat_command(fd, fileName);

    } else {
        printf("Invalid command.\n");
        close(fd);
        return 1;

    }

    close(fd);
    return EXIT_SUCCESS;
}