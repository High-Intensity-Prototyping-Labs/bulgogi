/**
 * @file bul_fs.h
 * @brief Handles the FS operations for bulgogi.
 */

#ifndef BUL_FS_H
#define BUL_FS_H

// Settings 
#define DEFAULT_FS_MODE 0777
#define DEFAULT_FS_SEP "/"

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

/**
 * @brief Joins two paths together.
 *
 * WARNING:
 * This function returns a `malloc()`'d string which 
 * must be freed when discarded.
 *
 * ASSUMPTIONS:
 * 1. There are no leading or trailing path separators.
 * 2. Both paths are null-terminated.
 *
 * @param[in] a Parent path (left of separator).
 * @param[in] b Child path (right of separator).
 * @return Joint paths.
 */
bul_fs_path_t bul_fs_join(bul_fs_path_t a, bul_fs_path_t b);

/**
 * @brief Touches a file (see `touch` in GNU coreutils).
 *
 * NOTE:
 * Without `O_TRUNC` it was not possible to update file timestamp
 * using `open` or `fopen`. It was preferrable to cut the timestamp 
 * update feature of `touch` in favor of simplicity.
 *
 * Therefore: this is different from the GNU `touch` in that an 
 * existing file will not have its timestamp updated.
 *
 * @param[in] file Path to the file to touch.
 * @return `BUL_FS_OK` case success, see `bul_fs_status_t` otherwise.
 */
bul_fs_status_t bul_fs_touch(bul_fs_path_t file);

#endif // BUL_FS_H
