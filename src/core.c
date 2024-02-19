/**
 * @file core.c
 * @brief Bulgogi core library.
 */

#include "core.h"

// Standard C Libraries 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bul_core_s bul_core_init(void) {
        bul_core_s core = {
                .map = 0,
                .size = 0,
                .level = 0,
                .maxlvl = 0,
                .stack = NULL,
                .targets = NULL,
        };

        core.stack = malloc(sizeof(bul_id_t));
        core.targets = malloc(sizeof(bul_core_s));

        return core;
}

void bul_core_next_event(bul_core_s *core, yaml_event_t *event) {
        switch(event->type) {
        case YAML_DOCUMENT_START_EVENT:
                bul_core_document_start(core);
                break;
        case YAML_DOCUMENT_END_EVENT:
                bul_core_document_end(core);
                break;
        case YAML_MAPPING_START_EVENT:
                bul_core_mapping_start(core);
                break;
        case YAML_MAPPING_END_EVENT:
                bul_core_mapping_end(core);
                break;
        case YAML_SCALAR_EVENT:
                bul_core_scalar(core, event);
                break;
        default:
                break;
        }
}

void bul_core_document_start(bul_core_s *core) {
        bul_id_t id = 0;

        // Add document as a target.
        id = bul_core_target_add(core, BUL_DOC_NAME);

        // Set scope and update level
        core->stack[core->level] = id;
        core->level++;
        bul_core_stack_grow_if(core);
}

void bul_core_document_end(bul_core_s *core) {
        core->level--;
}

void bul_core_mapping_start(bul_core_s *core) {
        core->map = 1;
        /* Defers level increase to next scalar event */
}

void bul_core_mapping_end(bul_core_s *core) {
        core->level--;
}

void bul_core_scalar(bul_core_s *core, yaml_event_t *event) {
        char *name = NULL;
        bul_id_t id = BUL_MAX_ID;
        bul_id_t parent_id = BUL_MAX_ID;
        bul_target_s *parent = NULL;
        bul_target_s *target = NULL;

        name = (char*)event->data.scalar.value;

        if((target = bul_core_target_find(core, name))) {
        /* Matching target found in scope */
                id = target->id;
        } else {
        /* No existing target in scope */
                id = bul_core_target_add(core, name);
        }

        if(core->level > 0) {
                parent_id = core->stack[core->level-1];
                /* Parent is the previous entry in the stack */
                parent = &core->targets[parent_id];
                bul_target_add_dep(parent, id);
        }

        if(core->map) {
                core->stack[core->level] = id;
                core->level++;
                bul_core_stack_grow_if(core);

                core->map = 0;
        }
}

void bul_core_stack_grow_if(bul_core_s *core) {
        if(core->level > core->maxlvl) {
                core->stack = realloc(core->stack, (core->level+1) * sizeof(bul_id_t));
                /* capacity is level+1 */
                core->maxlvl = core->level;
        }
}

void bul_core_grow(bul_core_s *core) {
        core->size++;
        core->targets = realloc(core->targets, (core->size+1) * sizeof(bul_target_s));
        /* capacity is size+1 */
}

bul_id_t bul_core_target_add(bul_core_s *core, char *name) {
        bul_id_t     id = 0;
        bul_target_s target;

        id     = core->size;
        target = bul_target_init(id, name);

        bul_core_grow(core);

        core->targets[id] = target;

        return id;
}

bul_target_s *bul_core_target_find(bul_core_s *core, char *name) {
        bul_target_s *match = NULL;

        /* Global Search */
        if(core->level == 0) {
                size_t x = 0;

                for(x = 0; x < core->size; x++) {
                        if(strcmp(core->targets[x].name, name) == 0) {
                                match = &core->targets[x];
                                break;
                        }
                }
        /* Scope-bound Search */
        } else {
                size_t x = 0;

                bul_target_s *dep = NULL;
                bul_target_s *scope = NULL;
                bul_id_t dep_id = BUL_MAX_ID;
                bul_id_t scope_id = BUL_MAX_ID;

                scope_id = core->stack[0];
                scope = &core->targets[scope_id];
                /* AKA parent */

                for(x = 0; x < scope->size; x++) {
                        dep_id = scope->deps[x];
                        dep = &core->targets[dep_id];

                        if(strcmp(dep->name, name) == 0) {
                                match = dep;
                                break;
                        }
                }
        }

        return match;
}

void bul_core_free(bul_core_s *core) {
        free(core->stack);
        for(; core->size != 0; core->size--) {
                free(core->targets[core->size-1].name);
                free(core->targets[core->size-1].deps);
        }
        free(core->targets);
}

bul_target_s bul_target_init(bul_id_t id, char *name) {
        bul_target_s target = {
                .id = BUL_MAX_ID,
                .name = NULL,
                .size = 0,
                .deps = NULL,
        };

        target.id = id;
        target.name = strdup(name);
        target.deps = malloc(sizeof(bul_id_t));

        return target;
}

void bul_target_add_dep(bul_target_s *target, bul_id_t dep_id) {
        bul_id_t dep_num = 0;

        dep_num = target->size;

        bul_target_grow(target);
        
        target->deps[dep_num] = dep_id;
}

void bul_target_grow(bul_target_s *target) {
        target->size++;
        target->deps = realloc(target->deps, (target->size+1) * sizeof(bul_id_t));
        /* target capacity is size+1 */
}

void bul_core_print(bul_core_s *core) {
        size_t x = 0;
        bul_id_t id = 0;
        char *name = NULL;
        
        printf("bul_core_s {\n");
        printf("\t.map = %d\n", core->map);
        printf("\t.size = %lu\n", core->size);
        printf("\t.level = %lu\n", core->level);
        printf("\t.maxlvl = %lu\n", core->maxlvl);
        printf("\t.stack = {\n");
        /* print stack */
        for(x = 0; x <= core->maxlvl; x++) {
                printf("\t\t");
                if(x == core->level) {
                        printf("(*level) => ");
                }
                if(x == core->maxlvl) {
                        name = "NULL";
                } else {
                        id = core->stack[x];
                        name = core->targets[id].name;
                }
                printf("core->stack[%lu] => %s,\n", x, name);
        }
        printf("\t},\n");
        printf("\t.targets = {\n");
        /* print targets */
        for(x = 0; x < core->size; x++) {
                bul_core_print_target(core, x, 2);
        }
        printf("\t}\n");
        printf("}\n");
}

static void indent(size_t level) {
        for(; level != 0; level--) {
                printf("\t");
        }
}

void bul_core_print_target(bul_core_s *core, bul_id_t target_id, size_t indent_level) {
        bul_target_s *target = NULL;

        target = &core->targets[target_id];

        indent(indent_level); printf("bul_target_s {\n");
        indent(indent_level); printf("\t.id = %u\n", target->id);
        indent(indent_level); printf("\t.size = %lu\n", target->size);
        indent(indent_level); printf("\t.name = %s\n", target->name);
        indent(indent_level); printf("\t.deps = {\n");
        /* print deps */
        {
                size_t x = 0;
                bul_id_t dep_id = 0;
                bul_target_s *dep = NULL;

                for(x = 0; x < target->size; x++) {
                        dep_id = target->deps[x];
                        dep = &core->targets[dep_id];

                        indent(indent_level); printf("\t\tcore->targets[%u] => %s,\n", dep_id, dep->name);
                }
        }
        indent(indent_level); printf("\t}\n");
        indent(indent_level); printf("}\n");
}

void bul_core_from_file(bul_core_s *core, FILE *file) {
        yaml_parser_t parser;
        yaml_event_t  event;

        int done = 0;
        int error = 0;
 
        yaml_parser_initialize(&parser);
        yaml_parser_set_input_file(&parser, file);

        while(!done && !error) {
                if(!yaml_parser_parse(&parser, &event)) {
                        error = 1;
                        continue;
                }

                bul_core_next_event(core, &event);

                done = (event.type == YAML_STREAM_END_EVENT);
                yaml_event_delete(&event);
        }

        yaml_parser_delete(&parser);
}
