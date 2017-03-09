/*File: usfgrep.c
Purpose: This program implements the unix command to search files 
for the occurrence of a string that is input by the user.

Compile: gcc -o usfgrep usfgrep.c
Run: ./usfgrep <string> <file1> [<file2>...]
--------------------------------------------------------
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

char *filename;
char *query;
int line  = 0;

/*--------------------------------------------------------
Function:     querySearch
Purpose:      Searches through the file line for the input 
              string given by the user
*/
int querySearch(char input[], char query[], int charCount) {
     int i = 0; 
     int j = 0;
     int firstLetter;
     int found = -1;
 
     while(i < charCount)
     {
        while(input[i] != query[0] && i < charCount)
        {
            i++;
        }

        //keeps track of i in case first letter is spotted in query
        firstLetter = i;

        while (input[i] == query[j] && i < charCount && query[j] != '\0') 
        {
           i++;
           j++;
        }

        if (query[j] == '\0')
        {
           found = 1;
           return found;
        }

        i = firstLetter + 1;
        j = 0;
     }

    return found;
}

/*--------------------------------------------------------
Function:     printLine
Purpose:      Prints the file name, line number, and line
              contents for each line that contains the 
              string given by the user
*/
void printLine(int line, int charCount, char fileLine[]) {
    printf("%s[%d]: ", filename, line);
    int z;

    for(z = 0; z < charCount; z++)
    {
        printf("%c", fileLine[z]);
    }

    printf("\n");

    return;
}

/*--------------------------------------------------------
Function:     readLine
Purpose:      reads the file line by line to search for the 
              input string given by the user and prints out 
              the file name, line number, and line content if
              the string is found within that line
*/
int readLine(int fd) {
    int endOfFile = 0;
    char c;
    int n;
    int i = 0;
    char fileLine[511];
    int charCount = 0;
    int searchResult;

    //reads in the file line by line
    while((n = read(fd, &c, 1)) > 0)
    {
        if (c == '\n')
        {
            line++;
            break;
        } 
        else
        {
            fileLine[i] = c;
            charCount++;
            if(charCount > 511){
                printf("This file has exceeded the max number of chars in one line. Good bye.\n");
                exit(1);
            }
        }
        i++;
    }

    //checks if last line with remaining characters
    if(charCount > 0 && n == 0)
    {
        endOfFile = -1;
        line++;
    }

    //if end of file with blank line or empty text file
    if(charCount == 0 && n == 0)
    {
        endOfFile = -1;
    }

    //prints the line info if there is a match
    if((searchResult = querySearch(fileLine, query, charCount)) == 1)
    {
        printLine(line, charCount, fileLine);
    }

    return endOfFile;
}

/*--------------------------------------------------------
Function:     readFile
Purpose:      Reads through the file until the end of the 
              file is reached
*/
void readFile(int fd) {
    int endOfFile;

    while((endOfFile = readLine(fd)) >= 0);

    return;
}

/*--------------------------------------------------------*/
int main(int argc, char *argv[]) {
    int fd;
    char buffer[1];

    if (argc <= 2)
    {
        printf("Insufficient arguments\nUsage: usfgrep <string> <file1> [<file2>...]\n");
        exit(-1);
    }

    query = argv[1];

    int v;
    for(v = 2; v < argc; v++)
    {
        filename = argv[v];
        line = 0;

        fd = open(filename, O_RDONLY);

         //If fd has negative number there is an error
        if (fd < 0)
        {
            printf("Cannot open %s\n", filename);
        }
        else{
            readFile(fd);
        }

    }
    
    close(fd);

    return 0;
}
