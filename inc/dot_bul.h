/**
 * @file dot_bul.h
 * @brief Manages the .bul directory in the project
 */

#ifndef BUL_DOT_BUL_H
#define BUL_DOT_BUL_H

// Project headers 
#include "engine.h"
#include "fs.h"

// Settings 
#define DOT_BUL ".bul"

/** Global engine context */
extern bul_engine_s engine;

/**
 * @brief Initializes the bulgogi directory and the context engine.
 *
 * ASSUMPTIONS:
 * 1. The global engine context has not been initialized already.
 *      a. This could cause a memory leak if done repeatedly in the same session.
 *
 */
void bul_dot_init(void);

/**
 * @brief Adds a target tracker to the bulgogi directory.
 *
 * NOTE:
 * This function uses target name hints to signal the intended 
 * usage of the target. Because of built-in target rules, there 
 * is no guarantee that the user-configured dependency tree is 
 * valid according to the desired hints.
 *
 * This may cause dependencies initially added as EXE to become 
 * LIB as some target may be linked (depend on) a target which 
 * was originally labelled as EXE, for example.
 *
 * ASSUMPTIONS:
 * 1. The engine is assumed to have been initialized.
 *
 * @param[in] name Clean name of the target to add.
 * @param[in] usage Target usage (type from user POV).
 * @return ID of the added target or `UINT_MAX` in case of failure.
 */
bul_id_t bul_dot_add_target(bul_name_t name, bul_usage_t usage);

/**
 * @brief Adds dep to the specified target by ID.
 *
 * ASSUMPTIONS:
 * 1. The engine is assumed to have been initialized.
 * 2. Both `target` and `dep` must exist as valid targets in the engine.
 *
 * @param[in] target ID of the target to add `dep` to.
 * @param[in] dep ID of the dep to add to `target`.
 */
void bul_dot_add_target_dep(bul_id_t target, bul_id_t dep);

/**
 * @brief Adds sources to a target according to a path pattern.
 *
 * PATTERNS:
 * - `*` - Will match any file at the directory level.
 * - `*.%` - Will match any file with the extension `%` (can be any extension).
 * - `**` - Will match any file at the directory level and recursively to exhaustion.
 * - `**.%` - Will match any file with the extension `%` at the directory level and recursively to exhaustion.
 *
 * @param[in] target ID of target to associate sources to.
 * @param[in] path Path pattern of source files to associate to `target`.
 */
void bul_dot_add_sources(bul_id_t target, bul_fs_path_t path);

#endif // BUL_DOT_BUL_H
