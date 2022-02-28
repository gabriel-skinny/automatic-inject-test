#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "syscall.h"
#include <fcntl.h>
#include <unistd.h>
#include "main.h"
#include <ctype.h>

#define MAXFILENAME 100
#define MAXFILEBUFFER 1200
#define MAXCONSTRUCTORLINES 20
#define MAXCONSTRUCTORSIZELINES 100
#define MAXVAR 10

typedef struct {
  char *type;
  char *name;
} Variables;

void readfilename(char * filename, char * varname, char * varfilecontent);
void strcatwithspace(char *dest, char *src);
void execCommand(char *command, char *dest);
void buildCommand(char *args, char *dest);
void superstrcat(char *dest, char *argv[], int n);
void getcontructorlines(char *filecontent, char *dest[]);
void allocarrayofpointer(char *arg[]);
void assingvariables(char *constructorlines[], Variables* destVar[]);
void getvariablename(char *variableLine, char *destname);
void allocarrayofstrucvar(Variables *argv[]);


int main(int argc, char *argv[]) {
  char *filename = (char * )malloc(MAXFILENAME);
  char *filecontent = (char * )malloc(MAXFILEBUFFER);
  char *constructorlines[MAXCONSTRUCTORLINES];
  Variables *vars[MAXVAR];

  allocarrayofstrucvar(vars);
  allocarrayofpointer(constructorlines);

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
  getcontructorlines(filecontent, constructorlines);
  
  for (int i = 0; i < 6; i++) {
    printf("\n Constructor lines: %s", constructorlines[i]);
  }

  assingvariables(constructorlines, vars);

  printf("\n\n Var type: %s, Var name: %s", (*vars) -> type, (*vars) -> name);

  printf("\n\n\n");

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

void getcontructorlines(char * filecontent, char *dest[]) {
  char constructor[] = "constructor";
  char lineConstructor[400];
  int countConstructor = 0;

  for (int i = 0; i < strlen(filecontent); i++) {  
    if (filecontent[i] == constructor[countConstructor++]) 
      ;
    else 
      countConstructor = 0;

    if (countConstructor == strlen(constructor)) {
      int j = 0;
      while(filecontent[++i] != '{') 
        lineConstructor[j++] = filecontent[i];
      
      lineConstructor[j] = '\0';
      break;
    }
  }

  int destCounter = 0;
  for (int i = 0; i < strlen(lineConstructor); i++) { 
    if (lineConstructor[i] != '\0' && lineConstructor[i] != ' '){
      int k = 0;
      while (lineConstructor[i] != '\n' && i < strlen(lineConstructor)){
        dest[destCounter][k++] = lineConstructor[i++];
      }
      if (i > 1) {
        dest[destCounter][k] = '\0';
        destCounter++;
        k = 0;
      }  
    }
  }
}

void assingvariables(char *constructorlines[], Variables* destVar[]) {
  char *name = (char *)malloc(MAXCONSTRUCTORSIZELINES);
  for (int i = 0; i < MAXCONSTRUCTORLINES && constructorlines[i]; i++){
    if (strstr(constructorlines[i], "Repository")){
      getvariablename(constructorlines[++i], name);
      (*destVar) -> type = "repository"; 
      (*destVar) -> name = name;
    }
    else if (strstr(constructorlines[i], "UseCase")) {
      getvariablename(constructorlines[++i], name);
      (*destVar) -> type = "service"; 
      (*destVar) -> name = name;
    }

    else if (strstr(constructorlines[i], "Helper")) {
      getvariablename(constructorlines[++i], name);
      (*destVar) -> type = "helper"; 
      (*destVar) -> name = name;
    }
  }
}

void getvariablename(char *variableLine, char *destname) {
  char limit[] = "readonly";
  int limitCount = 0;
 
  while(*variableLine != '\0') {
     printf("\nLine here %c \n\n", *variableLine);
    if (*variableLine++ == limit[limitCount]) {
      limitCount++;
    }

    if (limitCount == strlen(limit)) {
      while(*variableLine != ':') 
        *destname++ = ++*variableLine;
      
      *destname = '\0';
    }
  }
}

void buildCommand(char *args, char *dest) {
  char baseCommand[150] = "/bin/find ";
  char pipe[] = "|";
  char endGrepCommand[] = "/bin/grep -i";
  char *superstrcatargv[] = { BASE_PATH, pipe, endGrepCommand, args};  

  superstrcat(baseCommand, superstrcatargv, 4);

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

void allocarrayofpointer(char *arg[]) {
  for (int i = 0; i < MAXCONSTRUCTORLINES; i++) 
    arg[i] = (char *) malloc(MAXCONSTRUCTORSIZELINES);
}

void allocarrayofstrucvar(Variables *arg[]){
   for (int i = 0; i < MAXVAR; i++)
    arg[i] = malloc(sizeof(Variables *));
}