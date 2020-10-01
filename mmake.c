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

//ts
FILE *openFile(char *name);
void printRule(struct rule *rule);
int checkTimeDifference(const char *target, const char **prereq);
void execCommand(struct rule *rule);
void buildMakefile(char *name, char **tarArray, int option);



int main(int argc, char **argv) {
    //t
    char defaultName[] = "mmakefile";
    char *fileName = (char *) malloc(sizeof(defaultName));
    char **targetArr = (char **) malloc(sizeof(char));

    if(fileName == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    
    if(targetArr == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    //fileName = strdup("mmakefile");

    //If no arguments, build file "mmakefile"
    if(argc == 1) {
       
        printf("Building file with option value: %d, file: %s\n", 0, defaultName);
        buildMakefile(defaultName, targetArr, 0);
    }
    //File with arguments
    else {
        int opt;
        int option = OPT_DEFAULT;
        //const char **targets;
        //const char *target;

        strcpy(fileName, defaultName);

        //Do different things depending on the flag used
        while((opt = getopt(argc, argv, "Bsf:")) != -1) {
            switch(opt) {
                case 'f':
                    // Let user specify a certain makefile
                    if(sizeof(optarg) > sizeof(defaultName)) {
                        //Reallocate memory for argument
                        fileName = (char *) realloc(fileName, sizeof(optarg));

                        if(fileName == NULL) {
                            perror("Error reallocating memory");
                            exit(EXIT_FAILURE);
                        }
                    }
                    
                    strcpy(fileName, optarg);
                    //printf("Current optind: %d\n ", optind);
                    //printf("New makefile to use: %s\n", fileName);

                    break;
                case 's':
                    //close stdout
                    //printf("Current optind: %d\n ", optind);
                    //printf("Closing stdout..\n");
                    close(STDOUT_FILENO);
                    break;
                case 'B':
                    //printf("Current optind: %d\n ", optind);
                    //printf("Forcing rebuild...\n");
                    //target = argv[optind];
                    option = OPT_FORCE_BUILD;
                    //buildMakefile(fileName, NULL, 1);
                    break;
                default:
                    //Specify target
                    //printf("Current optind: %d\n ", optind);
                    printf("Unknown option!\n");
                    //const char *target = strdup(argv[1]);
                    //buildMakefile(fileName, target);
                    exit(EXIT_SUCCESS);
            }
        }

        //arguments after options are targets to build
        //target = strdup(argv[optind]);
        /*
        printf("argv[optind] = %s\n", argv[optind]);
        printf("Option: %d\n", option);
        printf("Target: %s\n", target);
        printf("Filename: %s\n", fileName);
        */
        //Let user choose a list of targets or go with default
        int targetCount = 0;
    

        printf("Loops: %d\n", argc-optind);
        for(int i = 0; i < argc-optind; i++) {
            //Allocate enough size for each new target argument
            printf("Reallocating mem for array..\n");
            targetArr = realloc(targetArr, sizeof(targetArr)+(sizeof(argv[optind+i])));
            
            if(targetArr == NULL) {
                perror("Error reallocating memory");
                exit(EXIT_FAILURE);
            }

            targetArr[i] = argv[optind + i];

            printf("TargetArr[%d]: %s\n", i, targetArr[i]);
            targetCount++;
        }

        
        //Either check all targets in the rule or specific targets, use option to decide
        if(targetArr[0] != NULL) {
            option += (OPT_SPEC_TARGET+targetCount);
        }

        //Parse the makefile and build the targets
        printf("Building file with option value: %d, file: %s\n", option, fileName);
        buildMakefile(fileName, targetArr, option);

        printf("freeing file\n");
    }




    
    printf("freeing target\n");
    free(targetArr);
    free(fileName);
    

    return 0;
}
void buildMakefile(char *name, char **tarArray, int option) {

    //Open file for reading
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
        //Option is sent in as (2 + targetCount)

        printf("looping %d times\n", option-OPT_SPEC_TARGET );

        for(int i = 0; i < option-OPT_SPEC_TARGET; i++) {
            
            target = tarArray[i];

            printf("Target: %s\n", target);
            
            //Check if target exists
            
            //printf("getting rules.. for %s\n", target);
            //Check makefile rules for default target
            
            
            printf("Checking rule..\n");
            struct rule *rules = makefile_rule(m, target); 

            printRule(rules);

            //struct rule *nextRule = rules->next;
            //printRule(nextRule);
                
            const char **prereq = rule_prereq(rules);

            if(checkTimeDifference(target, prereq) == 1) {
                //BUILD
                //printf("Prereq has been modified, building...\n");
                //Create process and execute command
                printf("Executing command..\n");
                execCommand(rules);
            }
            else {
                printf("%s is already up to date.\n", target);
            }
        }
        
    }
    //If no option given, build all targets in file
    else {
        if(tarArray[0] == NULL) {
            printf("No target specified, using default...\n");
            target = makefile_default_target(m);
        }
        //Let user decide which targets to build
        else {
            target = tarArray[0];
        }

        //printf("Default target: %s\n", target);

        //Check makefile rules for default target
        struct rule *rules = makefile_rule(m, target);

        //printRule(rules);

        //struct rule *nextRule = rules->next;
        //printRule(nextRule);
            
        const char **prereq = rule_prereq(rules);

        if(checkTimeDifference(target, prereq) == 1 || option == 1) {
            //BUILD
            //printf("Prereq has been modified, building...\n");
            //Create process and execute command
            //printf("executing first command..\n");
            execCommand(rules);
        }
        else {
            printf("%s is already up to date.\n", target);
        }
        
        //printf("Checking for more rules..\n");

        //Check if building is needed for each rule
        while(rules->next != NULL) {

            //printf("Next rule..\n");
            rules = rules->next;
            //Wrong tar, need to loop anyway?
            target = rules->target;
            prereq = rule_prereq(rules);
            
            if(checkTimeDifference(target, prereq) == 1) {
                //BUILD
                //printf("Prereq has been modified, building...\n");
                //Create process and execute command¨
                //printf("Executing next command...\n");
                execCommand(rules);
            }
            else {
                printf("%s is already up to date.\n", target);
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

int checkTimeDifference(const char *target, const char **prereq) {

    //Might just use difftime (?)
    struct stat targetStat;
    struct stat prereqStat;

    //Check size of string array and loop through prerequisites to check difference
    //int arrSize = sizeof(prereq) / sizeof(prereq[0]);
    //printf("Size of string array: %d\n", arrSize);

    //printf("Checking target stats\n");

    //If the file doesn't exist, return 1 to build
    if(stat(target, &targetStat) < 0) {
        printf("File doesnt exist!\n");
        return 1;
    }
    //printf("Access time  = %ld\n",targetStat.st_atime);
    //printf("Target modification time  = %ld\n",targetStat.st_mtime);

    printf("\n");

    //Get the latest modification time of the target file
    time_t targetTime = targetStat.st_mtime;

    int i = 0;
    //Check all prerequisites
    while(prereq[i] != NULL) {

        //printf("Checking prereq stats\n");
        //Get stats for the prerequisites, exit if failed
        if(stat(prereq[i], &prereqStat) < 0) {
            fprintf(stderr, "Error checking stat!\n");
            exit(EXIT_FAILURE);
        }
        //printf("File: %s\n", prereq[i]);
        //printf("Access time  = %ld\n",prereqStat.st_atime);
        //printf("Prereq modification time  = %ld\n",prereqStat.st_mtime);

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
        char *printCom;

        //Execute command, exit if failed
        int i = 0;
        while(commandArgs[i] != NULL) {
            if(i == 0) {
                printCom = strdup(commandArgs[i]);
                strcat(printCom, " ");
            }
            else {
                strcat(printCom, commandArgs[i]);
                strcat(printCom, " ");
            }
            i++;
        }
        printf("%s\n", printCom);

        //Execute command, exit if failed
        if(execvp(commandArgs[0], commandArgs) < 0) {
            perror("Error executing program");
            exit(EXIT_FAILURE);
        }
                
    }
    else {
        //Parent
        int status;
        //printf("Waiting for child..\n");
        wait(&status);
    }
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
        //printf("PREREQUISITE[%d]: %s\n", i, prereq[i]);
        i++;
    }
    int j = 0;
    while(cmd[j] != NULL) {
        //printf("COMMAND[%d]: %s\n", j, cmd[j]);
        j++;
    }

    printf("\n");
}
