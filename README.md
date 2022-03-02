# automatic-inject-test
A CLI program that gets a file name and inject all needed dependencies to test a class in a respective test file.

## How to use it
  - First you need to edit the config.h file and set the base_path to your project src folder.
  - Then you need to compile it using gcc, no flags nedded because it only uses headers of libc
  - To execute it you need to pass the -file as an argument and then you pass the file you want to test like: `./a.out -file ${fileToTest}`

## How it works
  - It analizes the constructor of the file passed and creates a jest like suit injecting the depedencies.
  - Now it has a few types that are supported, these types are neded to future implement of imports. Suported types:
    - Format
    - Verify
    - Adapter
    - Repository
    - UseCase
    - Factory
    - Helper 
  
  
   
 
