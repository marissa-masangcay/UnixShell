/*File: usfls.c
Purpose: This program mimics the ls command in the terminal.

Compile: gcc -o usfls usfls.c
Run: ./usfls <optional -a> 
----------------------------------------------------------*/

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*--------------------------------------------------------*/
int main(int argc, char **argv)
{
    DIR *dirp;
    struct dirent *dp;
    bool list_all = false;

    if (argc == 2 && strcmp(argv[1], "-a") == 0) {
        list_all = true;
    }

    dirp = opendir(".");
    if (dirp == NULL) {
        printf("Cannot opendir()\n");
        exit(-1);
    }

    while ((dp = readdir(dirp)) != NULL) {
        //checks if flag -a is present
        if (list_all) {
            printf("%s\n", dp->d_name);
        }
        else {
            if (dp->d_name[0] != '.') {
                printf("%s\n", dp->d_name);
            }
        }
    }
    closedir(dirp);

    return 0;
}