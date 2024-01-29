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

// Settings 
#define BUL_MAX_ID UINT_MAX
#define BUL_EXE_MK "*"
#define BUL_LIB_MK "lib"

typedef unsigned int bul_id_t;
typedef char* bul_name_t;

typedef enum {
        BUL_EXE,
        BUL_LIB,

} bul_usage_t;

typedef enum {
        BUL_VALID,
        BUL_AMB,
        BUL_MISSING_EXE,
} bul_valid_t;

/**
 * struct bul_target - Defines a bulgogi target.
 *
 * @id - Unique ID for the target (internal representation).
 * @name - Target name as seen in the configuration.
 * @usage - Inferred target usage (see `bul_usage_t`).
 */
typedef struct bul_target {
        bul_id_t id;
        bul_name_t name;
        bul_usage_t usage;
        size_t size;
        bul_id_t *deps;

} bul_target_s;

/**
 * struct bul_engine - Stores inference engine state.
 *
 * @in_seq - Whether or not the parser is currently in a sequence.
 * @size - Number of targets in the project (also = number of names).
 * @focus - ID of the current target in focus (init to BUL_MAX_ID).
 * @names - List of target names arranged by ID. (names[ID] = name).
 * @targets - List of targets arranged by ID. (targets[ID] = target).
 * @deps - List of dependencies by target ID. (deps[ID] = {IDs}).
 */
typedef struct bul_engine {
        bool in_seq;
        size_t size;
        bul_id_t focus;
        bul_name_t *names;
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
 * @return The usage hint (if any) or BUL_EXE (default).
 */
bul_usage_t bul_detect_usage(bul_name_t name); 

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

#endif // BUL_ENGINE_H
