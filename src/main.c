/**
* @file main.c
* @brief Main source file for bulgogi
*
*/

// Standard C Liraries
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

// Project Headers 
#include "bul_fs.h"
#include "bul_dot_bul.h"
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
        printf("Creating target (%s)\n", argv[1]);
        bul_dot_add_target(argv[1], BUL_LIB);
}

int main(int argc, char *argv[]) {
        if(argc == 1) {
                load_and_print_project();
        } else if(argc == 2) {
                test_dot_bul_features(argv);
        } else {
                printf("Lovely day, but I don't recognize that number of args.\n");
        }

        return 0;
}
