/**
 * @file core.h
 * @brief Bulgogi core library header.
 */

#ifndef BUL_CORE_H
#define BUL_CORE_H

// Standard C Libraries
#include <stddef.h>

typedef unsigned int bul_id_t;

struct bul_target {
        bul_id_t id;
        size_t   size;
        char     *name;
        bul_id_t *children;
};
typedef struct bul_target bul_target_s;

struct bul_core {
        unsigned int level;
        bul_target_s *targets;
        size_t size;
};
typedef struct bul_core bul_core_s;

/**
 * General assumptions for these functions:
 * 1. Pointers are non-NULL.
 * 2. All structs are initialized.
 * 3. All C-library memory initializations work.
 */

/**
 * @brief Initializes a core struct.
 *
 * @return An initialized core struct.
 */
bul_core_s bul_core_init(void);

/**
 * @brief Frees an initialized core struct.
 *
 * @param[in] core Core struct to initialize.
 */
void bul_core_free(bul_core_s *core);

#endif // BUL_CORE_H
