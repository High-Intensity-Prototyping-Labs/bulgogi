/**
 * @file bul_fs.h
 * @brief Handles the FS operations for bulgogi.
 */

#ifndef BUL_FS_H
#define BUL_FS_H

// Settings 
#define DEFAULT_FS_MODE 0777

typedef char* bul_fs_path_t;

typedef enum {
        /** Filesystem operation OK */
        BUL_FS_OK,
        /** Filesystem error, errno available */
        BUL_FS_ERR,
        /** Filesystem error, no errno available */
        BUL_FS_NO_ERRNO,
        /** Filesystem error, unknown */
        BUL_FS_UNKNOWN,
} bul_fs_status_t;

/**
 * @brief Wrapper for standard `mkdir()`.
 *
 * @param[in] path Path to directory to create.
 * @return `BUL_FS_OK` in case success, see `bul_fs_status_t` otherwise.
 */
bul_fs_status_t bul_fs_mkdir(bul_fs_path_t path);

#endif // BUL_FS_H
