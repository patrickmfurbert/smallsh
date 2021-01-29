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

void get_command(void){

    //declare variables
    char* command = (char*)malloc(MAXCHARS);
    size_t len = 0;

    //prompt user for input
    fprintf(stdout, ": ");

    //flush the input

    //get input from user
    getline(&command, len, stdin);

    //look for new line and remove it

    //parser

    //print
    fprintf(stdout, "%s", command);

    //free dynamically allocated memory for command
    free(command);
}



void parse_command(void){

}