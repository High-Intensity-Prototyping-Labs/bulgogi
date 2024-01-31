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

/**
 * @brief Initializes the bulgogi directory.
 *
 */
void bul_dot_init(void);

/**
 * @brief Adds a target tracker to the bulgogi directory.
 *
 * @param[in] name Clean name of the target to add.
 * @param[in] usage Target usage (type from user POV).
 */
void bul_dot_add_target(bul_name_t name, bul_usage_t usage);

#endif // BUL_DOT_BUL_H
