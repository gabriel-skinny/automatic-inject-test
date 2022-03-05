#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "syscall.h"
#include <fcntl.h>
#include <unistd.h>
#include "config.h"
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
void formatDependencies(Variables *vars[], char *dest);
void typingvariables(char *sut, Variables *vars[], char *dest);
void makeinterface(char *varname, char *dest);
void maketestsuit(char* sut, Variables *vars[], char *dest);
void writetestinfile(char *testsuit, char *sutfilepath, char*sut);
void makeimport(Variables *vars[], char *dest);
void findpathforvar(char *name, char *dest);
void instanciatingvars(Variables *vars[], char *dest);



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
  formatDependencies(vars, alldependencies);

  sprintf(dest, "sut = new %s(%s);", sut, alldependencies);
}

void typingvariables(char *sut, Variables *vars[], char *dest) {
  char *temp = (char *) malloc(MAXVARNAME);

  char *sutinterface = (char *) malloc(MAXVARNAME);
  makeinterface(sut, sutinterface);
  sprintf(temp, "  let sut: %s;\n", sutinterface);
  strcat(dest, temp);

  while ((*vars) -> name != NULL) {
    char *interface = (char *) malloc(MAXVARNAME);
    makeinterface((*vars) -> name, interface);

    sprintf(temp, "  let %s: %s;\n", (*vars++) -> name, interface);
    strcat(dest, temp);
  }
}

void instanciatingvars(Variables *vars[], char *dest) { 
  while ((*vars) -> name != NULL) {
    char *temp = (char *) malloc(MAXVARNAME);
    char *class = (char *) malloc(MAXVARNAME);
    strcpy(class, (*vars) -> name);
    class[0] = class[0] - ('a' - 'A');

    sprintf(temp, "     %s = new %s();\n", (*vars++) -> name, class);
    strcat(dest, temp);
  }
}

void makeimport(Variables *vars[], char *dest) {
  while((*vars) -> name != NULL) {
    char *import = (char *) malloc(MAXFILEPATH);
    char *interface = (char *) malloc(MAXVARNAME);
    char *pathtointerface = (char *) malloc(MAXFILEPATH);

    makeinterface((*vars) -> name, interface);
    findpathforvar((*vars) -> name, pathtointerface);

    sprintf(import, "import { %s } from 'src/%s';\n", interface, pathtointerface);
    strcat(dest, import);

    free(import);
    free(interface);
    free(pathtointerface);

    *vars++;
  }
}

void findpathforvar(char *name, char *dest) {
  char *base_src = "/src/";
  int limit;
  char *command = (char *) malloc(MAXVARNAME);
  char *rigth_path = (char *) malloc(MAXFILEPATH);

  buildCommand(name, command);
  execCommand(command, rigth_path);
  printf("\n Path: %s\n", rigth_path);

  limit = 0;

  
  while(*rigth_path != '\0') {
    if (*rigth_path++ == base_src[limit])
      limit++;
    else limit = 0;
    if (limit == strlen(base_src)) {
      while(*rigth_path != '\0')
        *dest++ = *rigth_path++;

      *dest = '\0';
      break;
    }
  }
   
}

void maketestsuit(char* sut, Variables *vars[], char *dest) {
  char *dependencies = (char * )malloc(MAXFILENAME);
  char *classes = (char *) malloc(500);
  char *varlines = (char * )malloc(500);
  char *imports = (char *) malloc(500);


  typingvariables(sut, vars, varlines);
  instanciatingvars(vars, classes);
  makeDependencieinjection(sut, vars, dependencies);
  makeimport(vars, imports);

  sprintf(dest, "%s\n\ndescribe('%s', () => {\n%s\n  beforeAll(() => {\n%s\n     %s\n  }); \n});", imports, sut, varlines, classes, dependencies);
}

void writetestinfile(char *testsuit, char *sutfilepath, char*sut) {
  int fd, len;
  char *testfilepath = (char *) malloc(MAXVARNAME);
  char *testpath = (char *) malloc(MAXVARNAME);
  char *pointertotest = testpath;
  unsigned int mode = 00777;

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
  sprintf(testpath, "%s/tests", testpath, sut);
  sprintf(testfilepath, "%s/%s.spec.ts", testpath, sut);

  retry:
  if((fd = creat(testfilepath, 'w')) == -1) {
    if (errno == 2) {
      printf("\nNo test directory found in: %s\n", testfilepath);
      printf("\nCreating test directory...\n");
      if (mkdir(testpath, mode) == -1) 
        fprintf(stderr, "Error creating directory: %s\n", strerror(errno));
      else  {
        printf("\nDirectory created \n");
        goto retry;
      };
    }
    fprintf(stderr, "Error creating file: %s\n", strerror(errno));
    exit(1);
  }

  printf("\nTest writen in : %s", testfilepath);

  write(fd, testsuit, strlen(testsuit));

  if (fchmod(fd, mode) == -1) {
    fprintf(stderr, "Could not change file permission: %s\n", strerror(errno));
    exit(1);
  }

  close(fd);
}

void makeinterface(char *varname, char *dest) {
  sprintf(dest, "I%s", varname);
  if (dest[1] >= 'a' && dest[1] <= 'z') 
    dest[1] = dest[1] - ('a' - 'A');
  
}

void formatDependencies(Variables *vars[], char *dest) {
  while((*vars) -> name != NULL) {
   /*  char *classCapitalized = (char *) malloc(100);
    strcpy(classCapitalized, (*vars) -> name);
    classCapitalized[0] = classCapitalized[0] - ('a' - 'A');   */
    
    char *formated = (char *) malloc(100);

    sprintf(formated, "%s,", (*vars) -> name);
    strcat(dest, formated);
    
    *vars++;
  }

  dest[strlen(dest) - 1] = '\0';
}

void getdependencies(char *constructorlines[], Variables* destVar[]) {
  for (int i = 0; i < MAXCONSTRUCTORLINES && constructorlines[i]; i++){
    char *name = (char *)malloc(MAXCONSTRUCTORSIZELINES);

    if (strstr(constructorlines[i], "Repository")){
      if (strstr(constructorlines[i], "Inject")) i++;
      getvariablename(constructorlines[i], name);
      (*destVar) -> type = "repository"; 
      (*destVar) -> name = name;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "UseCase")) {
      if (strstr(constructorlines[i], "Inject")) i++;
      getvariablename(constructorlines[i], name);
      (*destVar) -> type = "service"; 
      (*destVar) -> name = name;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "Format")) {
      if (strstr(constructorlines[i], "Inject")) i++;
      getvariablename(constructorlines[i], name);
      (*destVar) -> type = "helper"; 
      (*destVar) -> name = name;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "Verify")) {
      if (strstr(constructorlines[i], "Inject")) i++;
      getvariablename(constructorlines[i], name);
      (*destVar) -> type = "helper"; 
      (*destVar) -> name = name;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "Factory")) {
      if (strstr(constructorlines[i], "Inject")) i++;
      getvariablename(constructorlines[i], name);
      (*destVar) -> type = "factory"; 
      (*destVar) -> name = name;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "Helper")) {
      if (strstr(constructorlines[i], "Inject")) i++;
      getvariablename(constructorlines[i], name);
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

  sprintf(dest, "%s %s %s %s %s*", baseCommand, BASE_PATH, pipe, endGrepCommand, arg);
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