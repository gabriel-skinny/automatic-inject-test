# automatic-inject-test
A CLI program that gets a file name and inject all needed dependencies to test a class in a respective test file.

## How to use it
  - First you need to edit the config.h file and set the base_path to your project src folder.
  - Then you need to compile it using gcc, no flags nedded because it only uses headers of libc
  - To execute it you need to pass the -file as an argument and then you pass the file you want to test like: `./a.out -file ${fileToTest}`

## How it works
  - Find the file using unix find command in the base src folder and uses grep to search for the specifig file, if it has multiple finds the first its picked. The file name does not need to be case sensitive.
  - Reads the file 
  - It analizes the constructor of the file passed and creates a jest like suit, injecting all the depedencies and creating typed variables.
  - It creates a jest like test file under /tests based on your sut file path 
  - Now it has a few types that are supported, these types are neded to future implement of imports. Suported types:
    - Format
    - Verify
    - Adapter
    - Repository
    - UseCase
    - Factory
    - Helper 

## Features to implement
  - Flag to use no types
  - With grep return multiple files the user shold chose witch one it want to test
  - If a test file is already created with that name it should throw an error
  - If the constructor does not have inject(nestjs like injection) it should not skip for the next line to get the depedencie
  - Make all the imports based on sut file path and varible types
  
   
 
