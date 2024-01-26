/**
 * @file yaml_ext.h
 * @brief Custom extensions to the libyaml API.
 */

#ifndef YAML_EXT_H
#define YAML_EXT_H

// Standard C Libraries 
#include <stddef.h>
#include <stdint.h>

// External Dependencies
#include "yaml.h"


/**
 * @brief Prints a pretty struct of a yaml_event_t
 *
 * @param[in] event Pointer to the initialized event to print.
 *
 */
void yaml_print_event(yaml_event_t* event);

#endif // YAML_EXT_H
