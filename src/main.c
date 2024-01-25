/**
* @file main.c
* @brief Main source file for bulgogi
*
*/

// Standard C Liraries
#include <stdio.h>
#include <assert.h>

// Project Headers 
#include "yaml_ext.h"
#include "engine.h"

// External Dependencies
#include "yaml.h"

// Settings 
#define PROJECT_YAML "project.yaml"

int main(void) {
        bul_engine_s    engine;

        yaml_parser_t   parser;
        yaml_event_t    event;
        FILE            *file;

        int done  = 0;
        int count = 0;
        int error = 0;
        int valid = 0;
        char *msg = NULL;

        engine = bul_engine_init();

        assert(yaml_parser_initialize(&parser));

        file = fopen(PROJECT_YAML, "rb");
        assert(file);

        yaml_parser_set_input_file(&parser, file);

        while(!done && !error) {
                if(!yaml_parser_parse(&parser, &event)) {
                        error = 1;
                        continue;
                }

                // Do stuff 
                yaml_print_event(&event);
                bul_engine_next_event(&engine, &event);

                // Check if we're done 
                done = (event.type == YAML_STREAM_END_EVENT);

                // Need to delete the event every time
                yaml_event_delete(&event);

                count++;
        }

        yaml_parser_delete(&parser);
        assert(!fclose(file));

        printf("#-------------------- Engine pre-resolution\n");
        bul_engine_print(&engine);

        bul_engine_resolve_ambiguity(&engine);

        printf("#-------------------- Engine post-resolution\n");
        bul_engine_print(&engine);

        msg = bul_engine_assert_valid(&engine);
        valid = (msg == NULL);

        bul_engine_free(&engine);

        printf("%s (%d events)\n", (error ? "FAILURE" : "SUCCESS"), count);
        printf("%s (bul_engine)\n", (valid ? "OK" : msg));
        return 0;
}
