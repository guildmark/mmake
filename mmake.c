#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "parser.h"

#define TEST 10

// SHOULD NOT BE NEEDED ?
struct makefile {
	struct rule *rules;
};

struct rule {
	char *target;
	char **prereq;
	char **cmd;
	rule *next;
};


FILE *openFile(void);
void printRule(struct rule *rule);


int main(int argc, const char **argv) {

    //char testString[TEST];
    struct stat targetStat;
    //struct stat prereqStat;


    /*
    När mmake startas så kommer den ladda in alla regler från makefilen
    mmakefile i nuvarande katalog eller filen som är specificerad med -f
    flaggan. */
    //Read file

    if(argc != 2) {
        //Read specific file 'mmakefile'
        FILE *f = openFile();

        //Parse makefile, exit if failed
        makefile *m = parse_makefile(f);
        if(m == NULL) {
            fprintf(stderr, "Failed to parse makefile!\n");
            exit(EXIT_FAILURE);
        }
        //Get default target in current folder
        const char *target = makefile_default_target(m);

        printf("Default target: %s\n", target);

        //Check makefile rule
        struct rule *rules = makefile_rule(m, target);
        if(rules == NULL) {
            fprintf(stderr, "No rules in makefile!\n");
            exit(EXIT_FAILURE);
        }

        /*  Ett target ska endast byggas ifall någon av följande tre situationer
            gäller:

            Target filen existerar inte
            Tidpunkt då en prerequisite-fil modiferades är senare än tidpunkt då target-filen modiferades
            Flaggan -B används/
            */
        printRule(rules);
        //Check stat of target and prerequisites
        if(stat(target, &targetStat) < 0) {
            fprintf(stderr, "Error checking stat!\n");
            exit(EXIT_FAILURE);
        }
        printf("%s latest modified on: %ld", target, targetStat.st_atim);

        //If target file does not exist, build file
        if(target == NULL) {
            // BUILD
        }

        //When done, close the makefile
        makefile_del(m);
    }
    //File with arguments
    else {
        printf("%s\n", argv[1]);
    }
    

    

    return 0;
}

FILE *openFile(void) {

    char *fileName = "mmakefile";
    //Open file for reading
    FILE *f = fopen(fileName, "r");
    //if failed, exit with error
    if(f == NULL) {
        perror("File error");
         exit(EXIT_FAILURE);
    }

    


    return f;
}

/*
* TESTFUNCTION: Print contents of rule
* */
void printRule(struct rule *rule) {

    int i = 0;
    printf("TARGET: %s\n", rule->target);
    while(rule->prereq[i] != NULL) {
        printf("PREREQUISITE[%d]: %s\n", i, rule->prereq[i]);
        i++;
    }
    int j = 0;
    while(rule->cmd[i] != NULL) {
        printf("COMMAND[%d]: %s\n", j, rule->cmd[j]);
        j++;
    }
    printf("TARGET: %s\n", rule->target);
    printf("NEXT: %s\n", rule->next->target);
}

int fileHasChanged(void) {
    //Tidpunkt då en prerequisite-fil modiferades är senare än tidpunkt då target-filen modiferades

    return 1;
}