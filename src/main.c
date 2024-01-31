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
#include "yaml_ext.h"
#include "engine.h"

// External Dependencies
#include "yaml.h"

// Settings 
#define PROJECT_YAML "project.yaml"

int main(void) {
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
        return 0;
}
