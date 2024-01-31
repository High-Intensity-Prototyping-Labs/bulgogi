/**
 * @file bul_dot_dul.h
 * @brief Manages the .bul directory in the project
 */

#include "dot_bul.h"

// Standard C Libraries
#include <limits.h>
#include <stdio.h>

// Project headers 
#include "fs.h"
#include "engine.h"

// Global engine state 
bul_engine_s engine;

void bul_dot_init(void) {
        bul_fs_status_t stat;
        if((stat = bul_fs_mkdir(DOT_BUL)) != BUL_FS_OK) {
                perror("Failed to create bulgogi directory");
                return;
        }

        engine = bul_engine_init();
}

bul_id_t bul_dot_add_target(bul_name_t name, bul_usage_t usage) {
        bul_fs_path_t path = NULL;
        bul_fs_status_t res = BUL_FS_OK;
        bul_name_t hint_name = NULL;
        bul_target_s *target = NULL;
        bul_id_t id = UINT_MAX;

        path = bul_fs_join(DOT_BUL, name);

        res = bul_fs_touch(path);
        if(res != BUL_FS_OK) {
                perror("Failed to create target file in bulgogi directory");
                goto cleanup;
        }
        
        hint_name = bul_hint_name(name, usage);

        target = bul_engine_target_add(&engine, hint_name);
        id = target->id;

        free(hint_name);

cleanup:
        free(path);

        return id;
}
