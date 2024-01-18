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
        // Group argument placeholders
        string MODULE;
        string TARGET;
        bool create;

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
        auto module_add = module->add_subcommand("add", "Adds a module to the project");

        auto module_rm = module->add_subcommand("rm", "Removes a module from the project")
                ->require_subcommand();

        // Module add subcommand config
        module_add->add_option<string>("MODULE", MODULE, "Name of module to add");
        module_add->add_option<string>("TARGET", TARGET, "Parent target (depends on module)");
        module_add->add_flag<bool>("--create", create, "Create new module if not found in FS.");

        // Module add callback 
        module_add->callback([&](){
                add_module(MODULE, TARGET, create);
        });
}

void client::add_module(string MODULE, string TARGET, bool create) {
        cout << "Adding a module..." << endl;
        if(create) {
                cout << "Going ahead with creating a new module in the FS" << endl;
        } else {
                cout << "Opting to instead search the FS for the MODULE=" << MODULE << endl;
        }
        cout << "The desired MODULE = " << MODULE << endl;
        cout << "The desired TARGET = " << TARGET << endl;

        // Load project 
        // Project project = Project::load();
}
