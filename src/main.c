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

#ifndef DEBUG
// Project Headers 
#include "fs.h"
#include "dot_bul.h"
#include "yaml_ext.h"
#include "engine.h"
#else
#include "core.h"
#include "yaml_ext.h"
#endif

// External Dependencies
#include "yaml.h"

// Settings 
#define PROJECT_YAML "project.yaml"

#ifndef DEBUG
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

static bul_id_t test_dot_bul_features(char *argv[]) {
        printf("Creating target (%s)\n", argv[2]);
        return bul_dot_add_target(argv[2], BUL_LIB);
}

static void test_longer_dot_bul_features(char *argv[]) {
        bul_id_t id = test_dot_bul_features(argv);
        bul_dot_add_sources(id, argv[3]);

}

static void some_subcommand(char *argv[]) {
        if(strcmp(argv[1], "init") == 0) {
                bul_dot_init();
                printf("Initialized .bul directory.\n");
        } else if(strcmp(argv[1], "add") == 0) {
                printf("Missing a target name\n");
                printf("Try: bul add <TARGET>\n");
        } else if(strcmp(argv[1], "add_sources") == 0) {
                printf("Missing a pattern\n");
                printf("Try: bul add_sources <TARGET> <PATTERN>\n");
        } else {
                printf("Not exactly a subcommand...\n");
        }
}
#endif

int main(int argc, char *argv[]) {
#ifndef DEBUG
        if(argc == 1) {
                load_and_print_project();
        } else if(argc == 2) {
                some_subcommand(argv);
        } else if(argc == 3) {
                test_dot_bul_features(argv);
        } else if(argc == 4) {
                test_longer_dot_bul_features(argv);
        } else {
                printf("Lovely day, but I don't recognize that number of args.\n");
        }
#else 
        bul_core_s    core;
        yaml_parser_t parser;
        yaml_event_t  event;
        FILE          *file;

        int done = 0;
        int error = 0;
 
        (void)argc;
        (void)argv;

        core = bul_core_init();

        yaml_parser_initialize(&parser);

        file = fopen(PROJECT_YAML, "rb");
        assert(file);

        yaml_parser_set_input_file(&parser, file);

        while(!done && !error) {
                if(!yaml_parser_parse(&parser, &event)) {
                        error = 1;
                        continue;
                }

                bul_core_next_event(&core, &event);
#ifdef DEBUG2
                printf("---\n");
                yaml_print_event(&event);
                printf("\n");
                bul_core_print(&core);
                printf("---\n");
#endif
                done = (event.type == YAML_STREAM_END_EVENT);
                yaml_event_delete(&event);
        }

        yaml_parser_delete(&parser);
        fclose(file);

        bul_core_print(&core);

        bul_core_free(&core);

#endif // DEBUG

        return 0;
}
