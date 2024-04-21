#ifndef _CAT_H
#define _CAT_H

/**
 * @brief Displays the contents of a file using the cat command.
 * 
 * @param fd File descriptor of the file system.
 * @param fileName Name of the file to display.
 * 
 * @return void
*/
void cat_command(int fd, char* fileName);

#endif // !_CAT_H