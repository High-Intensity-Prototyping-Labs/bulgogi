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
#include <optional>
#include <filesystem>

// Using Declarations
using std::string;
using project::Project;
using project::Dependency;
namespace fs = std::filesystem;

namespace client {
        // Placeholder for the CLI11 args 
        struct Args {
                string  MODULE;
                string  TARGET;
                bool    create;
                bool    all;
        };

        // Client errors 
        enum class Err {
                TargetNotFound,
                DuplicateModule,
                OpenProjectYamlErr,
        };

        // Configures the CLI11 app for CLI args.
        void cli(CLI::App& app, Args& args);
        void err(Err e, std::optional<string> info);

        // Commands
        void add_module(Args& args);
        void rm_module(Args& args);
        void test(void);
}
