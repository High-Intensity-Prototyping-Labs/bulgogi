/**
 * @headerfile client.hpp "client.hpp"
 * @brief Provides handlers for the CLI.
 */

#pragma once

// Project headers
#include "CLI11.hpp"

// Standard C++ Libraries
#include <string>

namespace client {
        // Placeholder for the CLI11 args 
        struct Args {
                std::string  MODULE;
                std::string  TARGET;
                bool    all;
                bool    create;
                bool    cached;
        };

        // Client errors 
        enum class Err {
                TargetNotFound,
                DuplicateModule,
                OpenProjectYamlErr,
                ModuleDirMissing,
                SaveProjectErr,
                ProjectAlreadyInit,
                TreeCmdFailed,
        };

        // Configures the CLI11 app for CLI args.
        void cli(CLI::App& app, Args& args);
        void err(Err e, std::optional<std::string> info);

        // Commands
        void init(void);
        void add_module(Args& args);
        void rm_module(Args& args);
        void tree(void);
        void test(void);
}
