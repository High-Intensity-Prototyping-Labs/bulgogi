#ifndef BULGOGI_CLIENT_H
#define BULGOGI_CLIENT_H

// Standard C Libraries 
#include <stdbool.h>
#include <stdlib.h>

// List of subcommands
enum commands {
        cli_none,
        cli_init,
        cli_module,
        cli_clean,
        cli_tree,
        cli_test,
};

// List of module subcommands
enum module_subcommands {
        cli_module_add,
        cli_module_rm,
};

// Args struct
struct args {
        char*   MODULE;
        char*   TARGET;
        bool    all;
        bool    create;
        bool    cached;
};

// Command definition
struct command {
        const char* name;
        const char* spacing;
        const char* desc;
        void (*callback)(struct args*);
}; 

// Parse CLI args 
// Assumes argc and argv are passed from main().
// Assumes args is non-NULL. Undefined behaviour otherwise.
// Args is populated with the results from argc and argv.
void cli(int argc, char *argv[], struct args* args);

// Runs the callback of the associated command
void callback(int argc, char *argv[], struct args* args);

// Tests all the flags
void flags(int argc, char *argv[], struct args* args);

// Prints the CLI help message 
void help(void);

// Command callbacks
void init(struct args* args);
void module(struct args* args);

// List of top-level CLI options 
struct command OPTIONS[] = {
        {"-h,--help", "\t\t", "Prints this help page", NULL},
        {0},
};

// List of module command options 
struct command* MODULE_OPTIONS = OPTIONS;

// List of module add command options 
struct command MODULE_ADD_OPTIONS[] = {
        {"--create", "\t\t", "Create directory for module (and parent target)", NULL},
        {0},
};

// List of top-level CLI commands
struct command COMMANDS[] = {
        {"init", "\t\t\t", "Initializes the project in the current directory", init},
        {"module", "\t\t", "Manage project modules", module},
        {0},
};

#endif // BULGOGI_CLIENT_H  
