#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#include "parser.h"

#define MAX_LENGTH 1024

struct makefile {
	struct rule *rules;
};

struct rule {
	char *target;
	char **prereq;
	char **cmd;
	rule *next;
};


FILE *openFile(char *name);
void printRule(struct rule *rule);
int checkTimeDifference(const char *target, const char **prereq);

void execCommand(struct rule *rule) {
     pid_t pid = fork();
    //If problem occurs, exit
    if(pid < 0) {
        perror("Error creating process");
        exit(EXIT_FAILURE);
    }
    //Child process
   if(pid == 0) {
        //Child stuff
        char **commandArgs = rule_cmd(rule);
        //Execute command, exit if failed
        printf("Executing command %s\n", commandArgs[0]);
        if(execvp(commandArgs[0], commandArgs) < 0) {
            perror("Error executing program");
            exit(EXIT_FAILURE);
        }
                
    }
    else {
        //Parent
        int status;
        wait(&status);
    }
}


int main(int argc, const char **argv) {

    char *fileName = "mmakefile2";

    //Read file
    if(argc != 2) {

        //Read specific file 'mmakefile'
        FILE *f = openFile(fileName);

        //Parse makefile, exit if failed
        makefile *m = parse_makefile(f);
        if(m == NULL) {
            fprintf(stderr, "Failed to parse makefile!\n");
            exit(EXIT_FAILURE);
        }
        //Get default target in current folder
        const char *target = makefile_default_target(m);

        printf("Default target: %s\n", target);

        //Check makefile rules for default target
        struct rule *rules = makefile_rule(m, target);
    
        
        printRule(rules);

        struct rule *nextRule = rules->next;
        printRule(nextRule);
        
        const char **prereq = rule_prereq(rules);
        if(checkTimeDifference(target, prereq) == 0) {
            //BUILD
            //printf("Prereq has been modified, building...\n");
            //Create process and execute command
            execCommand(rules);
        }
        //Else if file doesnt exist
        //Else if -B flag is used



        
        //When done, close the makefile
        makefile_del(m);
    }
    //File with arguments
    else {
        printf("%s\n", argv[1]);
    }
    

    

    return 0;
}

FILE *openFile(char *name) {

    //Open file for reading
    FILE *f = fopen(name, "r");
    //if failed, exit with error
    if(f == NULL) {
        perror("File error");
         exit(EXIT_FAILURE);
    }

    return f;
}

int checkTimeDifference(const char *target, const char **prereq) {

    //Might just use difftime (?)
    struct stat targetStat;
    struct stat prereqStat;

    //Check size of string array and loop through prerequisites to check difference
    //int arrSize = sizeof(prereq) / sizeof(prereq[0]);
    //printf("Size of string array: %d\n", arrSize);

     printf("Checking target stats\n");
    if(stat(target, &targetStat) < 0) {
        fprintf(stderr, "Error checking stat!\n");
        exit(EXIT_FAILURE);
    }
    printf("Access time  = %ld\n",targetStat.st_atime);
    printf("Modification time  = %ld\n",targetStat.st_mtime);

    printf("\n");

    time_t targetTime = targetStat.st_mtime;


    int i = 0;
    //Check all prerequisites
    while(prereq[i] != NULL) {

        printf("Checking prereq stats\n");

        if(stat(prereq[i], &prereqStat) < 0) {
            fprintf(stderr, "Error checking stat!\n");
            exit(EXIT_FAILURE);
        }
        printf("File: %s\n", prereq[i]);
        printf("Access time  = %ld\n",prereqStat.st_atime);
        printf("Modification time  = %ld\n",prereqStat.st_mtime);

        time_t prereqTime = prereqStat.st_mtime;

        //Check time difference between target file and preqreq file
        //double timeDiff = difftime(targetTime, prereqTime);
        long int timeDiff = targetTime - prereqTime;
        printf("Time difference: %ld\n", timeDiff);

        if(difftime(targetTime, prereqTime) < 0) {
            return 1;
        }

        i++;

    }

    return 0;

}


/*
* TESTFUNCTION: Print contents of rule
* */
void printRule(struct rule *rule) {

    const char **prereq = rule_prereq(rule);
    char **cmd = rule_cmd(rule);

    int i = 0;
    //printf("TARGET: %s\n", rule->target);
    while(prereq[i] != NULL) {
        printf("PREREQUISITE[%d]: %s\n", i, prereq[i]);
        i++;
    }
    int j = 0;
    while(cmd[j] != NULL) {
        printf("COMMAND[%d]: %s\n", j, cmd[j]);
        j++;
    }

    printf("\n");
}
