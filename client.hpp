/**
 * @headerfile client.hpp "client.hpp"
 * @brief Provides handlers for the CLI.
 */

#pragma once

// Project headers
#include "CLI11.hpp"
#include "project.hpp"

// Standard C++ Libraries
#include <string>

using project::Project;
using project::Target;
using project::Module;

namespace client {
        void cli(CLI::App&);
        void add_module(void);
}
