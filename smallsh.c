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
#include <unistd.h>
#include <sys/types.h>

//defines
#define MAXCHARS 2048
#define MAXARGS 512
#define NUM_BUILT_INS 3

//forward functions declarations
void start(void);
char* get_command(void);
struct command* parse_command(char* command);
int run_command(struct command* arguments);
void free_args(struct command* arguments);

//declarations for built-in functions
int exit_command(struct command* arguments);
int cd_command(struct command* arguments);
int status_command(struct command* arguments);

//structs
struct command {
    int num_args;
    char** args;
};

//globals
char* built_in_commands[] = {
    "exit",
    "cd",
    "status"
};

int (*built_in_functions[]) (struct command* arguments) = {
    &exit_command,
    &cd_command,
    &status_command
};

//main function
int main(int argc, char** argv) {

    //start the shell
    start();

    return EXIT_SUCCESS;
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
        status = run_command(args);

        //free dynamically allocated memory
        free(command);      //free the initial getline string
        free_args(args);    //frees the stuff inside of the struct
        free(args);         //frees the struct

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
        
        //variables for parse_command
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


int run_command(struct command* arguments){

    int status = 1; //continues loop

    //handles an empty command line or line that begins with #
    if(arguments->num_args == 0 || arguments->args[0][0] == '#') 
    {
        return status; 
    }

    //if the user types exit(needs to be replaced later)
    if(!strcmp(arguments->args[0], "exit")){
        status = 0;
        return status;
    }

    //check for built-in commands
    for(int i = 0; i<NUM_BUILT_INS; i++){
        if(strcmp(arguments->args[0], built_in_commands[i]) == 0){
            return (*built_in_functions[i])(arguments);
        }
    }

    //fill in for a command running
    fprintf(stdout, "*****run command*****\n");

    return status;
}


void free_args(struct command* arguments){
    for(int i = 0; i<arguments->num_args; i++){
        free(arguments->args[i]);
    }
    free(arguments->args);
}


int exit_command(struct command* arguments)
{
    return 0;
}


int cd_command(struct command* arguments)
{
    //get environmental variable HOME
    if(arguments->num_args == 1){
            char* home = getenv("HOME");
            if(chdir(home) != 0) {
                perror("smallsh");
            }
    }else 
    if (arguments->num_args == 2){
            if(chdir(arguments->args[1]) != 0) {
                perror("smallsh");
            }
    }
    return 1;
}


int status_command(struct command* arguments)
{
    return 0;
}