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

} bul_target_s;

/**
 * struct bul_engine - Stores inference engine state.
 *
 * @in_seq - Whether or not the parser is currently in a sequence.
 * @size - Number of targets in the project (also = number of names).
 * @names - List of target names arranged by ID. (names[ID] = name).
 * @targets - List of targets arranged by ID. (targets[ID] = target).
 * @deps - List of dependencies by target ID. (deps[ID] = {IDs}).
 */
typedef struct bul_engine {
        bool in_seq;
        size_t size;
        bul_name_t *names;
        bul_target_s *targets;
        bul_id_t **deps;

} bul_engine_s;

#endif // BUL_ENGINE_H
