/**
 * @file bul_dot_dul.h
 * @brief Manages the .bul directory in the project
 */

#include "bul_dot_bul.h"

// Standard C Libraries
#include <stdio.h>

// Project headers 
#include "bul_fs.h"
#include "engine.h"

void bul_dot_init(void) {
        bul_fs_status_t stat;
        if((stat = bul_fs_mkdir(DOT_BUL)) != BUL_FS_OK) {
                perror("Failed to create bulgogi directory");
                return;
        }
}

void bul_dot_add_target(bul_name_t name) {
        bul_fs_path_t path = NULL;
        bul_fs_status_t res = BUL_FS_OK;
        
        path = bul_fs_join(DOT_BUL, name);

        res = bul_fs_touch(path);
        if(res != BUL_FS_OK) {
                perror("Failed to create target file in bulgogi directory");
                goto cleanup;
        }

cleanup:
        free(path);
}
