#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "parser.h"

#define TEST 10


FILE *readFile(const char **argv);

int main(int argc, const char **argv) {

    char testString[TEST];

    //No file argument
    if(argc == 1) {
        //Check current directory
        printf("No arguments, checking dir...\n");
    }
    //Read file
    else if(argc == 2) {
        FILE *f = readFile(argv);
        while(fgets(testString, TEST, f)) {
            printf("line: %s\n", testString);
        }
    }
    //File with arguments
    else {
        printf("File with arguments\n");
    }

    

    return 0;
}

FILE *readFile(const char **argv) {

    //Open file for reading
    FILE *f = fopen(argv[1], "r");
    //if failed, exit with error
    if(f == NULL) {
        perror("File error");
         exit(EXIT_FAILURE);
    }
    


    return f;
}