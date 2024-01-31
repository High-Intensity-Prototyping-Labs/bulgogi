/**
 * @file bul_fs.h
 * @brief Handles the FS operations for bulgogi.
 */

#include "bul_fs.h"

// Standard C Libraries
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>

bul_fs_status_t bul_fs_mkdir(bul_fs_path_t path) {
        int res = 0;

        res = mkdir(path, DEFAULT_FS_MODE);

        if(res == -1) {
                return BUL_FS_ERR;
        } else {
                return BUL_FS_OK;
        }
}
