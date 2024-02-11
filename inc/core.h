/**
 * @file core.h
 * @brief Bulgogi core library header.
 */

#ifndef BUL_CORE_H
#define BUL_CORE_H

// Standard C Libraries
#include <stddef.h>
#include <limits.h>

// External Dependencies 
#include "yaml.h"

#define BUL_DOC_NAME "DOCUMENT"
#define BUL_MAX_ID UINT_MAX

typedef unsigned int bul_id_t;

struct bul_target {
        /** Unique target ID */
        bul_id_t id;
        /** Number of children */
        size_t   size;
        /** Target name (unique in scope) */
        char     *name;
        /** Target deps */
        bul_id_t *deps;
};
typedef struct bul_target bul_target_s;

struct bul_core {
        /** FLags previous MAPPING event */
        int map;
        /** Number of targets stored */
        size_t size;
        /** Current recursion depth */
        size_t level;
        /** Largest `level` recorded */
        size_t maxlvl;
        /** Recursion stack for traversing levels */
        bul_id_t *stack;
        /** Array of targets stored by ID */
        bul_target_s *targets;
};
typedef struct bul_core bul_core_s;

/**
 * General assumptions for these functions:
 * 1. Pointers are non-NULL.
 * 2. All structs are initialized.
 * 3. Configuration files are parsed sequentially.
 */

/**
 * @brief Initializes a core struct.
 *
 * WARNINGS:
 * 1. This function uses `malloc()` 
 * 2. The struct must subsequently be freed using `bul_core_free()`.
 *
 * @return An initialized core struct.
 */
bul_core_s bul_core_init(void);

/**
 * @brief Processes the next YAML event.
 *
 * ASSUMPTIONS:
 * 1. YAML events are parsed sequentially from start to finish.
 *
 * @param[in] core Core context to use.
 * @param[in] event Next YAML event to parse.
 */
void bul_core_next_event(bul_core_s *core, yaml_event_t *event);

/**
 * @brief Processes a YAML event of type `YAML_DOCUMENT_START_EVENT`.
 *
 * @param[in] core Core context to use.
 */
void bul_core_document_start(bul_core_s *core);

/**
 * @brief Processes a YAML event of type `YAML_DOCUMENT_END_EVENT`.
 *
 * @param[in] core Core context to use.
 */
void bul_core_document_end(bul_core_s *core);

/**
 * @brief Processes a YAML event of type `YAML_MAPPING_START_EVENT`.
 *
 * @param[in] core Core context to use.
 */
void bul_core_mapping_start(bul_core_s *core);

/**
 * @brief Processes a YAML event of type `YAML_MAPPING_END_EVENT`.
 *
 * @param[in] core Core context to use.
 */
void bul_core_mapping_end(bul_core_s *core);

/**
 * @brief Processes a YAML event of type `YAML_SCALAR_EVENT`.
 *
 * @param[in] core Core context to use.
 * @param[in] event Event of type `YAML_SCALAR_EVENT`.
 */
void bul_core_scalar(bul_core_s *core, yaml_event_t *event);

/**
 * @brief Grows the stack if needed.
 *
 * NOTE: This does not affect the current `level` value, mainly for semantic purposes.
 *
 * The decision to grow the stack is based on the `maxlvl` value.
 * If the `level` value is greater than `maxlvl`, the stack will be `realloc`'d to fit.
 *
 * @param[in] core Core context to use.
 */
void bul_core_stack_grow_if(bul_core_s *core);

/**
 * @brief Increases the size of the core's capacity by 1.
 *
 * @param[in] core Core context to grow.
 */
void bul_core_grow(bul_core_s *core);

/**
 * @brief Adds a target to the core.
 *
 * ASSUMPTIONS:
 * 1. An existing (duplicate) target does not already exist.
 *
 * @param[in] core Core context to use.
 * @param[in] name Name of the target to add.
 * @return bul_id_t ID of the newly added target.
 */
bul_id_t bul_core_target_add(bul_core_s *core, char *name);

/**
 * @brief Searches for target with matching name within the current scope.
 *
 * The current scope is defined by the target ID situated at `core->level - 1` in `core->stack`.
 * Dependencies of the target scope are scanned non-recursively for a match.
 * If the `core->level` is 0, the scope is global meaning any `core->target[N]` can match.
 *
 * @param[in] core Core context to use.
 * @param[in] name Name of the target to find.
 * @return Pointer to the first match or `NULL` in case of no match.
 */
bul_target_s *bul_core_target_find(bul_core_s *core, char *name);

/**
 * @brief Frees an initialized core struct.
 *
 * @param[in] core Core struct to initialize.
 */
void bul_core_free(bul_core_s *core);

/**
 * @brief Initializes a target struct.
 *
 * WARNINGS:
 * 1. This target uses `strdup()` which allocates dynamic memory.
 *
 * @param[in] id ID of the target to initialize.
 * @param[in] name Names of the target to initialize.
 * @return The initialized target.
 */
bul_target_s bul_target_init(bul_id_t id, char *name);

/**
 * @brief Adds a child target to `target`.
 *
 * @param[in] core Core context to use.
 * @param[in] target_id ID of the target to add the dep to.
 * @param[in] dep_id ID of the dep to add.
 */
void bul_target_add_dep(bul_target_s *target, bul_id_t dep_id);

/**
 * @brief Grows the child capacity of the target by 1.
 *
 * @param[in] target Target to grow.
 */
void bul_target_grow(bul_target_s *target);

/**
 * @brief Prints the core in human-readable format to `stdout`.
 *
 * @param[in] core Core context to print.
 */
void bul_core_print(bul_core_s *core);

/**
 * @brief Prints the target specified by ID in human-readable format to `stdout`.
 *
 * @param[in] core Core context to use.
 * @param[in] target_id ID of the target to print.
 * @param[in] indent_level Level of indentation to use (baseline).
 */
void bul_core_print_target(bul_core_s *core, bul_id_t target_id, size_t indent_level);

/**
 * @brief Loads a YAML file into an initialized core.
 *
 * ASSUMPTIONS:
 * 1. The `core` was initialized using `bul_core_init()`.
 *
 * @param[in] file Open filestream to read from (`"rb"` mode).
 * @return Loaded and initialized core from YAML file.
 */
void bul_core_from_file(bul_core_s *core, FILE *file);

#endif // BUL_CORE_H
