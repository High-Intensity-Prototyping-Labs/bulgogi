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
                .deps = malloc(sizeof(bul_id_t*)),
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
                free(engine->deps[x]);
        }
        free(engine->names);
        free(engine->deps);
}

void bul_engine_grow(bul_engine_s *engine) {
        engine->size++;
        engine->names = realloc(engine->names, (engine->size + 1) * sizeof(bul_name_t));
        engine->targets = realloc(engine->targets, (engine->size + 1) * sizeof(bul_target_s));
        engine->deps = realloc(engine->deps, (engine->size + 1) * sizeof(bul_id_t*));
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
                .usage = BUL_EXE,
                .size = 0,
        };

        engine->targets[id] = target;
        engine->deps[id] = malloc(sizeof(bul_id_t));

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
        bul_id_t *dep_list = NULL;

        target = &engine->targets[engine->focus];
        size = target->size;
        dep_list = engine->deps[engine->focus];

        bul_engine_target_grow(engine);
        dep_list[size] = dep_id;
}

size_t bul_engine_target_grow(bul_engine_s *engine) {
        bul_target_s *target = NULL;
        bul_id_t *dep_list = NULL;

        target = &engine->targets[engine->focus];
        dep_list = engine->deps[target->id];

        target->size++;
        dep_list = realloc(dep_list, (target->size + 1) * sizeof(bul_id_t));

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
                                bul_engine_target_print(engine, engine->deps[x][y], 3);
                                printf(",");
                        }
                        printf("\t\t},\n");
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
        indent(indent_level); printf("\t.usage = "); bul_engine_target_usage_print(target); printf(",\n");
        indent(indent_level); printf("\t.size = %lu,\n", target->size);
        indent(indent_level); printf("}\n");
}

void bul_engine_target_usage_print(bul_target_s *target) {
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
