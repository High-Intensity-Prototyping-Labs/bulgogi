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
using project::operator<<;

namespace client {
        // Placeholder for the CLI11 args 
        struct Args {
                string  MODULE;
                string  TARGET;
                bool    create;
                bool    all;
        };

        // Configures the CLI11 app for CLI args.
        void cli(CLI::App& app, Args& args);

        // Commands
        void add_module(Args& args);
        void rm_module(Args& args);
}
