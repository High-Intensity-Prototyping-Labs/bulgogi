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
void client::cli(CLI::App& app) {
        // App settings
        app.require_subcommand();

        // Subcommands
        auto module = app.add_subcommand("module", "Manage project modules")
                ->require_subcommand();

        auto tree = app.add_subcommand("tree", "Displays a tree of the project")
                ->require_subcommand();

        auto clean = app.add_subcommand("clean", "Cleans the project")
                ->require_subcommand();

        // Module subcommands
        auto module_add = module->add_subcommand("add", "Adds a module to the project")
                ->require_subcommand();

        auto module_rm = module->add_subcommand("rm", "Removes a module from the project")
                ->require_subcommand();

        // Module add subcommand config
        module_add->add_option("MODULE", "Name of module to add");
        module_add->add_option("TARGET", "Parent target (depends on module)");
        module_add->add_flag("--create", "Create new module if not found in FS.");
}

void client::add_module(void) {
        Project project;

        project.targets["default"] = Target::Executable;
        project.modules["module1"] = Module::Executable;
}
