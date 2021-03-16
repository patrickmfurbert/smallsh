# smallsh in c
Smallsh is a shell emulator that runs similiarly to bash. It can run common commands like cd,
mkdir, exit, and many more. Not designed to handle quotes. This is a portfolio project for 
CS344 - Operating Systems I at Oregon State University. A test script is included 
to verify the program runs per the requirements.

#### To compile:
gcc -std=gnu99 -g -o smallsh smallsh.c

#### To execute the test script:
./p3testscript > mytestresults 2>&1
