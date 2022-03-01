#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "syscall.h"
#include <fcntl.h>
#include <unistd.h>
#include "main.h"
#include <ctype.h>

#define MAXFILENAME 100
#define MAXFILEBUFFER 1500
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
void getSut(char *filecontent, char *sut);
void makeDependencieinjection(char *sut, Variables *vars[], char *dest);
void transformVariablesinClasses(Variables *vars[], char *dest);


int main(int argc, char *argv[]) {
  char *filename = (char * )malloc(MAXFILENAME);
  char *filecontent = (char * )malloc(MAXFILEBUFFER);
  char *constructorlines[MAXCONSTRUCTORLINES];
  char *sut = (char * )malloc(MAXFILENAME);
  char *dependencies = (char * )malloc(MAXFILENAME);
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

  getSut(filecontent, sut);
  assingvariables(constructorlines, vars);

  printf("\n Sut is: %s \n", sut);

   for (int i = 0; i < 3; i++) {
    printf("\n\n Var name: %s, Var type: %s", (vars[i]) -> name, (vars[i]) -> type);
  }  

  makeDependencieinjection(sut, vars, dependencies);

  printf("\n Dependencies: %s", dependencies);

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

  close(fd);
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

void getSut(char *filecontent, char *sut) {
  char *limit = "class";
  int limitCount = 0;

  while (*filecontent) {
    if (*filecontent++ == limit[limitCount]) 
      limitCount++;
    else 
      limitCount = 0;

    if (limitCount == strlen(limit)) {
      char *sutlimit = "implements";
      int sutlimitCount = 0;

      while(*filecontent != '{') {
        *sut++ = *++filecontent;
        
        if (*filecontent == sutlimit[sutlimitCount]) {
          sutlimitCount++;
        }
        else{
           sutlimitCount = 0;
        }

        if (sutlimitCount == strlen(sutlimit)) {
          *(sut - sutlimitCount) = '\0';
          break;        
        }
        
      }
      break; 
    }
  }
}

void makeDependencieinjection(char *sut, Variables *vars[], char *dest) {
  char *alldependencies = (char *)malloc(200);
  transformVariablesinClasses(vars, alldependencies);

  sprintf(dest, "new %s(%s)", sut, alldependencies);
}

void transformVariablesinClasses(Variables *vars[], char *dest) {
  while((*vars) -> name != NULL) {
    char *classCapitalized = (char *) malloc(100);
    strcpy(classCapitalized, (*vars) -> name);
    classCapitalized[0] = classCapitalized[0] - ('a' - 'A');  
    
    char *class = (char *) malloc(100);

    sprintf(class, "new %s(),", classCapitalized);
    strcat(dest, class);
    
    *vars++;
  }

  dest[strlen(dest) - 1] = '\0';
}

void assingvariables(char *constructorlines[], Variables* destVar[]) {
  for (int i = 0; i < MAXCONSTRUCTORLINES && constructorlines[i]; i++){
    char *name = (char *)malloc(MAXCONSTRUCTORSIZELINES);

    if (strstr(constructorlines[i], "Repository")){
      getvariablename(constructorlines[++i], name);
      (*destVar) -> type = "repository"; 
      (*destVar) -> name = name;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "UseCase")) {
      getvariablename(constructorlines[++i], name);
      (*destVar) -> type = "service"; 
      (*destVar) -> name = name;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "Factory")) {
      getvariablename(constructorlines[++i], name);
      (*destVar) -> type = "factory"; 
      (*destVar) -> name = name;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "Helper")) {
      getvariablename(constructorlines[++i], name);
      (*destVar) -> type = "helper"; 
      (*destVar) -> name = name;
      *destVar++;
    }
  }
}

void getvariablename(char *variableLine, char *destname) {
  char *limit = (char *) malloc(20);
  int limitCount = 0;
 
  if (strstr(variableLine, "readonly")) limit = "readonly"; else limit = "private";

  for(int i = 0; i < strlen(variableLine); i++) {
    if (variableLine[i] == limit[limitCount]) 
      limitCount++;
    else 
      limitCount = 0;
    

    if (limitCount == strlen(limit)) {
      i++;
      while(variableLine[i] != ':') 
        *destname++ = variableLine[++i];
      
      *--destname = '\0';
      break;
    }
  }

  
}

void buildCommand(char *arg, char *dest) {
  char baseCommand[150] = "/bin/find ";
  char pipe[] = "|";
  char endGrepCommand[] = "/bin/grep -i";

  sprintf(dest, "%s %s %s %s %s", baseCommand, BASE_PATH, pipe, endGrepCommand, arg);
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