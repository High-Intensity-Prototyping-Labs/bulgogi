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
        for(size_t x = 0; x < engine->size; x++) {
                if(strcmp(engine->names[x], name) == 0) {
                        return &engine->targets[x];
                }
        }

        // Target not found
        return NULL;
}

bul_target_s *bul_engine_target_add(bul_engine_s *engine, bul_name_t name) {
        bul_id_t id = 0;

        id = engine->size;
        bul_engine_grow(engine);
        engine->names[id] = malloc(strlen(name)+1);
        strcpy(engine->names[id], name);

        bul_target_s target = {
                .id = id,
                .name = engine->names[id],
                .usage = BUL_AMB,
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
        case BUL_AMB:
                printf("BUL_AMB");
                break;
        }
}

size_t bul_engine_count_exe_deps(bul_engine_s *engine, bul_target_s *target) {
        size_t          count   = 0;
        bul_id_t        dep_id  = 0;
        bul_target_s    *dep    = NULL;

        for(size_t x = 0; x < target->size; x++) {
                dep_id = target->deps[x];
                dep = &engine->targets[dep_id];

                if(dep->usage == BUL_EXE) {
                        count++;
                }
        }

        return count;
}

bul_usage_t bul_clean_name(bul_name_t name) {
        bul_usage_t hint = BUL_AMB;
        size_t name_len = 0;
        size_t lib_len = 0;
        size_t diff = 0;

        name_len = strlen(name);
        lib_len = strlen(BUL_LIB_MK);

        if(name[name_len-1] == BUL_EXE_MK) {
                hint = BUL_EXE;
                name[name_len-1] = '\0';
        } else if(name_len > lib_len) {
                hint = (strncmp(name, BUL_LIB_MK, lib_len) == 0) ? BUL_LIB : BUL_AMB;
                if(strncmp(name, BUL_LIB_MK, lib_len) == 0) {
                        hint = BUL_LIB;
                        diff = name_len - lib_len;
                        memmove(&name[0], &name[lib_len], diff);
                        name[diff] = '\0';
                }
        }

        return hint;
}

char *bul_engine_assert_valid(bul_engine_s *engine) {
        bul_target_s *target = NULL;

        for(size_t tid = 0; tid < engine->size; tid++) {
                target = &engine->targets[tid];

                // TOOD: Set appropriate condtions based on whether target is exe or lib.
                if(bul_engine_count_exe_deps(engine, target) != 0) {
                        return "Ambiguity detected. Consider adding (*) or (lib) markers.";
                }
        }

        return NULL;
}
