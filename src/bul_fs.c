/**
 * @file bul_fs.h
 * @brief Handles the FS operations for bulgogi.
 */

#include "bul_fs.h"

// Standard C Libraries
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

bul_fs_status_t bul_fs_mkdir(bul_fs_path_t path) {
        int res = 0;

        res = mkdir(path, DEFAULT_FS_MODE);

        if(res == -1) {
                return BUL_FS_ERR;
        } else {
                return BUL_FS_OK;
        }
}

bul_fs_path_t bul_fs_join(bul_fs_path_t a, bul_fs_path_t b) {
        size_t alen = 0;
        size_t blen = 0;
        bul_fs_path_t joint = NULL;

        alen = strlen(a);
        blen = strlen(b);

        joint = malloc(alen + blen + 1);
        sprintf(joint, "%s%s%s", a, DEFAULT_FS_SEP, b);

        return joint;
}
