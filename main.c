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
#define MAXFILEFINDS 25
#define MAXCONSTRUCTORSIZELINES 100
#define MAXVAR 10
#define MAXVARNAME 200
#define MAXFILEPATH 200
#define MAXTESTSUITSIZE 1200

typedef struct {
  char *type;
  char *name;
  char *interface;
} Variables;

void readfile(char * filename, char * varname, char * varfilecontent,  char *destfilepath);
void strcatwithspace(char *dest, char *src);
void execCommand(char *command, char *dest);
void chosefilepath(char *filepaths[], int npaths, char *dest);
void buildCommand(char *args, char *dest);
void superstrcat(char *dest, char *argv[], int n);
void getcontructorlines(char *filecontent, char *dest[]);
void allocarrayofpointer(char *arg[], int size, int stringsize);
void freearrayofpointer(char *arg[], int size);
void getdependencies(char *constructorlines[], Variables* destVar[]);
void getclassnameandtype(char *variableLine, char *destname, char *destinterface);
void allocarrayofstrucvar(Variables *argv[]);
void getSut(char *filecontent, char *sut);
void makeDependencieinjection(char *sut, Variables *vars[], char *dest);
void formatDependencies(Variables *vars[], char *dest);
void typingvariables(char *sut, Variables *vars[], char *dest);
void makeinterface(char *varname, char *dest);
void maketestsuit(char* sut, Variables *vars[], char *dest, char* filecontent);
void writetestinfile(char *testsuit, char *sutfilepath, char*sut);
void makeimport(char *filecontent, Variables *vars[], char *dest);
void findpathforvar(char *name, char *dest);
void instanciatingvars(Variables *vars[], char *dest);
void creatingspyclass(Variables *vars[], char *dest);


int main(int argc, char *argv[]) {
  char *filename = (char * )malloc(MAXFILENAME);
  char *filecontent = (char * )malloc(MAXFILEBUFFER);
  char *constructorlines[MAXCONSTRUCTORLINES];
  char *sut = (char * )malloc(MAXVARNAME);
  char *testsuit = (char *) malloc(MAXTESTSUITSIZE);
  char *filepath = (char *) malloc(MAXFILEPATH);

  Variables *vars[MAXVAR];

  allocarrayofstrucvar(vars);
  allocarrayofpointer(constructorlines, MAXCONSTRUCTORLINES, MAXCONSTRUCTORSIZELINES);

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

  maketestsuit(sut, vars, testsuit, filecontent);
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
    fprintf(stderr, "\nCould not read file: %s\n", strerror(errno));
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
  char *alldependencies = (char *) malloc(300);
  formatDependencies(vars, alldependencies);

  sprintf(dest, "sut = new %s(%s);", sut, alldependencies);

  free(alldependencies);
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

void creatingspyclass(Variables *vars[], char *dest) {
  while((*vars) -> name != NULL) {
    char *temp = (char *) malloc(MAXVARNAME); 
    char *interface = (char *) malloc(MAXVARNAME);
    char *class = (char *) malloc(MAXVARNAME);

    strcpy(class, (*vars) -> name);
    class[0] = class[0] - ('a' - 'A');

    makeinterface((*vars) -> name, interface);

    sprintf(temp, "class %sSpy implements %s {};\n\n", class, interface);
    strcat(dest, temp);

    free(temp);
    free(class),
    free(interface);

    *vars++;
  }
}

void instanciatingvars(Variables *vars[], char *dest) { 
  while ((*vars) -> name != NULL) {
    char *temp = (char *) malloc(MAXVARNAME);
    char *class = (char *) malloc(MAXVARNAME);
    strcpy(class, (*vars) -> name);
    class[0] = class[0] - ('a' - 'A');

    sprintf(temp, "     %s = new %sSpy();\n", (*vars++) -> name, class);
    strcat(dest, temp);

    free(temp);
    free(class);
  }
}

void makeimport(char *filecontent, Variables *vars[], char *dest) {
  while((*vars) -> name != NULL) {
    char *interface = (char *) malloc(MAXVARNAME);
    char *import = (char *) malloc(100);
    int i, c;

    makeinterface((*vars) -> name, interface);

    for (i = 0; i < strlen(filecontent); i++) {
      c = 0;
      
      while(filecontent[i] != '\n') {
        
        import[c++] = filecontent[i++];
      }
      import[c++] = '\n';
      import[c] = '\0';

      if (strstr(import, interface)) break;
    }
   
    strcat(dest, import);
    
    free(import);

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
  
  printf("\nPaths: %s", rigth_path);

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

void maketestsuit(char* sut, Variables *vars[], char *dest, char* filecontent) {
  char *dependencies = (char * )malloc(500);
  char *classes = (char *) malloc(500);
  char *varlines = (char * )malloc(500);
  char *imports = (char *) malloc(500);
  char *spyclasses = (char * )malloc(500);

  typingvariables(sut, vars, varlines);
  instanciatingvars(vars, classes);
  creatingspyclass(vars, spyclasses);
  makeDependencieinjection(sut, vars, dependencies);
  makeimport(filecontent, vars, imports);

  sprintf(dest, "%s\n\n%s\n\ndescribe('%s', () => {\n%s\n  beforeAll(() => {\n%s\n     %s\n  }); \n});", imports, spyclasses, sut, varlines, classes, dependencies);

  free(dependencies);
  free(classes);
  free(varlines);
  free(imports);
  free(spyclasses);
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
    char *interface = (char *)malloc(MAXCONSTRUCTORSIZELINES);

    if (strstr(constructorlines[i], "Repository")){
      if (strstr(constructorlines[i], "Inject")) i++;
      getclassnameandtype(constructorlines[i], name, interface);
      (*destVar) -> type = "repository"; 
      (*destVar) -> name = name;
      (*destVar) -> interface = interface;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "UseCase")) {
      if (strstr(constructorlines[i], "Inject")) i++;
      getclassnameandtype(constructorlines[i], name, interface);
      (*destVar) -> type = "service"; 
      (*destVar) -> name = name;
      (*destVar) -> interface = interface;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "Format")) {
      if (strstr(constructorlines[i], "Inject")) i++;
      getclassnameandtype(constructorlines[i], name, interface);
      (*destVar) -> type = "helper"; 
      (*destVar) -> name = name;
      (*destVar) -> interface = interface;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "Verify")) {
      if (strstr(constructorlines[i], "Inject")) i++;
      getclassnameandtype(constructorlines[i], name, interface);
      (*destVar) -> type = "helper"; 
      (*destVar) -> name = name;
      (*destVar) -> interface = interface;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "Factory")) {
      if (strstr(constructorlines[i], "Inject")) i++;
      getclassnameandtype(constructorlines[i], name, interface);
      (*destVar) -> type = "factory"; 
      (*destVar) -> name = name;
      (*destVar) -> interface = interface;
      *destVar++;
    }

    else if (strstr(constructorlines[i], "Helper")) {
      if (strstr(constructorlines[i], "Inject")) i++;
      getclassnameandtype(constructorlines[i], name, interface);
      (*destVar) -> type = "helper"; 
      (*destVar) -> name = name;
      (*destVar) -> interface = interface;
      *destVar++;
    }
  }
}

void getclassnameandtype(char *variableLine, char *destname, char *destinterface) {
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

      while(variableLine[i] != ',') 
        *destinterface++ = variableLine[++i];
      
      *--destinterface = '\0';
      break;
    }
  }

  
}

void buildCommand(char *arg, char *dest) {
  char baseCommand[150] = "/bin/find";
  char pipe[] = "|";
  char endGrepCommand[] = "/bin/grep -i";

  sprintf(dest, "%s %s %s %s %s*", baseCommand, BASE_PATH, pipe, endGrepCommand, arg);
};

void execCommand(char * command, char *dest) {
  FILE *fp;
  int c, npaths;
  char *filepaths[MAXFILEFINDS];
  allocarrayofpointer(filepaths, MAXFILEFINDS, MAXFILEPATH * sizeof(char));

  if ((fp = popen(command, "r")) == NULL) {
    fprintf(stderr, "Faild to run command\n");
    return;
  }

  npaths = 0;

  for(int i = 0; (c = fgetc(fp)) != EOF; i++) {
    if (c == '\n') {
      filepaths[npaths++][i] = '\0';
      i = -1;
    }
    else 
      filepaths[npaths][i] = c;     
      
  };

  chosefilepath(filepaths, npaths, dest);
  
  freearrayofpointer(filepaths, MAXVAR);
  pclose(fp);
  
  return;
}

void chosefilepath(char *filepaths[], int npaths, char *dest) {
  if (npaths == 1) {
    strcpy(dest, filepaths[npaths]);
  
    return;
  }

  int filechosen, i;

  printf("\nMore than one file was found\n");
  printf("Chose one: \n");
  for (i = 0; i < npaths; i++) {
    printf(" [%d]-%s\n", i, filepaths[i]);
  }
  
  scanf("%i", &filechosen);

  strcpy(dest, filepaths[filechosen]);
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

void allocarrayofpointer(char *arg[], int size, int stringsize) {
  for (int i = 0; i < size; i++) 
    arg[i] = (char *) malloc(stringsize);
}

void freearrayofpointer(char *arg[], int size) {
  for (int i = 0; i < size; i++) 
    free(arg[i]);
}

void allocarrayofstrucvar(Variables *arg[]){
   for (int i = 0; i < MAXVAR; i++)
    arg[i] = malloc(sizeof(Variables *));
}