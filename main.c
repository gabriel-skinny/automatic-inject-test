#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "syscall.h"
#include <fcntl.h>
#include <unistd.h>
#include "main.h"

#define MAXFILENAME 100
#define MAXFILEBUFFER 500


void readfilename(char * filename, char * varname, char * varfilecontent);
void strinvertconcat(char *dest, char *src);
void execCommand(char * command, char *dest);
void buildCommand(char *args, char *dest);

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

  printf("%s", filecontent);

  return 0;
}

void readfilename(char * filename, char * globalfilename, char *filecontent) {  
  int fd;
  char command[100];

  printf("filename: %s", filename);

  buildCommand(filename, command);
  execCommand(command, filecontent);

  strinvertconcat(rigthfilepath, filename);
/* 
  if((fd = open(filename, 'r')) == -1) {
    fprintf(stderr, "File not found");
    return;
  } */

  /* read(fd, filecontent, MAXFILEBUFFER); */
}

void strinvertconcat(char *dest, char *s) {
  while(*dest++ != '\0')
    if (*dest == '\0') {
      while(*s)
        *dest++ = *s++;
    }
    *dest = '\0';
}

void buildCommand(char *args, char *dest) {
  char baseCommand[150] = "/bin/find ";
  char endGrepCommand[] = "| /bin/grep -i ";
  strcat(baseCommand, BASE_PATH);
  strcat(baseCommand, endGrepCommand);
  strcat(baseCommand, args);

  strcpy(dest, baseCommand);
};

void execCommand(char * command, char *dest) {
  FILE *fp;

  if ((fp = popen(command, "r")) == NULL) {
    fprintf(stderr, "Faild to run command\n");
    return;
  }

  while((fgets(dest, sizeof(dest), fp)) != NULL) {
    printf("Dest: %s\n", dest);
  };

  pclose(fp);
  
  return;
}