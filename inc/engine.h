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
#define BUL_EXE_MK '*'
#define BUL_LIB_MK "lib"

typedef unsigned int bul_id_t;
typedef char* bul_name_t;

typedef enum {
        BUL_EXE,
        BUL_LIB,
        BUL_AMB,

} bul_usage_t;

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
 * @brief Resolves target ambiguity based on inferencing results.
 *
 * @param[in] engine Engine context to use.
 */
void bul_engine_resolve_ambiguity(bul_engine_s *engine);

/**
 * @brief Returns the target's number of ambiguous deps.
 *
 * ASSUMPTIONS:
 * 1. Engine has been initialized.
 * 2. A project configuration has had inference run.
 * 3. A valid tid to an initialized target was provided.
 *
 * @param[in] engine Engine context to use.
 * @param[in] tid ID of the target to check.
 * @return Number of dependencies with usage BUL_AMB.
 */
size_t bul_engine_count_target_ambiguity(bul_engine_s *engine, bul_target_s *target);

/**
 * @brief Cleans name of special executable or library markers.
 *
 * @param[in,out] name Name to clean.
 * @return Detected usage marker in name if present, BUL_AMB otherwise.
 */
bul_usage_t bul_clean_name(bul_name_t name);

/**
 * @brief Applies known target usage to its dependencies.
 *
 * ASSUMPTIONS:
 * 1. An inference on an initialized engine has been run.
 * 2. Target points to an initialized target,
 *
 * @param[in] engine Engine context to use.
 * @param[in] target Point to target with known usage to apply to its deps.
 */
void bul_engine_apply_usage_deps(bul_engine_s *engine, bul_target_s *target);

/**
 * @brief Applies a usage to the first ambiguous dep in a target.
 *
 * ASSUMPTIONS:
 * 1. Deps have been narrowed down to a single unique ambiguous one.
 * 2. The desired usage to apply to the remining ambiguous one is known.
 *
 * @param[in] engine Engine context to use.
 * @param[in] target Target with deps to apply.
 * @param[in] usage Desired usage to apply to dep.
 */
void bul_engine_apply_unique_amb_dep(bul_engine_s *engine, bul_target_s *target, bul_usage_t usage);

/**
 * @brief Asserts the validity of the finished inferencing state.
 *
 * @param[in] engine Engine context to evaluate.
 * @return NULL if valid, otherwise returns an error message.
 */
char *bul_engine_assert_valid(bul_engine_s *engine);

#endif // BUL_ENGINE_H
