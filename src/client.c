#include "client.h"

// Standard C Libraries
#include <stdio.h>
#include <string.h>

void cli(int argc, char **argv, struct args* args) {
        switch(argc) {
        case 2:
                callback(argc, argv, args);
                break;
        default:
                help();
        }
}

void callback(int argc, char **argv, struct args *args) {
        for(int x = 0; COMMANDS[x].name; x++) {
                if(strcmp(argv[1], COMMANDS[x].name) == 0) {
                        COMMANDS[x].callback(args);
                }
        }
}

void help(void) {
        printf(
                "bulgogi - A flexible build-system assistant.\n"
                "Usage: bul [OPTIONS] SUBCOMMAND\n"
                "\n"
                "Options:\n"
                "  -h,--help\t\tPrint this help message.\n"
                "\n"
                "Commands:\n"
        );
        struct command* CMD = &COMMANDS[0];
        while(CMD->name) {
                printf("  %s%s%s\n", CMD->name, CMD->spacing, CMD->desc);
                CMD++;
        }
}
