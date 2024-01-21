/**
*
* main.c 
*
* Main source file for the bulgogi C program.
*
*/

#include "client.h"

int main(int argc, char *argv[]) {
        struct args args;
        cli(argc, argv, &args);
        return 0;
}
