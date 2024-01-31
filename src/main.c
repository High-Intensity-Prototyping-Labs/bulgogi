/**
* @file main.c
* @brief Main source file for bulgogi
*
*/

// Standard C Liraries
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// Project Headers 
#include "fs.h"
#include "dot_bul.h"
#include "yaml_ext.h"
#include "engine.h"

// External Dependencies
#include "yaml.h"

// Settings 
#define PROJECT_YAML "project.yaml"

static void load_and_print_project(void) {
        bul_engine_s    engine;
        bul_valid_t     valid;
        bul_fs_status_t status;

        engine = bul_engine_init();

        status = bul_engine_from_file(&engine, PROJECT_YAML);
        if(status != BUL_FS_OK) {
                if(status == BUL_FS_ERR) {
                        perror("Unable to load project.yaml.");
                } else {
                        printf("Unknown error.\n");
                }
                exit(EXIT_FAILURE);
        }

        bul_engine_print(&engine);

        valid = bul_engine_valid(&engine);

        bul_engine_free(&engine);

        printf("PROJECT IS %s\n", (valid == BUL_VALID) ? "VALID" : "INVALID");
}

static void test_dot_bul_features(char *argv[]) {
        printf("Creating target (%s)\n", argv[2]);
        bul_dot_add_target(argv[2]);
}

static void some_subcommand(char *argv[]) {
        if(strcmp(argv[1], "init") == 0) {
                bul_dot_init();
                printf("Initialized .bul directory.\n");
        } else if(strcmp(argv[1], "add") == 0) {
                printf("Missing a target name\n");
                printf("Try: bul add <TARGET>\n");
        } else {
                printf("Not exactly a subcommand...\n");
        }
}

int main(int argc, char *argv[]) {
        if(argc == 1) {
                load_and_print_project();
        } else if(argc == 2) {
                some_subcommand(argv);
        } else if(argc == 3) {
                test_dot_bul_features(argv);
        } else {
                printf("Lovely day, but I don't recognize that number of args.\n");
        }

        return 0;
}
