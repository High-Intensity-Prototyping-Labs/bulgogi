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
using project::module_id;
using project::target_id;
using project::Dependency;

using std::string;

namespace client {
        void cli(CLI::App&);
        Project load(void);
        void add_module(module_id& module, target_id& target);
}
