/**
 * @file bul_dot_dul.h
 * @brief Manages the .bul directory in the project
 */

#ifndef BUL_DOT_BUL_H
#define BUL_DOT_BUL_H

// Project headers 
#include "engine.h"

// Settings 
#define DOT_BUL ".bul"

/** Global engine context */
extern bul_engine_s engine;

/**
 * @brief Initializes the bulgogi directory.
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
 *
 * @param[in] name Clean name of the target to add.
 * @param[in] usage Target usage (type from user POV).
 * @return ID of the added target or `UINT_MAX` in case of failure.
 */
bul_id_t bul_dot_add_target(bul_name_t name, bul_usage_t usage);

#endif // BUL_DOT_BUL_H
