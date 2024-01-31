/**
 * @file bul_dot_dul.h
 * @brief Manages the .bul directory in the project
 */

#include "bul_dot_bul.h"

// Standard C Libraries
#include <stdio.h>

// Project headers 
#include "bul_fs.h"

void bul_dot_init() {
        bul_fs_status_t stat;
        if((stat = bul_fs_mkdir(DOT_BUL)) != BUL_FS_OK) {
                perror("Failed to create bulgogi directory");
                return;
        }
}
