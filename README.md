# automatic-inject-test
A CLI program that gets a file name and inject all needed dependencies to test a class in a respective test file

## How to use it
  - First you need to edit the config.h file and set the base_path to your project src folder.
  - Then you need to compile it using gcc, no flags nedded because it only uses headers of libc
  - To execute it you need to pass the -file as an argument and then you pass the file you want to test like: `./a.out -file ${fileToTest}`

## How it works
  - Find the file using unix find command in the base src folder and uses grep to search for the specific file, if it has multiple finds you can chose wich one do you wanna test. The file name does not need to be case sensitive.
  - Reads the file 
  - It analizes the constructor of the file passed and creates a jest like suit, injecting all the depedencies and creating typed variables and imports for all of them.
  - It creates a test file under /tests based on your sut file path 

## Features to implement
  - Flag to use no types
  - Flag to not build a spy class
  - If a test file is already created with that name it should throw an error
  - Filter files founded with grep to show only classes(Ex: do not show files that have protocol folder in their path or ends with spec.ts)
  - Make import of sut and sut interface
  - Get sut interface from implements rather than just changing sut name
   
 
