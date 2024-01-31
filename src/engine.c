/**
 * @file engine.c
 * @brief Inference engine for bulgogi projects.
 *
 */

#include "engine.h"

// Standard C Libraries 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Project headers 
#include "fs.h"
#include "yaml.h"

// Note: Allocates enough room for size + 1 values
bul_engine_s bul_engine_init(void) {
        return (bul_engine_s) {
                .in_seq = false,
                .size = 0,
                .focus = BUL_MAX_ID,
                .names = malloc(sizeof(bul_name_t)),
                .targets = malloc(sizeof(bul_target_s)),
        };
}

void bul_engine_next_event(bul_engine_s *engine, yaml_event_t *event) {
        switch(event->type) {
        case YAML_SEQUENCE_START_EVENT:
                engine->in_seq = true;
                break;
        case YAML_SEQUENCE_END_EVENT:
                engine->in_seq = false;
                break;
        case YAML_SCALAR_EVENT:
                bul_engine_process_scalar(engine, event);
                break;
        default:
                break;
        }
}

void bul_engine_process_scalar(bul_engine_s *engine, yaml_event_t *event) {
        // Add new target (if doesn't exist)
        
        bul_name_t name = NULL;
        bul_target_s *target = NULL;

        name = (char*)event->data.scalar.value;

        if((target = bul_engine_target_find(engine, name))) {
                bul_engine_target_update(engine, target);
        } else {
                target = bul_engine_target_add(engine, name);
                assert(target);
        }

        if(engine->in_seq) {
                // Add target as dependency to focused target
                bul_engine_target_add_dep(engine, target->id);
        } else {
                // Focus on target
                engine->focus = target->id;
        }
}

void bul_engine_free(bul_engine_s *engine) {
        free(engine->targets);
        for(size_t x = 0; x < engine->size; x++) {
                free(engine->names[x]);
                free(engine->targets[x].deps);
        }
        free(engine->names);
}

void bul_engine_grow(bul_engine_s *engine) {
        engine->size++;
        engine->names = realloc(engine->names, (engine->size + 1) * sizeof(bul_name_t));
        engine->targets = realloc(engine->targets, (engine->size + 1) * sizeof(bul_target_s));
}

bul_target_s *bul_engine_target_find(bul_engine_s *engine, bul_name_t name) {
        bul_name_t clean_name = NULL;

        clean_name = bul_clean_name(name);

        for(size_t x = 0; x < engine->size; x++) {
                if(strcmp(engine->names[x], clean_name) == 0) {
                        return &engine->targets[x];
                }
        }

        free(clean_name);

        // Target not found
        return NULL;
}

bul_target_s *bul_engine_target_add(bul_engine_s *engine, bul_name_t name) {
        bul_id_t id = 0;
        bul_usage_t usage = BUL_EXE;

        id = engine->size;
        bul_engine_grow(engine);
        engine->names[id] = bul_clean_name(name);

        usage = bul_detect_usage(name);

        bul_target_s target = {
                .id = id,
                .name = engine->names[id],
                .usage = usage,
                .size = 0,
                .deps = malloc(sizeof(bul_id_t)),
        };

        engine->targets[id] = target;

        return &engine->targets[id];
}

void bul_engine_target_update(bul_engine_s *engine, bul_target_s *target) {
        (void)engine;
        target->usage = BUL_LIB;
}

void bul_engine_target_add_dep(bul_engine_s *engine, bul_id_t dep_id) {
        assert(engine->focus < BUL_MAX_ID);

        size_t size = 0;
        bul_target_s *target = NULL;
        bul_id_t **dep_list = NULL;

        target = &engine->targets[engine->focus];
        size = target->size;
        dep_list = &engine->targets[engine->focus].deps;

        bul_engine_target_grow(engine);
        (*dep_list)[size] = dep_id;
}

size_t bul_engine_target_grow(bul_engine_s *engine) {
        bul_target_s *target = NULL;
        bul_id_t **dep_list = NULL;

        target = &engine->targets[engine->focus];
        dep_list = &engine->targets[engine->focus].deps;

        target->size++;
        *dep_list = realloc(*dep_list, (target->size + 1) * sizeof(bul_id_t));
        
        return target->size;
}

void bul_engine_print(bul_engine_s *engine) {
        printf("bul_engine_s {\n");

        if(engine == NULL) {
                printf("\tNULL\n");
        } else {
                printf("\t.in_seq = %d,\n", engine->in_seq);
                printf("\t.size = %lu,\n", engine->size);
                printf("\t.focus = %u,\n", engine->focus);
                printf("\t.names = {");
                for(size_t x = 0; x < engine->size; x++) {
                        printf("\n\t\t%s,", engine->names[x]);
                }
                printf("\t},\n");
                printf("\t.targets = {");
                for(size_t x = 0; x < engine->size; x++) {
                        printf("\n");
                        bul_engine_target_print(engine, x, 2);
                        printf(",");
                }
                printf("\t},\n");
                printf("\t.deps = {");
                for(size_t x = 0; x < engine->size; x++) {
                        printf("\n\t\t%s = {", engine->names[x]);
                        for(size_t y = 0; y < engine->targets[x].size; y++) {
                                printf("\n");
                                bul_engine_target_print(engine, engine->targets[x].deps[y], 3);
                                printf(",");
                        }
                        printf("\n\t\t},\n");
                }
                printf("\t},\n");
        }

        printf("}\n");
}

static void indent(int lvl) { for(int x = 0; x < lvl; x++) { printf("\t"); } }

void bul_engine_target_print(bul_engine_s *engine, bul_id_t id, int indent_level) {
        bul_target_s *target = NULL;

        target = &engine->targets[id];

        indent(indent_level); printf("bul_target_s {\n");
        indent(indent_level); printf("\t.id = %u,\n", target->id);
        indent(indent_level); printf("\t.name = %s,\n", target->name);
        indent(indent_level); printf("\t.usage = "); bul_target_usage_print(target); printf(",\n");
        indent(indent_level); printf("\t.size = %lu,\n", target->size);
        indent(indent_level); printf("}");
}

void bul_target_usage_print(bul_target_s *target) {
        switch(target->usage) {
        case BUL_EXE:
                printf("BUL_EXE");
                break;
        case BUL_LIB:
                printf("BUL_LIB");
                break;
        }
}

bul_usage_t bul_detect_usage(bul_name_t name) {
        bul_usage_t usage = BUL_EXE;
        bul_hint_t hint = BUL_HINT_NONE;

        hint = bul_detect_hint(name);

        if(hint == BUL_HINT_LIB) {
                usage = BUL_LIB;
        }

        return usage;
}

bul_hint_t bul_detect_hint(bul_name_t name) {
        bul_hint_t hint = BUL_HINT_NONE;

        size_t name_len = 0;
        size_t exe_len = 0;
        size_t lib_len = 0;

        name_len = strlen(name);
        exe_len = strlen(BUL_EXE_MK);
        lib_len = strlen(BUL_LIB_MK);

        if(name_len > exe_len) {
                if(strncmp(&name[name_len-1], BUL_EXE_MK, exe_len) == 0) {
                        hint = BUL_HINT_EXE;
                }
        }

        if(name_len > lib_len) {
                if(strncmp(name, BUL_LIB_MK, strlen(BUL_LIB_MK)) == 0) {
                        hint = BUL_HINT_LIB;
                }
        }

        return hint;
}

bul_valid_t bul_engine_valid(bul_engine_s *engine) {
        bul_target_s *target = NULL;
        bul_valid_t valid = BUL_VALID;

        for(size_t x = 0; x < engine->size; x++) {
                target = &engine->targets[x];
                valid = bul_engine_valid_target(engine, target);
                if(valid != BUL_VALID) {
                        bul_engine_print_invalid(engine, target, valid);
                        break;
                }
        }

        return valid;
}

bul_valid_t bul_engine_valid_target(bul_engine_s *engine, bul_target_s *target) {
        size_t exe_cnt = 0;
        bul_valid_t valid = BUL_VALID;

        if(target->usage == BUL_EXE && target->size > 0) {
                exe_cnt = bul_engine_target_cnt_exe(engine, target);

                if(exe_cnt > 1) {
                        valid = BUL_AMB;
                } else if(exe_cnt < 1) {
                        valid = BUL_MISSING_EXE;
                } else {
                        valid = BUL_VALID;
                }
        }

        return valid;
}

size_t bul_engine_target_cnt_exe(bul_engine_s *engine, bul_target_s *target) {
        bul_target_s *dep = NULL;
        bul_id_t dep_id = 0;
        size_t cnt = 0;

        for(size_t x = 0; x < target->size; x++) {
                dep_id = target->deps[x];
                dep = &engine->targets[dep_id];

                if(dep->usage == BUL_EXE) {
                        cnt++;
                }
        }

        return cnt;
}

void bul_engine_print_invalid(bul_engine_s *engine, bul_target_s *target, bul_valid_t status) {
        (void)engine;
        switch(status) {
        case BUL_VALID:
                printf("Project configuration is valid.\n");
                break;
        case BUL_AMB:
                printf("Target (%s) is ambiguous. Consider adding dep hints (lib) or (*).\n", target->name);
                break;
        case BUL_MISSING_EXE:
                printf("Target (%s) is missing an executable component.\n", target->name);
                break;
        }
}

bul_name_t bul_clean_name(bul_name_t name) {
        size_t begin = 0;
        size_t end = 0;
        bul_hint_t hint = BUL_HINT_NONE;

        hint = bul_detect_hint(name);
        end = strlen(name);

        if(hint == BUL_HINT_EXE) {
                end -= strlen(BUL_EXE_MK);
        } else if(hint == BUL_HINT_LIB) {
                begin = strlen(BUL_LIB_MK);
        }

        return strndup(&name[begin], end);
}

bul_name_t bul_hint_name(bul_name_t name, bul_usage_t usage) {
        bul_name_t hint_name = NULL;
        size_t padding = 0;
        char *first = NULL;
        char *second = NULL;

        switch(usage) {
        case BUL_LIB:
                padding = strlen(BUL_LIB_MK);
                first = BUL_LIB_MK;
                second = name;
                break;
        case BUL_EXE:
                padding = strlen(BUL_EXE_MK);
                first = name;
                second = BUL_EXE_MK;
                break;
        }

        hint_name = malloc(strlen(name)+padding+1);
        sprintf(hint_name, "%s%s", first, second);

        return hint_name;
}

 bul_fs_status_t bul_engine_from_file(bul_engine_s *engine, const char *file_name) {
        yaml_parser_t   parser;
        yaml_event_t    event;
        FILE            *file;

        int done        = 0;
        int error       = 0;

        yaml_parser_initialize(&parser);

        file = fopen(file_name, "rb");
        if(!file) {
                return BUL_FS_ERR;
        }

        yaml_parser_set_input_file(&parser, file);

        while(!done && !error) {
                if(!yaml_parser_parse(&parser, &event)) {
                        error = 1;
                        continue;
                }

                bul_engine_next_event(engine, &event);

                done = (event.type == YAML_STREAM_END_EVENT);
                yaml_event_delete(&event);
        }

        yaml_parser_delete(&parser);
        fclose(file);

        return BUL_FS_OK;
}
