/**
 * @file client.cc
 * @brief Provides handlers for the CLI.
 */

#include "client.hpp"
#include <filesystem>

// Using Declarations
using std::string;

using project::Project;
using project::Dependency;

// Namespace aliases
namespace fs = std::filesystem;
/**
 * Configures the CLI11 App 
 *
 * @param app Reference to the initialized CLI11 App in `main()`
 */
void client::cli(CLI::App& app, Args& args) {
        // App settings
        app.require_subcommand();
        app.preparse_callback([](size_t argCount){
                if (argCount == 0) {
                   throw(CLI::CallForHelp());
                }
        });

        // Subcommands
        auto module = app.add_subcommand("module", "Manage project modules")
                ->require_subcommand();

        auto tree = app.add_subcommand("tree", "Displays a tree of the project")
                ->require_subcommand();
        (void)tree;

        auto clean = app.add_subcommand("clean", "Cleans the project")
                ->require_subcommand();
        (void)clean;

        auto test = app.add_subcommand("test", "Runs a test feature");

        // Module subcommands
        auto module_add = module->add_subcommand("add", "Adds a module to the project");
        auto module_rm = module->add_subcommand("rm", "Removes a module from the project");

        // Module add subcommand config
        module_add->add_option<string>("MODULE", args.MODULE, "Name of module to add")->required(true);
        module_add->add_option<string>("TARGET", args.TARGET, "Parent target (depends on module)")->default_val("default");
        module_add->add_flag<bool>("--create", args.create, "Create new module if not found in FS.")->default_val(false);
        module_add->callback([&]() { add_module(args); });

        // Module rm subcommand config 
        module_rm->add_option<string>("MODULE", args.MODULE, "Name of the module to remove")->required(true);
        module_rm->add_option<string>("TARGET", args.TARGET, "Target attached to the module")->default_val("default");
        module_rm->add_flag<bool>("--all", args.all, "Remove all module attachments to targets")->default_val(false);
        module_rm->callback([&]() { rm_module(args); });

        // Test command config
        test->callback([&]() { client::test(); });
}

void client::err(Err e, std::optional<string> info) {
        const auto VALUE_UNKNOWN = "(UNKNOWN)";
        switch (e) {
        case Err::TargetNotFound:
                std::cout << "Target not found in project: " << info.value_or(VALUE_UNKNOWN) << std::endl;
                break;
        case Err::DuplicateModule:
                std::cout << "Module already attached to target: " << info.value_or(VALUE_UNKNOWN) << std::endl;
                break;
        case Err::OpenProjectYamlErr:
                std::cout << "Unable to open or create the " << PROJECT_YAML << "file." << std::endl;
                break;
        case Err::ModuleDirMissing:
                std::cout << "Module directory for " << info.value_or(VALUE_UNKNOWN) 
                        << " not found or incorrect. Perhaps you meant to pass the --create flag?" << std::endl;
                break;
        }
}

void client::add_module(Args& args) {
        // Load project 
        Project project = Project::load();

        // Placeholder to track module validation status 
        bool duplicates = false;
        bool dir_exists = false;
        bool valid_tree = false;

        // Placeholders for directories 
        auto path = fs::path(args.MODULE);
        auto src = fs::path(path / "src");
        auto inc = fs::path(path / "inc");
        auto pri = fs::path(src / "inc");

        // Check for duplicates 
        if(project.contains_module(args.MODULE, args.TARGET)) {
                client::err(Err::DuplicateModule, "("+args.MODULE+", "+args.TARGET+")");
                duplicates = true;
        }

        // Check if module directory exists in the FS
        if(fs::is_directory(path)) {
                dir_exists = true;

                // Validate directory structure...
                bool src_exists = fs::is_directory(src);
                bool inc_exists = fs::is_directory(inc);
                bool pri_exists = fs::is_directory(pri);
                
                valid_tree = src_exists && inc_exists && pri_exists;
        }

        //-- Main control branch --//
        
        bool add_module = false;
        bool need_spawn = false;

        if(!duplicates) {
                if(dir_exists && valid_tree) {
                        // Add module 
                        add_module = true;
                /* dir_exists == false || valid_tree == false */
                } else if(args.create) {
                        // Spawn directory 
                        need_spawn = true;
                        // Add module
                        add_module = true;
                /* (dir_exists == false || valid_tree == false) && args.create == false */
                } else {
                        // There is either something wrong with the directory structure 
                        // or it is outright missing.
                        client::err(Err::ModuleDirMissing, args.MODULE);
                }
        }

        //-- Main actions --//

        if(add_module) {
                // Add module to project 
                if(project.targets.count(args.TARGET)) {
                        project.targets[args.TARGET].push_back(Dependency::from(Dependency::Module, args.MODULE));
                        project.save();
                } else {
                        client::err(Err::TargetNotFound, args.TARGET);
                }
        }

        if(need_spawn) {
                fs::create_directories(src);
                fs::create_directories(inc);
                fs::create_directories(pri);
        }

        // TODO:
        // ~1. Check for duplicates before adding,
        // ~2. Check if directory exists in the filesystem (done at the CLI11 level if flag isnt passed),
        // ~3. If --create flag is passed, create directory structure,
        // 4. Have a means to save the project (Project::save() or something).
}

void client::rm_module(Args& args) {
        std::cout << "Removing a module..." << std::endl;
        if(args.all) {
                std::cout << "Removing ALL modules found" << std::endl;
        } else {
                std::cout << "Just removing module (" << args.MODULE << ") attached to target (" << args.TARGET << std::endl;
        }

        std::cout << "Module to remove: " << args.MODULE << std::endl;
        std::cout << "Target attached: " << args.TARGET << std::endl;
}

void client::test() {
        // Load project 
        auto project = Project::load();

        // Remove default target
        project.targets.erase("default");

        // Save project 
        project.save();
}
