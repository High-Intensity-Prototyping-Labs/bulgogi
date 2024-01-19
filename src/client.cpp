/**
 * @file client.cc
 * @brief Provides handlers for the CLI.
 */

#include "client.hpp"

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

        auto clean = app.add_subcommand("clean", "Cleans the project")
                ->require_subcommand();

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
}

void client::err(Err e, std::optional<string> info) {
        const auto VALUE_UNKNOWN = "(UNKNOWN)";
        switch (e) {
        case Err::TargetNotFound:
                std::cout << "Target not found in project: " << info.value_or(VALUE_UNKNOWN) << std::endl;
                break;
        }
}

void client::add_module(Args& args) {
        // DEBUG:
        std::cout << "Adding a module..." << std::endl;
        if(args.create) {
                std::cout << "Going ahead with creating a new module in the FS" << std::endl;
        } else {
                std::cout << "Opting to instead search the FS for the MODULE=" << args.MODULE << std::endl;
        }
        std::cout << "The desired MODULE = " << args.MODULE << std::endl;
        std::cout << "The desired TARGET = " << args.TARGET << std::endl;

        // Load project 
        Project project = Project::load();

        // DEBUG:
        std::cout << "Project loaded. Contains the following: " << std::endl;
        std::cout << project << std::endl;

        // Add module to project 
        if(project.targets.count(args.TARGET)) {
                project.targets[args.TARGET].push_back(Dependency::make(Dependency::Module, args.MODULE));
        } else {
                client::err(Err::TargetNotFound, args.TARGET);
        }

        // DEBUG:
        std::cout << "Final Project State:" << std::endl;
        std::cout << project << std::endl;
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
