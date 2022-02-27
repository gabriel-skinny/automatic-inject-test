#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "syscall.h"
#include <fcntl.h>
#include <unistd.h>
#include "main.h"

#define MAXFILENAME 100
#define MAXFILEBUFFER 1200


void readfilename(char * filename, char * varname, char * varfilecontent);
void strcatwithspace(char *dest, char *src);
void execCommand(char * command, char *dest);
void buildCommand(char *args, char *dest);
void superstrcat(char *dest, char *argv[], int n);

int main(int argc, char *argv[]) {
  char *filename = (char * )malloc(MAXFILENAME);
  char *filecontent = (char * )malloc(MAXFILEBUFFER);

  if (argc < 2) {
    fprintf(stderr, "At least one arguments has to be provided");
    return -1;
  }

  while(--argc > 0 && *++argv) {    
    if (*argv[0] == '-'){
      if (strncmp(*argv, "-file", sizeof *argv) == 0)
        readfilename(*++argv, filename, filecontent);  
    }   
  }

  printf("\n\nFILE CONTENT: %s\n\n", filecontent);

  return 0;
}

void readfilename(char * filename, char * globalfilename, char *filecontent) {  
  int fd;
  char command[100];
  char filepath[100];

  buildCommand(filename, command);
  execCommand(command, filepath);

  if((fd = open(filepath, 'r')) == -1) {
    fprintf(stderr, "File not found");
    return;
  }

  read(fd, filecontent, MAXFILEBUFFER);
}



void buildCommand(char *args, char *dest) {
  char baseCommand[150] = "/bin/find ";
  char pipe[] = "|";
  char endGrepCommand[] = "/bin/grep -i";
  char *superstrcatargv[] = { BASE_PATH, pipe, endGrepCommand, args};  

  superstrcat(baseCommand, superstrcatargv, 4);

  printf("\nCOMMAND: %s\n", baseCommand);

  strcpy(dest, baseCommand);
};

void execCommand(char * command, char *dest) {
  FILE *fp;
  int c;
  if ((fp = popen(command, "r")) == NULL) {
    fprintf(stderr, "Faild to run command\n");
    return;
  }

  while((c = fgetc(fp)) != EOF && c != '\n') {
    *dest++ = c; 
  };

  pclose(fp);
  
  return;
}

void superstrcat(char *dest, char **argv, int n) {
  while (n-- > 0) 
    strcatwithspace(dest, *argv++);
}

void strcatwithspace(char *dest, char *s) {
  while(*dest++ != '\0')
    if (*dest == '\0') {
      *dest++ = ' ';
      while(*s)
        *dest++ = *s++;
    }
    *dest++ = ' ';
    *dest = '\0';
}