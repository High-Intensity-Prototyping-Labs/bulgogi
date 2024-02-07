/**
 * @file core.c
 * @brief Bulgogi core library.
 */

#include "core.h"
#include <malloc/_malloc.h>

// Standard C Libraries 

bul_core_s bul_core_init(void) {
        bul_target_s *targets = NULL;

        targets = malloc(sizeof(bul_core_s));

        bul_core_s core = {
                .size = 0,
                .level = 0,
                .targets = targets,
        };

        return core;
}

void bul_core_free(bul_core_s *core) {
        free(core->targets);
}
