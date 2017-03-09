/*File: usfsh.c
Purpose: This program mimics the terminal shell.

Compile: gcc -o usfsh usfsh.c
Run: ./usfsh
--------------------------------------------------------
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MAX_CHAR 255


/*--------------------------------------------------------
Function:     reset_commandline
Purpose:      Clears out all chars from the array
*/
void reset_commandLine (char *commandline)
{
  int g;
  for (g = 0; g <= MAX_CHAR; g++) {
    commandline[g] = ' ';
  }     

  return;
}


/*--------------------------------------------------------
Function:     is_cd
Purpose:      Checks to see if the cmd is a cd cmd
*/
bool is_cd (char **args)
{
  bool rv = false;

  if (strcmp(args[0], "cd") == 0) {
    rv = true;
  }

  return rv;
}


/*--------------------------------------------------------
Function:     is_exit
Purpose:      Checks to see if the user typed exit to exit
              the program
*/
bool is_exit (char **args)
{
  bool rv = false;

  if (strcmp(args[0], "exit") == 0) {
    rv = true;
  }

  return rv;
}


/*--------------------------------------------------------
Function:     is_pipe_redirection
Purpose:      Checks to see if the cmd is a piping cmd
*/
bool is_pipe_redirection(char **args, int *pos_found)
{
  int i;
  bool rv = false;

  for (i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "|") == 0) {
      *pos_found = i;
      rv = true;
    }
  }

  return rv;
}


/*--------------------------------------------------------
Function:     is_file_redirection
Purpose:      Checks to see if the cmd is a file redirection
*/
bool is_file_redirection(char **args, int *pos_found)
{
  int i;
  bool rv = false;

  for (i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], ">") == 0) {
      *pos_found = i;
      rv = true;
    }
  }

  return rv;
}


/*--------------------------------------------------------
Function:     execute_cd
Purpose:      Executes the cd cmd input by the user
*/
int execute_cd(char **args, int num_args)
{
  int i;
  int rv;

  if (num_args == 1) {
    rv = chdir(getenv("HOME"));
  }
  else {
    rv = chdir(args[1]);
  }

  if (rv != 0) {
    printf("ERROR changing directory\n");
    exit(-1);
  }

  return rv;
}


/*--------------------------------------------------------
Function:     execute_redirect
Purpose:      Executes the file redirection input by the user
*/
void execute_redirect(char **args, int redirect_pos) {
  pid_t id;
  int fd;
  char* file_args[redirect_pos];
  int j;

  int b = 0;
  for (j = 0; j < redirect_pos+1; j++) {
    file_args[b] = args[j];
    b++;
  }
  file_args[redirect_pos] = NULL;

  if ((fd = open(args[redirect_pos+1], O_WRONLY|O_TRUNC|O_CREAT, 0644)) < 0) {
    printf("cannot open out\n");
    exit(-1);
  }

  id = fork();

  if (id == 0) {
    close(1);
    dup(fd);  
    close(fd);
    execvp(args[0], file_args);
    printf("ERROR in file redirection child\n");
    exit(-1);
  }

  close(fd);
  id = wait(NULL);

  return;
}


/*--------------------------------------------------------
Function:     execute_piping
Purpose:      Executes the piping cmd input by the user
*/
void execute_piping(char **args, int piping_pos) {

  pid_t id;
  int count;
  int total = 0;
  char buf[100];
  int fildes[2];
  int i;
  int j;
  char* pipe_args[piping_pos];

  int b = 0;
  for (j = 0; j < piping_pos+1; j++) {
    pipe_args[b] = args[j];
    b++;
  }
  pipe_args[piping_pos] = NULL;
  
  pipe(fildes);

  id = fork();

    /* Child 1 */
  if (id == 0) {
    close(1);
    dup(fildes[1]);
    close(fildes[0]);
    close(fildes[1]);
    execvp(args[0], pipe_args);
    printf("ERROR in piping child 1\n");
    exit(0);
  }

  id = fork();

    /* Child 2 */
  if (id == 0) {
    close(0);
    dup(fildes[0]);
    close(fildes[1]);
    close(fildes[0]);
    execvp(args[piping_pos+1], &args[piping_pos+1]);
    printf("ERROR in piping child 2\n");
    exit(0);
  }

  close(fildes[0]);
  close(fildes[1]);

  id = wait(NULL);
  id = wait(NULL);

  return;
}


/*--------------------------------------------------------
Function:     execute_cmd
Purpose:      Excutes the cmd input by the user
*/
void  execute_cmd(char **args) {
 pid_t  id;
 char *cmd = args[0];

 if ((id = fork()) < 0) {
  printf("ERROR: forking child process failed\n");
  exit(-1);
}
else if (id == 0) {
 if (execvp(cmd, args) < 0) {  
  printf("%s: command not found\n", cmd);
  exit(-1);
}
}
else {
 id = wait(NULL);
}

return;
}


/*--------------------------------------------------------
Function:     parse_args
Purpose:      parses out the command line to form an args 
              array
*/
void parse_args(char *args[], char *commandline) {
  char *token;
  int j = 0;
  const char delim[2] = " ";

  token = strtok(commandline, delim);

  if (token != NULL) {
   args[j] = token;
   j++;
 }

 while (token != NULL) {
   token = strtok(NULL, delim);
   args[j] = token;
   j++;
 }

 return;
}


/*--------------------------------------------------------
Function:     read_user_input
Purpose:      Reads in the input from the user and places 
              it into the commandline array
*/
int read_user_input(char *commandline) {
  int i = 0;
  char c;
  int n;
  int num_words = 0;

  write(1, "$ ", 2);

  while ((n = read(0, &c, 1)) > 0) {

    if (i > MAX_CHAR) {
      printf("ERROR: max characters reached\n");
      exit(-1);
    }
    if (c == ' ') {
      num_words++;
    }

    if (c == '\n') {
      commandline[i] = '\0';
      break;
    } 
    commandline[i] = c;
    i++;
  }

  if (i != 0) {
    num_words++;
  }

  return num_words;
}

/*--------------------------------------------------------*/
int main(int argc, char **argv) {
  int pos_found = -1;
  int num_args;
  char commandline[MAX_CHAR];

  while (1) {

   num_args = read_user_input(commandline);

   if (num_args != 0) {

    //instantiate args array with number of args input
    char* args[num_args];

    parse_args(args, commandline);

    if (is_exit(args)) {
      exit(0);
    }
    else if (is_cd(args)) {
      execute_cd(args, num_args);
    }  
    else if (is_pipe_redirection(args, &pos_found)) {
      execute_piping(args, pos_found);
    }
    else if (is_file_redirection(args, &pos_found)) {
      execute_redirect(args, pos_found);
    }
    else {
      execute_cmd(args);
    }

    reset_commandLine(commandline);   

  }

}

return 0;
}