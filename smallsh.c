/*
Author: Patrick Furbert
Date: 1/28/2021
Class: CS344 - Operating Systems
*/

/*
Description: smallsh shell emulator written in c - can execute typical commands found in bash shell
*/

//includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>


//defines
#define MAXCHARS 2048
#define MAXARGS 512
#define NUM_BUILT_INS 3

//forward functions declarations
void start(void);
char* get_command(void);
struct command* parse_command(char* command);
int run_command(struct command* arguments);
int launch_execvp(struct command* arguments);
void free_args(struct command* arguments);
void store_pid(pid_t pid);
char* string_replace(char* source, char* substring, char* new_string);
void handle_sigtstp(int signo);

//declarations for built-in functions
int exit_command(struct command* arguments);
int cd_command(struct command* arguments);
int status_command(struct command* arguments);

//struct that holds information for command
struct command {
    int num_args;
    char** args;
    char *output_redir, *input_redir;
    bool redirection, background;
};

//global variables
char* built_in_commands[] = {
    "exit",
    "cd",
    "status"
};
int exit_status = 0; //holds value of exit status
int process_counter = 0; //counter for number of processes created
int pid_array[500];  //array for number of process
bool background_allowed = true; //boolean for allowing background processes
bool sigtspted = false;

//array of built-in functions
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

//start the shell portion of the program
void start(void) {

    char* command;
    struct command* args; 
    int status; 

    //ignore ctrl+c / SIGINT
    struct sigaction ignore_sigint = {0};
    ignore_sigint.sa_handler = SIG_IGN;
    sigaction(SIGINT, &ignore_sigint, NULL);

    //handle ctrl+v / SIGTSTP
    struct sigaction sigstsp_action = {0};
    sigstsp_action.sa_handler = handle_sigtstp;
    sigstsp_action.sa_flags = 0;
    sigfillset(&sigstsp_action.sa_mask);
    sigaction(SIGTSTP, &sigstsp_action, NULL);

    //loop
    do {

        //initiate prompt
        fprintf(stdout, ": ");
        fflush(stdout);

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

    //exiting the smallsh and killing all stored PIDs
    for ( int i= 0; i < process_counter; i++) {
         kill(pid_array[i], SIGTERM); // kill the pid    
    }
    
}

//get command from the user
char* get_command(void) {

        //variables for getline
        size_t len = MAXCHARS;
        ssize_t chars_read;
        char* command = (char*)malloc(len * sizeof(char));
        
        //get input from user
        chars_read = getline(&command, &len, stdin); //--> getline calls realloc if the buffer is not larger enough

        //check for error
        if(chars_read == -1){
            clearerr(stdin); //reset stdin status
        }
        
        //replace '\n' with '\0'
        if(chars_read != -1 && command[chars_read-1] == '\n'){
            command[chars_read-1] = '\0';
        }

        return command;
}

//parse command 
struct command* parse_command(char* command){
        
        //variables for parse_command
        char *saveptr, *token, *delimiter = " \t\n\r\a";
        char** args = (char**)malloc(MAXARGS * sizeof(char*));
        char* str = (char*)malloc(sizeof(char) * MAXCHARS);
       // bool finished_args = false;
        int index = 0;

        //struct initialization
        struct command* my_command = (struct command*)malloc(sizeof(struct command));
        
        //redirection files
        my_command->input_redir = NULL;
        my_command->output_redir = NULL;

        //check for redirection symbol
        if((strstr(command,">") != NULL) || (strstr(command,"<") != NULL)){
            my_command->redirection = true;
        }
        else{
            my_command->redirection = false;
        }

        //copy the command into the string array
        strcpy(str, command);

        //check for $$ - variable expansion
        if(strstr(str, "$$") != NULL){
            int pid = getpid(); //get pid
            char mypid[21]; //create arbitrary sized buffer to store PID
            sprintf(mypid, "%d", pid);
            
            //check for every occurence of $$ in string and replace with PID
            do{
                str = string_replace(str, "$$", mypid);
            }while(strstr(str, "$$") != NULL);
        }

        //use for loop structure for generating tokens
        for(token = strtok_r(str, delimiter, &saveptr);
            token != NULL;
            token = strtok_r(NULL, delimiter, &saveptr))
            {
                //check if strtok returned pointer or NULL
                if(token){
                    //if there is redirection in the command we must find it
                    if(my_command->redirection){
                        if(!strcmp(token, ">")){ //found output redirection
                            token = strtok_r(NULL, delimiter, &saveptr); //grab the next token 
                            my_command->output_redir = (char*)malloc((strlen(token)+1) * sizeof(char));
                            strcpy(my_command->output_redir, token);
                        }else if(!strcmp(token, "<")){ //found input redirection
                            token = strtok_r(NULL, delimiter, &saveptr); //grab the next token
                            my_command->input_redir = (char*)malloc((strlen(token)+1) * sizeof(char));
                            strcpy(my_command->input_redir, token);
                        }else{ //loop as normal and add items to args
                            args[index] = (char*)malloc((strlen(token)+1) * sizeof(char)); 
                            strcpy(args[index++], token);
                        }
                    }
                    else
                    {   //loop as normal and add items to args
                        args[index] = (char*)malloc((strlen(token)+1) * sizeof(char));
                        strcpy(args[index++], token); 
                    }
                }
            }

        //set the last element in args to NULL for execvp
        args[index] = NULL;

        //check for & 
        if((index > 0) && !(strcmp(args[index-1], "&"))){ // this line requires short-circuit && 
            args[--index] = NULL; //change the & element to NULL
            my_command->background = true & background_allowed; //set command struct background boolean to true
        }else{
            my_command->background = false; //not a background command and set attribute to false
        }

        //free the dynamically allocated memory for str
        free(str);

        //assign args len to num_args and the pointer to the array to args
        my_command->num_args = index;
        my_command->args = args;

        return my_command;
}

//run the parsed command
int run_command(struct command* arguments){
     int status = 1; //continues loop

    //handles an empty command line or line that begins with #
    if(arguments->num_args == 0 || arguments->args[0][0] == '#') 
    {
        return status; 
    }

    //check for built-in commands
    for(int i = 0; i<NUM_BUILT_INS; i++){
        if(strcmp(arguments->args[0], built_in_commands[i]) == 0){
            return (*built_in_functions[i])(arguments);
        }
    }

    //fill in for a command running
    return launch_execvp(arguments);

    return status;
}

//launching execvp after fork in child process
int launch_execvp(struct command* arguments){

    pid_t pid;
    int output_file_descriptor, input_file_descriptor;

    pid = fork(); //fork child process
    store_pid(pid); //store the process id of the child process

    if(pid == 0){ //fork child
        //in the child

        if(arguments->redirection){

            //input redirection '<'
            if(arguments->input_redir){
                if((input_file_descriptor = open(arguments->input_redir, O_RDONLY)) < 0) {
                    perror("Couldn't open input file");
                    exit(1);
                }
                dup2(input_file_descriptor, STDIN_FILENO);
                close(input_file_descriptor);
            }

            //output redirection '>'
            if(arguments->output_redir){
                if((output_file_descriptor = creat(arguments->output_redir, 0644)) < 0) {
                    perror("Couldn't open the output file");
                    exit(1);
                }
                dup2(output_file_descriptor, STDOUT_FILENO);
                close(output_file_descriptor);
            }

        }else if(arguments->background){
            freopen ("/dev/null", "w", stdout); // or "nul" instead of "/dev/null"
            freopen ("/dev/null", "r", stdin); // or "nul" instead of "/dev/null" 
            fcloseall();// closing all of the streams
        }

        if(!arguments->background){
                //default ctrl+c / SIGINT
                struct sigaction default_action = {0};
                default_action.sa_handler = SIG_DFL;
                sigaction(SIGINT, &default_action, NULL);
        }

        //ignore ctrl + v SIGTSTP (background and foreground)
        struct sigaction ignore_action = {0};
        ignore_action.sa_handler = SIG_IGN;
        sigaction(SIGTSTP, &ignore_action, NULL);


        //execute command and check for failure (-1)
        if(execvp(arguments->args[0], arguments->args) == -1){
            perror("smallsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0 ) {
        //fork returns -1 on failure
        perror("smallsh");
    } else {
            //in the parent
            if(arguments->background) {
                waitpid(pid, &exit_status, WNOHANG); 
                fprintf(stdout, "background pid is %d\n", pid);
                fflush(stdout);
            }
            else{
                waitpid(pid, &exit_status, 0);
            }

            while ((pid = waitpid(-1, &exit_status, WNOHANG)) > 0) { // wait to read out children deaths
                fprintf(stdout,"background pid %d is done: ", pid); //print out that the background process has completed
                status_command(arguments); //status of the deceased child 
                fflush(stdout);
	        }
    }

    return 1;
}

//free up the dynamically allocated memory in the command struct
void free_args(struct command* arguments){

    //free output redirection string
    if(arguments->output_redir){
        free(arguments->output_redir);
    }

    //free input redirection string
    if(arguments->input_redir){
        free(arguments->input_redir);
    }

    //free all arguments
    for(int i = 0; i<arguments->num_args; i++){
        free(arguments->args[i]);
    }

    //free pointer to args
    free(arguments->args);

}

//substring replace function
char* string_replace(char* source, char* substring, char* new_string){

    char* substring_source;
    char* source_cpy = source;
    int difference;


    //check if there is enough length for the replacement
    if((difference = strlen(new_string) - strlen(substring)) > 0){
        source_cpy = realloc(source_cpy, (strlen(source_cpy) + difference + 1) * sizeof(char));
    }

    //if there is no substring return null
    if((substring_source = strstr(source_cpy, substring)) == NULL) {
       return NULL;
    }

    //move part after substring spaces over to make room for new string
    memmove(
        substring_source + strlen(new_string),
        substring_source + strlen(substring),
        strlen(substring_source) - strlen(substring) + 1
    );

    //copy the new string into the spot of the substring
    memcpy(substring_source, new_string, strlen(new_string));
    return source_cpy;

}

//store processes in pid array
void store_pid(pid_t pid){
    pid_array[process_counter++] = pid;
}

//initiate exit process
int exit_command(struct command* arguments)
{
    return 0;
}

//change directory command
int cd_command(struct command* arguments)
{
    //get environmental variable HOME
    if(arguments->num_args == 1){
            char* home = getenv("HOME");
            if((chdir(home)) != 0) {
                perror("smallsh");
            }
    }else 
    if (arguments->num_args == 2){
            if((chdir(arguments->args[1])) != 0) {
                perror("smallsh");
            }
    }
    return 1;
}

//print out the status of completed command(process)
int status_command(struct command* arguments)
{
    //existed by status
    if (WIFEXITED(exit_status)){
        fprintf(stdout, "exit value: %d\n", WEXITSTATUS(exit_status));
    }
    //exited by signal
    else{
        fprintf(stdout, "terminated by signal %d\n", WTERMSIG(exit_status));
    }
    return 1;
}

//toggle if background commands are allowed
void handle_sigtstp(int signo){
    //toggle background_allowed
    background_allowed ? (background_allowed = false) : (background_allowed = true);

    //print
    char* exit = "\nExiting foreground-only mode\n";
    char* enter = "\nEntering foreground-only mode(& is now ignored)\n";
    background_allowed ? write(STDOUT_FILENO, exit, strlen(exit)) : write(STDOUT_FILENO, enter, strlen(enter));

    fflush(stdout);
}
