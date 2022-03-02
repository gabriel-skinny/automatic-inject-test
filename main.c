#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "syscall.h"
#include <fcntl.h>
#include <unistd.h>
#include "main.h"
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>

#define MAXFILENAME 100
#define MAXFILEBUFFER 1500
#define MAXCONSTRUCTORLINES 20
#define MAXCONSTRUCTORSIZELINES 100
#define MAXVAR 10
#define MAXVARNAME 200
#define MAXFILEPATH 200
#define MAXTESTSUITSIZE 1200

typedef struct {
  char *type;
  char *name;
} Variables;

void readfile(char * filename, char * varname, char * varfilecontent,  char *destfilepath);
void strcatwithspace(char *dest, char *src);
void execCommand(char *command, char *dest);
void buildCommand(char *args, char *dest);
void superstrcat(char *dest, char *argv[], int n);
void getcontructorlines(char *filecontent, char *dest[]);
void allocarrayofpointer(char *arg[]);
void getdependencies(char *constructorlines[], Variables* destVar[]);
void getvariablename(char *variableLine, char *destname);
void allocarrayofstrucvar(Variables *argv[]);
void getSut(char *filecontent, char *sut);
void makeDependencieinjection(char *sut, Variables *vars[], char *dest);
void transformVariablesinClasses(Variables *vars[], char *dest);
void assingvariables(char *sut, Variables *vars[], char *dest);
void makeinterface(char *varname, char *dest);
void maketestsuit(char* sut, Variables *vars[], char *dest);
void writetestinfile(char *testsuit, char *sutfilepath, char*sut);



int main(int argc, char *argv[]) {
  char *filename = (char * )malloc(MAXFILENAME);
  char *filecontent = (char * )malloc(MAXFILEBUFFER);
  char *constructorlines[MAXCONSTRUCTORLINES];
  char *sut = (char * )malloc(MAXVARNAME);
  char *testsuit = (char *) malloc(MAXTESTSUITSIZE);
  char *filepath = (char *) malloc(MAXFILEPATH);

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
        readfile(*++argv, filename, filecontent, filepath);  
    }   
  }

  getcontructorlines(filecontent, constructorlines);

  getSut(filecontent, sut);
  getdependencies(constructorlines, vars);

  printf("\nSut is: %s \n", sut);

  maketestsuit(sut, vars, testsuit);
  writetestinfile(testsuit, filepath, sut);

  printf("\n\n\n");

  return 0;
}

void readfile(char * filename, char * globalfilename, char *filecontent, char *destfilepath) {  
  int fd;
  char command[100];
  char filepath[100];

  buildCommand(filename, command);
  execCommand(command, filepath);

  strcpy(destfilepath,filepath);

  if((fd = open(filepath, 'r')) == -1) {
    fprintf(stderr, "File not found");
    exit(1);
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
  char *limit = "export class";
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

        if (*filecontent == '\n') break;

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
      *(sut - 1) = '\0';
      break; 
    }
  }
}

void makeDependencieinjection(char *sut, Variables *vars[], char *dest) {
  char *alldependencies = (char *)malloc(200);
  transformVariablesinClasses(vars, alldependencies);

  sprintf(dest, "new %s(%s)", sut, alldependencies);
}

void assingvariables(char *sut, Variables *vars[], char *dest) {
  char *temp = (char *) malloc(MAXVARNAME);

  char *sutinterface = (char *) malloc(MAXVARNAME);
  sprintf(temp, "let sut: %s = new %s()\n", sutinterface, sut);
  strcat(dest, temp);

  while ((*vars) -> name != NULL) {
    char *interface = (char *) malloc(MAXVARNAME);
    makeinterface((*vars) -> name, interface);

    sprintf(temp, "let %s: %s = new %s()\n", (*vars++) -> name, interface, (*vars) -> name);
    strcat(dest, temp);
  }
}

void maketestsuit(char* sut, Variables *vars[], char *dest) {
  char *dependencies = (char * )malloc(MAXFILENAME);
  char *varlines = (char * )malloc(500);

  assingvariables(sut, vars, varlines);
  makeDependencieinjection(sut, vars, dependencies);

  sprintf(dest, "describe(%s, () => {\n\n %s\n beforeAll(() => {\n\n %s\n})\n})", sut, varlines, dependencies);
}

void writetestinfile(char *testsuit, char *sutfilepath, char*sut) {
  int fd, len;
  char *testfilepath = (char *) malloc(MAXVARNAME);
  char *pointertotest = testfilepath;

  len = strlen(sutfilepath);

  while(sutfilepath[--len]) {
    if (sutfilepath[len] == '/') {
       sutfilepath[len] = '\0';
      while(*sutfilepath != '\0') 
        *pointertotest++ = *sutfilepath++;
    break;
    }
  }

  sut[strlen(sut) -1] = '\0';
  sprintf(testfilepath, "%s/tests/%s.spec.ts", testfilepath, sut);


  if((fd = creat(testfilepath, 'w')) == -1) {
    fprintf(stderr, "Error creating file: %s\n", strerror(errno));
    exit(1);
  }

  printf("\nTest writen in : %s", testfilepath);

  write(fd, testsuit, strlen(testsuit));

  unsigned int mode = 00700;
  if (fchmod(fd, mode) == -1) {
    fprintf(stderr, "Could not change file permission: %s\n", strerror(errno));
    exit(1);
  }

  close(fd);
}

void makeinterface(char *varname, char *dest) {
  sprintf(dest, "I%s", varname);
  dest[1] = dest[1] - ('a' - 'A');
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

void getdependencies(char *constructorlines[], Variables* destVar[]) {
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

    else if (strstr(constructorlines[i], "Format")) {
      getvariablename(constructorlines[++i], name);
      (*destVar) -> type = "helper"; 
      (*destVar) -> name = name;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "Verify")) {
      getvariablename(constructorlines[++i], name);
      (*destVar) -> type = "helper"; 
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