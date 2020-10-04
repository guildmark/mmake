/*
* Author: Marcus Karlström
* CS-user: c13mkm
* Lab 2 for course Systemnära Programmering 5DV088
* Program: Creates a lesser version of the make command in UNIX
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#include "parser.h"

#define MAX_LENGTH 1024
#define OPT_SPEC_TARGET 50
#define OPT_FORCE_BUILD 1
#define OPT_DEFAULT 0

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
int checkTimeDifference(const char *target, const char **prereq, struct makefile *m);
void execCommand(struct rule *rule);
void buildMakefile(char *name, char **tarArray, int option, int forceOpt);
void checkArgs(char* defaultName, int argc, char **argv);
void printCommand(char **commandArr);
void checkPrereqs(struct rule *rule, struct makefile *m);
int checkFileExists(const char *path) {
    FILE *f = fopen(path, "r");

    if(f == NULL) {
        return 0;
    }
    fclose(f);

    return 1;
}




int main(int argc, char **argv) {
    //t
    char *defaultName= "mmakefilebig";
    char **defaultTar = (char **) malloc(sizeof(char));

    if(defaultTar == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    //If no arguments, build file "mmakefile"
    if(argc == 1) {
        //printf("Building file with option value: %d, file: %s\n", 0, defaultName);
        buildMakefile(defaultName, defaultTar, 0, 0);
    }
    //File with arguments
    else {
        checkArgs(defaultName, argc, argv);
    }
    
    //Free memory
    free(defaultTar);
    return 0;
}

void checkArgs(char* defaultName, int argc, char **argv) {

    char *fileName;
    char **targetArr = (char **) calloc(sizeof(char), 10);

    if(targetArr == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    int opt;
    int temp = 0;
    int option = OPT_DEFAULT;
    int forceOpt = OPT_DEFAULT;

    //Do different things depending on the flag used
    while((opt = getopt(argc, argv, "Bsf:")) != -1) {
        switch(opt) {
            case 'f':
                // Let user specify a certain makefile
                if(sizeof(optarg) > sizeof(defaultName)) {
                    //Reallocate memory for argument, exit with error if failed
                    fileName = (char *) realloc(fileName, sizeof(optarg)+1);

                    if(fileName == NULL) {
                        perror("Error reallocating memory");
                        exit(EXIT_FAILURE);
                    }
                }
                fileName = strdup(optarg);
                temp = 1;  

                break;
            case 's':

                close(STDOUT_FILENO);
                break;

            case 'B':
                forceOpt = OPT_FORCE_BUILD;

                break;
            default:

                printf("Unknown option!\n");
                exit(EXIT_SUCCESS);
        }
    }
    
    //Let user choose a list of targets or go with default
    int targetCount = 0;

    //printf("Loops: %d\n", argc-optind);
    for(int i = 0; i < argc-optind; i++) {
        //Allocate enough size for each new target argument

        targetArr = realloc(targetArr, sizeof(targetArr)+(sizeof(argv[optind+i])));
            
        if(targetArr == NULL) {
            perror("Error reallocating memory");
            exit(EXIT_FAILURE);
        }

        targetArr[i] = argv[optind + i];

        //printf("TargetArr[%d]: %s\n", i, targetArr[i]);
        targetCount++;
    }
    
    //Either check all targets in the rule or specific targets, use option to decide
    if(targetArr[0] != NULL) {
        option += (OPT_SPEC_TARGET+targetCount);
    }

    //Parse the makefile and build the targets
    if(temp == 0) {
        buildMakefile(defaultName, targetArr, option, forceOpt);
    }
    else {
        buildMakefile(fileName, targetArr, option, forceOpt);
        free(fileName);
    }

    //Free the used memo
    free(targetArr);
}


void buildMakefile(char *name, char **tarArray, int option, int forceOpt) {

    //Open file for reading
    //const char *fileName = name;
    FILE *f = fopen(name, "r");

    //if failed, exit with error
    if(f == NULL) {
        perror("File error");
        exit(EXIT_FAILURE);
    }

    //Parse makefile, exit if failed
    makefile *m = parse_makefile(f);

    if(m == NULL) {
        fprintf(stderr, "Failed to parse makefile!\n");
        exit(EXIT_FAILURE);
    }

    //Get default target in current folder or specified target
    const char *target;

    //For a certain option, try to build specific targets
    if(option > OPT_SPEC_TARGET) {

        for(int i = 0; i < option-OPT_SPEC_TARGET; i++) {
            
            target = tarArray[i];

            struct rule *rules = makefile_rule(m, target); 

            //printRule(rules);

            const char **prereq = rule_prereq(rules);

            
            if(checkTimeDifference(target, prereq, m) == 1 || forceOpt == OPT_FORCE_BUILD) {
                char **commandArgs = rule_cmd(rules);
                printCommand(commandArgs);
                execCommand(rules);
            }
        }
    }
    //If no option given, build all targets in file
    else {
        
        target = makefile_default_target(m);

        //printf("Default target: %s\n", target);

        //Check makefile rules for default target
        struct rule *rules = makefile_rule(m, target);

        //printRule(rules);
            
        const char **prereq = rule_prereq(rules);
        /*
        Om det finns en regel för någon av prerequisite-filerna ska mmake utföra samma
        logik rekursivt för att uppdatera prerequisite-filerna innan den avgör om den
        nuvarande regeln behöver behöver utföras. 
        */
       printf("Checking prereqs...\n");
       //Check prereqs
       checkPrereqs(rules, m);
       
        if(checkTimeDifference(target, prereq, m) == 1 || forceOpt == OPT_FORCE_BUILD) {
            //printf("(1) Building [%s]...\n", target);
            char **commandArgs = rule_cmd(rules);
            printCommand(commandArgs);
            execCommand(rules);
        }

        //printf("Checking for more rules..\n");

        //Check if building is needed for each rule in the makefile
        while(rules->next != NULL) {

            rules = rules->next;
            target = rules->target;
            //printf("Next target: %s\n", target);
            prereq = rule_prereq(rules);
            //printRule(rules);
            //For every rule, check the prereq and if they need updating first
            checkPrereqs(rules, m);

            if(checkTimeDifference(target, prereq, m) == 1 || forceOpt == OPT_FORCE_BUILD) {
                char **commandArgs = rule_cmd(rules);
                printCommand(commandArgs);
                //printf("(2) Building [%s]...\n", target);
                execCommand(rules);
            }

        }
    }

    //free(target);
    fclose(f);
    makefile_del(m);
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

int checkTimeDifference(const char *target, const char **prereq, struct makefile *m) {

    //Might just use difftime (?)
    struct stat targetStat;
    struct stat prereqStat;


    //printf("Checking target stats\n");

    //If the file doesn't exist, return 1 to build
    if(stat(target, &targetStat) < 0) {
        printf("File doesnt exist, returning to build..\n");
        return 1;
    }
    //printf("Access time  = %ld\n",targetStat.st_atime);
    //printf("Target modification time  = %ld\n",targetStat.st_mtime);

    //printf("\n");

    //Get the latest modification time of the target file
    time_t targetTime = targetStat.st_mtime;

    int i = 0;
    //Check all prerequisites
    while(prereq[i] != NULL) {

        /*
        //Check if the prerequisite files exist
        if(!checkFileExists(prereq[i])) {
            //Check rules
             struct rule *rules = makefile_rule(m, prereq[i]);
            if(rules == NULL) {
                fprintf(stderr, "Error! No rule to make %s, exiting...\n", prereq[i]);
                exit(EXIT_FAILURE);
            }
        }
        */
        if(stat(prereq[i], &prereqStat) < 0) { 
            //Check if theres a rule to build the prereq, exit with message if there is not
            //printf("Prereq file doesnt exist, checking for rule to build...\n");
            struct rule *rules = makefile_rule(m, prereq[i]);
            if(rules == NULL) {
                fprintf(stderr, "Error! No rule to make %s\n", prereq[i]);
                exit(EXIT_FAILURE);
            }
            
        }
        

        time_t prereqTime = prereqStat.st_mtime;

        //Check time difference between target file and preqreq file
        long timeDiff = targetTime - prereqTime;
        //printf("Time difference (%ld - %ld): %ld\n", targetTime, prereqTime, timeDiff);

        //If the prereq file has been modified later than the target, return 1
        if(timeDiff < 0) {
            return 1;
        }

        i++;
    }

    return 0;
}

void execCommand(struct rule *rule) {

    //Create new process, exit if problem occurs
    pid_t pid = fork();
    
    if(pid < 0) {
        perror("Error creating process");
        exit(EXIT_FAILURE);
    }
    //Child process
    if(pid == 0) {
        //Execute the command with arguments
        char **commandArgs = rule_cmd(rule);

        if(execvp(commandArgs[0], commandArgs) < 0) {
            perror("Error executing program");
            exit(EXIT_FAILURE);
        }
                
    }
    //If parent, wait for child
    else {
        //Parent
        int status;
        //printf("Waiting for child..\n");
        wait(&status);
    }
}

/*
* TESTFUNCTION for printing rules
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


void printCommand(char **commandArr) {

    char *command = (char *) malloc(sizeof(commandArr[0])+1);

    strcpy(command, commandArr[0]);

    int i = 1;

    while(commandArr[i] != NULL) {
        
        command = (char *) realloc(command, (sizeof(command)+sizeof(commandArr[i])+5));
        strcat(command, " ");
        strcat(command, commandArr[i]);
        
        i++;
    }
    //printf("%s\n", command);
    fprintf(stdout, "%s\n", command);

    free(command);
}

void checkPrereqs(struct rule *rule, struct makefile *m) {
    const char **prereq = rule_prereq(rule);
    const char **tempPrereq;
    
    int i = 0;

    //printf("Checking for prereq targets...\n");
    while(prereq[i] != NULL) {
        //Check if prereq file is a target file

        printf("Checking prereq [%s] for rules...\n", prereq[i]);
        struct rule *tempRule = makefile_rule(m, prereq[i]);

        //If the prereq has a rule, check if it needs to be built
        if(tempRule != NULL) {
            printf("Prereq [%s] has a rule, checking prereqs recursively...\n", prereq[i]);
            
            checkPrereqs(tempRule, m);

            tempPrereq = rule_prereq(tempRule);
            
            if(checkTimeDifference(prereq[i], tempPrereq, m) == 1) {
                    printf("Prereq [%s] has a rule and needs building...\n", prereq[i]);
                    char **commandArgs = rule_cmd(tempRule);
                    printCommand(commandArgs);
                    execCommand(tempRule);
            }
            else {
                
                printf("[%s] does not need building..\n", prereq[i]);
            }
        }
        
        i++;
    }
}
