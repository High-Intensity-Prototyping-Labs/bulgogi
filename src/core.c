/**
 * @file core.c
 * @brief Bulgogi core library.
 */

#include "core.h"
#include <malloc/_malloc.h>
#include <string.h>

// Standard C Libraries 

bul_core_s bul_core_init(void) {
        bul_target_s *targets = NULL;

        targets = malloc(sizeof(bul_core_s));

        bul_core_s core = {
                .size = 0,
                .level = 0,
                .prevs = BUL_MAX_ID,
                .scope = BUL_MAX_ID,
                .targets = targets,
        };

        return core;
}

void bul_core_next_event(bul_core_s *core, yaml_event_t *event) {
        switch(event->type) {
        case YAML_DOCUMENT_START_EVENT:
                bul_core_document_start(core, event);
                break;
        case YAML_DOCUMENT_END_EVENT:
                bul_core_document_end(core, event);
                break;
        default:
                break;
        }
}

void bul_core_document_start(bul_core_s *core, yaml_event_t *event) {
        bul_id_t id = 0;

        // Add document as a target.
        id = bul_core_target_add(core, BUL_DOC_NAME);

        // Set scope and update level 
        core->prevs = core->scope;
        core->scope = id;
        core->level++;
}

void bul_core_document_end(bul_core_s *core, yaml_event_t *event) {
        core->scope = core->prevs;
        core->level--;
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

        if(core->level > 0) {
                // Add to scope
                bul_target_add_child(core->targets + core->scope, id);
        }

        return id;
}

void bul_core_free(bul_core_s *core) {
        free(core->targets);
}

bul_target_s bul_target_init(bul_id_t id, char *name) {
        char *name_dup = NULL;
        bul_id_t *children = NULL;

        name_dup = strdup(name);
        children = malloc(sizeof(bul_id_t));

        bul_target_s target = {
                .id = id,
                .name = name_dup,
                .size = 0,
                .children = NULL,
        };

        return target;
}

void bul_target_add_child(bul_target_s *target, bul_id_t child_id) {
        bul_id_t child_num = 0;

        child_num = target->size;

        bul_target_grow(target);
        
        target->children[child_num] = child_id;
}

void bul_target_grow(bul_target_s *target) {
        target->size++;
        target->children = realloc(target->children, (target->size+1) * sizeof(bul_id_t));
        /* target capacity is size+1 */
}
