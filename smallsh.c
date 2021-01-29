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

//functions declarations
void get_command(void);
void parse_command(void);


//main
int main(int argc, char** argv) {


    //initiate prompt
    get_command();

    //parse input

    //do stuff

    //repeat

    return 0;
}

void get_command(void) {

    //exit variable
    int exit = 1;

    do{

        //variables for getline
        size_t len = MAXCHARS;
        ssize_t chars_read;
        char* command = (char*)malloc(len * sizeof(char));
        


        //show prompt for command line
        fprintf(stdout, ": ");

        //flush the input

        //get input from user
        chars_read = getline(&command, &len, stdin); //--> getline calls realloc if the buffer is not larger enough

        //check for error
        //pass for now
        
        //replace '\n' with '\0'
        if(chars_read != -1 && command[chars_read-1] == '\n'){
            command[chars_read-1] = '\0';
        }

        fprintf(stdout, "chars_read: %ld | last char: %d\n", chars_read, command[chars_read-1]);

        //check length of command didn't exceed MAXCHARS
        if(strlen(command) > MAXCHARS){
            
            //print out message for user
            fprintf(stdout, "command too long\n");

            //free command (clean up)
            free(command);

            //continue next iteration
            continue;

        }

        //look for new line and remove it

        //parser

        if(!strcmp(command, "exit")){
            exit = 0;
        }

        //free dynamically allocated memory for command(clean up)
        free(command);

        
    }while(exit);


}



void parse_command(void){

}