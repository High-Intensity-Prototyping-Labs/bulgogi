/**
 * @file core.c
 * @brief Bulgogi core library.
 */

#include "core.h"
#include <malloc/_malloc.h>
#include <string.h>

// Standard C Libraries 

bul_core_s bul_core_init(void) {
        bul_core_s core = {
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

        // Set scope
        core->stack[core->level] = id;
        /* Waiting for MAPPING to update level */
}

void bul_core_document_end(bul_core_s *core) {
        core->level--;
}

void bul_core_mapping_start(bul_core_s *core) {
        core->level++;
        bul_core_stack_grow_if(core);
}

void bul_core_mapping_end(bul_core_s *core) {
        core->level--;
}

void bul_core_scalar(bul_core_s *core, yaml_event_t *event) {
        char *name = NULL;
        bul_id_t id = BUL_MAX_ID;
        bul_id_t parent_id = BUL_MAX_ID;
        bul_target_s *parent = NULL;

        name = (char*)event->data.scalar.value;

        id = bul_core_target_add(core, name);

        core->stack[core->level] = id;

        if(core->level > 0) {
                parent_id = core->stack[core->level-1];
                /* Parent is the previous entry in the stack */
                parent = &core->targets[parent_id];
                bul_target_add_dep(parent, id);
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

void bul_core_free(bul_core_s *core) {
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
