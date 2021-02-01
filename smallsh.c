/*
Author: Patrick Furbert
Date: 1/28/2021
Class: CS344 - Operating Systems
*/


/*
Description:
*/

//includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

//defines
#define MAXCHARS 2048
#define MAXARGS 512

//forward functions declarations
void start(void);
char* get_command(void);
struct command* parse_command(char* command);
int run_command(char* command);
void free_args(struct command* arguments);

//structs
struct command {
    int num_args;
    char** args;
};

//main
int main(int argc, char** argv) {


    start();

    return 0;
}

void start(void) {

    char* command;
    struct command* args; 
    int status; 

    do {

        //initiate prompt
        fprintf(stdout, ": ");
        
        //get command line
        command = get_command();

        //parse input
        args = parse_command(command);

        //do stuff
        status = run_command(command);

        //free memory
        free(command);
        free_args(args);

    }while(status);
    
}


char* get_command(void) {


        //variables for getline
        size_t len = MAXCHARS;
        ssize_t chars_read;
        char* command = (char*)malloc(len * sizeof(char));
        
        //flush the input ?

        //get input from user
        chars_read = getline(&command, &len, stdin); //--> getline calls realloc if the buffer is not larger enough

        //check for error
        //pass for now
        
        //replace '\n' with '\0'
        if(chars_read != -1 && command[chars_read-1] == '\n'){
            command[chars_read-1] = '\0';
        }

        return command;

}


struct command* parse_command(char* command){
        
        char *saveptr, *token, *delimiter = " \t\n\r\a";
        char **args = (char**)malloc(MAXARGS * sizeof(char*));
        char str[MAXCHARS];
        int index = 0;
        struct command* my_command = (struct command*)malloc(sizeof(struct command));
        
        //copy the command into the string array
        strcpy(str, command);

        for(token = strtok_r(str, delimiter, &saveptr);
            token != NULL;
            token = strtok_r(NULL, delimiter, &saveptr))
            {
                if(token) 
                {
                    args[index] = (char*)malloc(sizeof(strlen(token)));
                    strcpy(args[index++], token);
                }
            }

        for(int i = 0; i < index; i++){
            fprintf(stdout, "%s\n", args[i]);
        }

        fprintf(stdout, "num args = %d\n", index);

        my_command->num_args = index;
        my_command->args = args;

        return my_command;
}


int run_command(char* command){

    //execute
    int status = 1; //continues loop

    if(!strcmp(command, "exit")){
        status = 0;
    }

    return status;
}

void free_args(struct command* arguments){
    for(int i = 0; i<arguments->num_args; i++){
        free(arguments->args[i]);
    }
    free(arguments->args);
}

