/**
 * @file bul_fs.h
 * @brief Handles the FS operations for bulgogi.
 */

#ifndef BUL_FS_H
#define BUL_FS_H

// Standard C Libraries 
#include <stddef.h>

// Settings 
#define DEFAULT_FS_MODE 0777
#define DEFAULT_FS_SEP "/"
#define DEFAULT_FS_SEP_CHAR '/'

/** Generic FS path type */
typedef char* bul_fs_path_t;

/** Type for filesystem patterns */
typedef enum {
        /** Normal path (no pattern) */
        BUL_PAT_NONE,
        /** `*` - Wildcard */
        BUL_PAT_WILD,
        /** `*.c` - Wildcard with file extension */
        BUL_PAT_WILD_EXT,
        /** `**` - Recursive wildcard */
        BUL_PAT_WILD_RECURSE,
        /** `**.h` - Recursive wildcard with file extension */
        BUL_PAT_WILD_RECURSE_EXT,
} bul_fs_pattern_t;

/** Pattern struct */
typedef struct bul_fs_pattern {
        char    *sym;
        size_t  len;
        bul_fs_pattern_t pat;
} bul_fs_pattern_s;

/** Table defining pattern symbols and lengths */
extern bul_fs_pattern_s bul_fs_pattern_table[];

typedef enum {
        /** Filesystem operation OK */
        BUL_FS_OK,
        /** Filesystem error, errno available */
        BUL_FS_ERR,
        /** Filesystem warning, errno available (typically non-fatal) */
        BUL_FS_WARN,
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

/**
 * @brief Detects a pattern in a file path.
 *
 * ASSUMPTIONS:
 * 1. The path is null-terminated.
 * 2. All patterns in the `bul_fs_pattern_table` have lengths of at least 1.
 * 3. The `bul_fs_pattern_table` terminates with an entry of length 0.
 *
 * @param[in] path Path containing the pattern to detect.
 * @return Detect pattern as defined in `bul_fs_pattern_t`.
 */
bul_fs_pattern_t bul_fs_detect_pattern(bul_fs_path_t path);

/**
 * @brief Detects a specific pattern in a path from an entry of the `bul_fs_pattern_table`.
 *
 * @param[in] path Path to evaluate.
 * @param[in] path_len Length of the path to evaluate.
 * @param[in] pattern Pointer to entry in the `bul_fs_pattern_table` to use.
 * @return `BUL_PAT_NONE` or the pattern in `pattern` if detected.
 */
bul_fs_pattern_t bul_fs_detect_pattern_of(bul_fs_path_t path, size_t path_len, bul_fs_pattern_s *pattern);

/**
 * @brief Returns a reference to the portion of the path which contains the file extension.
 *
 * ASSUMPTIONS:
 * 1. The path is null-terminated.
 * 2. The path contains a file extension.
 *      a. This means the detect pattern must correspond to any `_EXT` type.
 *
 * NOTE:
 * The path returned is actually a reference to a location in the provided 
 * path. Therefore they are 'quantum-linked' together.
 *
 * @param[in] path Path containing file extension.
 * @return Path which acts as the file extension. Failure to find extension results in 0-length NULL-terminated path.
 */
bul_fs_path_t bul_fs_get_pattern_ext(bul_fs_path_t path);

/**
 * @brief Returns the index of the '.' of the path's file extension.
 *
 * @param[in] path Path containing a file extension.
 * @param[in] path_len Path length.
 * @return Index of the file extension '.' or `0` if none found.
 */
size_t bul_fs_get_pattern_ext_index(bul_fs_path_t path, size_t path_len);

/**
 * @brief Returns a list of matching file paths based on pattern.
 *
 * WARNING:
 * This function dynamically allocates memory when a non-NULL value is 
 * returned. Be sure to `free()` the memory when it is no longer used.
 *
 * ASSUMPTIONS:
 * 1. The `pattern` type is anything but `BUL_PAT_NONE`.
 *
 * @param[in] path Path containing pattern to search.
 * @param[in] pattern Pattern type (see `bul_fs_detect_pattern()`).
 * @return `NULL` if no matching files found or a NULL-terminated list of file paths.
 */
bul_fs_path_t *bul_fs_search_files(bul_fs_path_t path, bul_fs_pattern_t pattern);

/**
 * @brief Frees a list of file paths returned by `bul_fs_search_files`.
 *
 * ASSUMPTIONS:
 * 1. The `files` pointer has been initialized.
 * 2. The list of `files` are null-terminated.
 *
 * @param[in] files List of file paths to free.
 */
void bul_fs_free_files(bul_fs_path_t *files);

/**
 * @brief Returns the index of the parent path segment end.
 *
 * WARNING:
 * Paths ending in a path separator (`'/'`) will yield a parent length 
 * that includes the null-terminator. When working with this value, it 
 * cannot be assumed that the bounds of the string are strictly within. 
 *
 * ASSUMPTIONS:
 * 1. Path are separated according to `DEFAULT_FS_SEP`.
 *
 * The parent path segment is considered to be all of the directories leading 
 * up to the child - the last file or directory of the path.
 *
 * If the path refers to a single file or directory, then the index points 
 * to the null-terminator of the original path.
 *
 * @param[in] path Path to evaluate.
 * @param[in] path_len Length of `path`.
 * @return Index of the parent path segment end. Will return `path_len` if no parent found.
 */
size_t bul_fs_path_get_parent_len(bul_fs_path_t path, size_t path_len);

/**
 * @brief Returns the start index of the child segment of the path.
 *
 * See `bul_fs_path_get_parent_len` for details on parent vs child segment.
 *
 * @param[in] path Path to evaluate.
 * @param[in] path_len Length of `path`.
 * @return Index of the start position in `path` of the child segment.
 */
size_t bul_fs_path_get_child_index(bul_fs_path_t path, size_t path_len);

#endif // BUL_FS_H
