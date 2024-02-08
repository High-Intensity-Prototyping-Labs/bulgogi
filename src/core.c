/**
 * @file core.c
 * @brief Bulgogi core library.
 */

#include "core.h"
#include <malloc/_malloc.h>
#include <string.h>

// Standard C Libraries 

bul_core_s bul_core_init(void) {
        bul_id_t *stack = NULL;
        bul_target_s *targets = NULL;

        stack = malloc(sizeof(bul_id_t));
        targets = malloc(sizeof(bul_core_s));

        bul_core_s core = {
                .size = 0,
                .level = 0,
                .maxlvl = 0,
                .stack = stack,
                .targets = targets,
        };

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

void bul_core_free(bul_core_s *core) {
        free(core->targets);
}

bul_target_s bul_target_init(bul_id_t id, char *name) {
        char *dup = NULL;
        bul_id_t *deps = NULL;

        dup = strdup(name);
        deps = malloc(sizeof(bul_id_t));

        bul_target_s target = {
                .id = id,
                .name = dup,
                .size = 0,
                .deps = deps,
        };

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
