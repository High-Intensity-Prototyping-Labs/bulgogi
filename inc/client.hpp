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

// Using Declarations
using std::string;
using project::Project;

namespace client {
        void cli(CLI::App& app);

        // Commands
        void add_module(string MODULE, string TARGET, bool create);
}
