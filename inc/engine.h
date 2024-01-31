/**
 * @file engine.h
 * @brief Inference engine for bulgogi projects.
 *
 */

#ifndef BUL_ENGINE_H
#define BUL_ENGINE_H

// Standard C Libraries 
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

// External Dependencies 
#include "yaml.h"

// Project headers 
#include "fs.h"

// Settings 
#define BUL_MAX_ID UINT_MAX
#define BUL_EXE_MK "*"
#define BUL_LIB_MK "lib"

/** Target ID Type */
typedef unsigned int bul_id_t;
/** Target name type */
typedef char* bul_name_t;

/** Target usage types */
typedef enum {
        /** Target is an executable */
        BUL_EXE,
        /** Target is a library */
        BUL_LIB,

} bul_usage_t;

typedef enum {
        BUL_HINT_EXE,
        BUL_HINT_LIB,
        BUL_HINT_NONE,
} bul_hint_t;

typedef enum {
        BUL_VALID,
        BUL_AMB,
        BUL_MISSING_EXE,
} bul_valid_t;

/**
 * struct bul_target - Defines a bulgogi target.
 *
 */
typedef struct bul_target {
        /** Unique ID for the target (internal representation). */
        bul_id_t id;
        /** Target name as seen in the configuration. */
        bul_name_t name;
        /** Inferred target usage (see `bul_usage_t`). */
        bul_usage_t usage;
        /** Number of targets/names tracked (size of project). */
        size_t size;
        /** List of dependencies by target ID. (deps[ID] = {IDs}). */
        bul_id_t *deps;

} bul_target_s;

/**
 * struct bul_engine - Stores inference engine state.
 *
 */
typedef struct bul_engine {
        /** Whether or not the parser is currently in a sequence. */
        bool in_seq;
        /** Number of targets in the project (also = number of names). */
        size_t size;
        /** ID of the current target in focus (init to BUL_MAX_ID). */
        bul_id_t focus;
        /** List of target names arranged by ID. (names[ID] = name). */
        bul_name_t *names;
        /** List of targets arranged by ID. (targets[ID] = target). */
        bul_target_s *targets;

} bul_engine_s;

/**
 * @brief Initializes an instance of the engine context.
 *
 * @return An initialized copy of an engine context.
 */
bul_engine_s bul_engine_init(void);

/**
 * @brief Processes the next YAML event from a config file.
 *
 * ASSUMPTIONS:
 * 1. YAML events are passed from start to finish.
 * 2. The `engine` context is initialized.
 * 3. The `event` is non-null.
 *
 * Failure to abide by this will result in undefined behaviour.
 *
 * @param[in] engine The engine context to use.
 * @param[in] event The next YAML event to process.
 */
void bul_engine_next_event(bul_engine_s *engine, yaml_event_t *event);

/**
 * @brief Processes a scalar YAML event.
 *
 * ASSUMPTIONS:
 * 1. YAML events are passed from start to finish.
 * 2. The `engine` context is initialized.
 * 3. The `event` is non-null.
 * 4. The `event` is of type scalar.
 *
 * @param[in] engine The engine context to use.
 * @param[in] event YAML scalar event to parse.
 */
void bul_engine_process_scalar(bul_engine_s *engine, yaml_event_t *event);

/**
 * @brief De-initializes the engine context.
 *
 * ASSUMPTIONS:
 * 1. The `engine` context is initialized.
 *
 * @param[in] engine The engine context to de-initialize.
 */
void bul_engine_free(bul_engine_s *engine);

/**
 * @brief Grow the engine's capacity by 1.
 *
 * @param[in] engine Engine context to use.
 */
void bul_engine_grow(bul_engine_s *engine);

/**
 * @brief Searches for a target by name.
 *
 * ASSUMPTIONS:
 * 1. The `engine` context is initialized.
 * 2. The target `name` is non-NULL.
 *
 * @param[in] engine Engine context to use.
 * @param[in] name Target name to search by.
 * @return Pointer to matching target or `NULL` if not found.
 */
bul_target_s *bul_engine_target_find(bul_engine_s *engine, bul_name_t name);

/**
 * @brief Adds a new target by name.
 *
 * ASSUMPTIONS:
 * 1. The `engine` context is initialized.
 * 2. The target `name` is non-NULL.
 *
 * NOTE:
 * This function creates its own dynamically allocated copy of
 * the passed `name`. It is therefore not necessary to preserve it 
 * after a successful call.
 *
 * @param[in] engine Engine context to use.
 * @param[in] name Name of target to add.
 * @return Pointer to newly added target or `NULL` if failed to add.
 */
bul_target_s *bul_engine_target_add(bul_engine_s *engine, bul_name_t name);

/**
 * @brief Updates existing target.
 *
 * ASSUMPTIONS:
 * 1. The `engine` context is initialized.
 * 2. The `target` is non-NULL.
 *
 * @param[in] engine Engine context to use.
 * @param[in] target Target to update.
 */
void bul_engine_target_update(bul_engine_s *engine, bul_target_s *target);

/**
 * @brief Adds dependency to the focused target by ID.
 *
 * ASSUMPTIONS:
 * 1. The `engine` context is initialized.
 * 2. The `dep_id` refers to an initialized target.
 *
 * @param[in] engine Engine context to use (tracks focus).
 * @param[in] dep_id ID of the dependency to add.
 */
void bul_engine_target_add_dep(bul_engine_s *engine, bul_id_t dep_id);

/**
 * @brief Grows the target in focus' dep list capacity by 1.
 *
 * @param[in] engine Engine context to use.
 * @return The new size of the target's dep list.
 */
size_t bul_engine_target_grow(bul_engine_s *engine);

/**
 * @brief Prints human-readable form of the struct.
 *
 * ASSUMPTIONS:
 * 1. Everything in a non-NULL engine is initialized.
 *
 * @param[in] engine Engine context to print.
 */
void bul_engine_print(bul_engine_s *engine);

/**
 * @brief Prints engine target by ID.
 *
 * ASSUMPTIONS:
 * 1. All of the parameters are non-NULL and initialized.
 *
 * @param[in] engine Engine context to use.
 * @param[in] id ID of the target to print.
 * @param[in] indent_level Level of indentation to use.
 */
void bul_engine_target_print(bul_engine_s *engine, bul_id_t id, int indent_level);

/**
 * @brief Prints the target usage in human-readable form.
 *
 * ASSUMPTIONS:
 * 1. The `target` is initialized.
 *
 * @param[in] target Pointer to the target whose usage is to be printed.
 */
void bul_target_usage_print(bul_target_s *target);

/**
 * @brief Returns the usage hint found in the name (if any).
 *
 * @param[in] name Name to evaluate.
 * @return The usage hint (if any) or `BUL_EXE` (default).
 */
bul_usage_t bul_detect_usage(bul_name_t name); 

bul_hint_t bul_detect_hint(bul_name_t name);

/**
 * @brief Validates whether engine rules are broken.
 *
 * @param[in] engine Engine context to use.
 * @return BUL_VALID in case valid, see `bul_valid_t` otherwise.
 */
bul_valid_t bul_engine_valid(bul_engine_s *engine);

/**
 * @brief Validates a targets by evaluating its dependencies.
 *
 * NOTE: A target with no dependencies is always considered valid.
 *
 * @param[in] engine Engine context to use.
 * @param[in] target Target to evaluate.
 * @return BUL_VALID in case valid, see `bul_valid_t` otherwise.
 */
bul_valid_t bul_engine_valid_target(bul_engine_s *engine, bul_target_s *target);

/**
 * @brief Counts a target's number of executable deps.
 *
 * @param[in] engine Engine context to use.
 * @param[in] target Target's exe deps to count.
 * @return Number of exe deps counted.
 */
size_t bul_engine_target_cnt_exe(bul_engine_s *engine, bul_target_s *target);

/**
 * @brief Prints an engine validation message.
 *
 * @param[in] engine Engine context to use.
 * @param[in] target Target to highlight in message.
 * @param[in] status Engine validation status to report.
 */
void bul_engine_print_invalid(bul_engine_s *engine, bul_target_s *target, bul_valid_t status);

/**
 * @brief Removes the hint from the target name.
 *
 * WARNING:
 * This function uses the `strdup()` method which requires the returned 
 * name to be `free()`'d.
 *
 * @param[in] name Target name to clean.
 * @return Target name without hints.
 */
bul_name_t bul_clean_name(bul_name_t name);

/**
 * @brief Adds the hint to the target name (opposite of `bul_clean_name`).
 *
 * WARNING:
 * This function uses the `strdup()` method which requires the returned 
 * name to be `free()`'d.
 *
 * @param[in] name Target name to add the hint to.
 * @param[in] usage Usage hint to add.
 * @return Target name with hint added.
 */
bul_name_t bul_hint_name(bul_name_t name, bul_usage_t usage);

/**
 * @brief Loads an engine context from file.
 *
 * @param[in] engine Engine context to load into.
 * @param[in] file_name YAML file to read.
 * @return `BUL_VALID` in case of valid project, see `bul_valid_t` otherwise.
 */
bul_fs_status_t bul_engine_from_file(bul_engine_s *engine, const char *file_name);

#endif // BUL_ENGINE_H
