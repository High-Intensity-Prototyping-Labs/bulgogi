/**
 * @headerfile client.hpp "client.hpp"
 * @brief Provides handlers for the CLI.
 */

#pragma once

// Project headers
#include "CLI11.hpp"

// Standard C++ Libraries
#include <string>
#include <optional>
#include <filesystem>

// Settings 
#define BUILD_DIR "build"
#define PROJECT_DIR "."
#define MODULE_SRC_DIR "src"
#define MODULE_INC_DIR "inc"
#define MODULE_PRI_DIR "inc"

namespace client {
        // Placeholder for the CLI11 args 
        struct Args {
                std::string  MODULE;
                std::string  TARGET;
                bool    all;
                bool    create;
                bool    cached;
                bool    purge;
                bool    exe;
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
                TargetDepends,
                GenerateFaied,
                PurgeWithoutAll,
                Ambiguity,
        };

        // Configures the CLI11 app for CLI args.
        void cli(CLI::App& app, Args& args);
        void err(Err e, std::optional<std::string> info);

        // Commands
        void init(void);
        void add_module(Args& args);
        void rm_module(Args& args);
        void tree(void);
        void generate(Args& args);
        void build(void);
        void clean(Args& args);
        void test(void);

        // Utilities 
        std::filesystem::path project_dir(void);
        std::filesystem::path module_dir(std::string& m);
        std::filesystem::path module_src_dir(std::string& m);
        std::filesystem::path module_inc_dir(std::string& m);
        std::filesystem::path module_pri_dir(std::string& m);

        void create_module_dirs(std::string& m);
        bool valid_module_dirs(std::string& m);
        bool module_dir_exists(std::string& m);
}
