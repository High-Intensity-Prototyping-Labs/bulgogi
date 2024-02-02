/**
 * @file bul_fs.h
 * @brief Handles the FS operations for bulgogi.
 */

#include "fs.h"

// Standard C Libraries
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <glob.h>
#include <errno.h>

bul_fs_pattern_s bul_fs_pattern_table[] = {
        {"*", 1, BUL_PAT_WILD},
        {"*.", 2, BUL_PAT_WILD_EXT},
        {"**", 2, BUL_PAT_WILD_RECURSE},
        {"**.", 3, BUL_PAT_WILD_RECURSE_EXT},
        {NULL, 0, 0},
};

bul_fs_status_t bul_fs_mkdir(bul_fs_path_t path) {
        int res = 0;

        res = mkdir(path, DEFAULT_FS_MODE);

        if(res == -1) {
                switch(errno) {
                case EEXIST:
                        return BUL_FS_WARN;
                default:
                        return BUL_FS_ERR;
                }
        } else {
                return BUL_FS_OK;
        }
}

bul_fs_path_t bul_fs_join(bul_fs_path_t a, bul_fs_path_t b) {
        size_t alen = 0;
        size_t blen = 0;
        size_t slen = 0;
        bul_fs_path_t joint = NULL;

        alen = strlen(a);
        blen = strlen(b);
        slen = strlen(DEFAULT_FS_SEP);

        joint = malloc(alen + blen + slen + 1);
        sprintf(joint, "%s%s%s", a, DEFAULT_FS_SEP, b);

        return joint;
}

bul_fs_status_t bul_fs_touch(bul_fs_path_t file) {
        FILE *fp = NULL;

        fp = fopen(file, "a");  // use "a" to create if not exists.
                                // avoids truncating with "w".
        if(!fp) {
                return BUL_FS_ERR;
        }

        fclose(fp);

        return BUL_FS_OK;
}

bul_fs_pattern_t bul_fs_detect_pattern(bul_fs_path_t path) {
        size_t path_len = 0;
        bul_fs_pattern_s *pattern = NULL;
        bul_fs_pattern_t res = BUL_PAT_NONE;

        path_len = strlen(path);

        pattern = &bul_fs_pattern_table[0];

        while(pattern->len != 0) {
                res = bul_fs_detect_pattern_of(path, path_len, pattern);
                /* Trying every pattern because `**` could overrule `**.` */

                pattern++;
        /* pattern->len == 0 || res != BUL_PAT_NONE */
        }

        return res;
}

bul_fs_pattern_t bul_fs_detect_pattern_of(bul_fs_path_t path, size_t path_len, bul_fs_pattern_s *pattern) {
        bul_fs_pattern_t res = BUL_PAT_NONE;

        for(size_t x = 0; (x+pattern->len-1) < path_len; x++) {
                if(strncmp(&path[x], pattern->sym, pattern->len) == 0) {
                        res = pattern->pat;
                        break;
                }
        }

        return res;
}

bul_fs_path_t bul_fs_get_pattern_ext(bul_fs_path_t path) {
        size_t path_len = 0;
        size_t ext_index = 0;

        path_len = strlen(path);
        ext_index = bul_fs_get_pattern_ext_index(path, path_len);

        if(ext_index == 0) {
                return &path[path_len];
        } else {
                return &path[ext_index+1];
        }
}

size_t bul_fs_get_pattern_ext_index(bul_fs_path_t path, size_t path_len) {
        for(size_t x = path_len-1; 0 <= x; x--) {
                if(path[x] == '.') {
                        return x;
                }
        }

        // Could not find ext
        return 0;
}

bul_fs_path_t *bul_fs_search_files(bul_fs_path_t path, bul_fs_pattern_t pattern) {
        bul_fs_path_t *files = NULL;

        glob_t globbuf;

        switch(pattern) {
        case BUL_PAT_WILD_RECURSE_EXT:
                printf("The BUL_PAT_WILD_RECURSE_EXT pattern type has not been implemented yet.\n");
                printf("Called from bul_fs_search_files() function.\n");
                /* This type ought to successively call glob until directories exhausted */
                /* The main caveat is that the directory structure needs to be preserved as they go deeper */
                return NULL;
        default:
                glob(path, GLOB_NOSORT | GLOB_TILDE, NULL, &globbuf);
                break;

        }

        files = malloc((globbuf.gl_pathc+1) * sizeof(bul_fs_path_t));
        for(size_t x = 0; x < globbuf.gl_pathc; x++) {
                files[x] = strdup(globbuf.gl_pathv[x]);
        }
        files[globbuf.gl_pathc] = NULL;

        globfree(&globbuf);

        return files;
}

void bul_fs_free_files(bul_fs_path_t *files) {
        bul_fs_path_t *file_p = NULL;

        file_p = &files[0];
        while(*file_p != NULL) {
                free(*file_p);
                file_p++;
        }
        
        free(files);
}

size_t bul_fs_path_get_parent_len(bul_fs_path_t path, size_t path_len) {
        size_t len = 0;

        for(size_t x = path_len-1; 0 <= x; x--) {
                if(path[x] == DEFAULT_FS_SEP_CHAR) {
                        len = x+1;
                        break;
                }
        }

        /** '/' = 1 
         * '/..' = 1 
         * '../' = path_len 
         * not found = 0 */

        return len;
}

size_t bul_fs_path_get_child_index(bul_fs_path_t path, size_t path_len) {
        return bul_fs_path_get_parent_len(path, path_len);
}
