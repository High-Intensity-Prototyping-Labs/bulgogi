/**
 * @file client.cc
 * @brief Provides handlers for the CLI.
 */

#include "client.hpp"

// Standard C Libraries 
#include <sys/wait.h>
#include <unistd.h>

// Standard C++ Libraries
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <utility>
#include <optional>
#include <algorithm>
#include <filesystem>
#include <unordered_map>

// Project Headers
#include "project.hpp"
#include "directory-tree-print.hpp"

// Using Declarations
using std::string;
using std::vector;
using std::ofstream;

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
        auto init = app.add_subcommand("init", "Initializes the project in the current working directory");

        auto module = app.add_subcommand("module", "Manage project modules")
                ->require_subcommand();

        auto tree = app.add_subcommand("tree", "Displays a tree of the project");

        auto clean = app.add_subcommand("clean", "Cleans the project")
                ->require_subcommand();
        (void)clean;

        auto test = app.add_subcommand("test", "Runs a test feature");

        // Init subcommand 
        init->callback([]() { client::init(); });

        // Module subcommands
        auto module_add = module->add_subcommand("add", "Adds a module to the project");
        auto module_rm = module->add_subcommand("rm", "Removes a module from the project")
                ->alias("remove");

        // Module add subcommand config
        module_add->add_option<string>("MODULE", args.MODULE, "Name of module to add")->required(true);
        module_add->add_option<string>("TARGET", args.TARGET, "Parent target (depends on module)")->default_val("default");
        module_add->add_flag<bool>("--create", args.create, "Create new module if not found in FS.")->default_val(false);
        module_add->callback([&]() { add_module(args); });

        // Module rm subcommand config 
        module_rm->add_option<string>("MODULE", args.MODULE, "Name of the module to remove")->required(true);
        module_rm->add_option<string>("TARGET", args.TARGET, "Target attached to the module")->default_val("default");
        module_rm->add_flag<bool>("--all", args.all, "Remove all module attachments to targets")->default_val(false);
        module_rm->add_flag<bool>("--cached", args.cached, "Only remove the module's listing the project yaml")->default_val(false);
        module_rm->callback([&]() { rm_module(args); });

        // Tree subcommand config
        tree->callback([]() { client::tree(); });

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
        case Err::SaveProjectErr:
                std::cout << "Failed to save " << PROJECT_YAML << ". IO Error." << std::endl;
                break;
        case Err::ProjectAlreadyInit:
                std::cout << "Project already initialized." << std::endl;
                break;
        case Err::TreeCmdFailed:
                std::cout << "Failed to run `tree` command using popen()" << std::endl;
                break;
        case Err::TargetDepends:
                std::cout << "At least 1 other target depends on target: " << info.value_or(VALUE_UNKNOWN) << std::endl;
                break;
        }
}

void client::init() {
        // Create project.yaml file. If exists, do nothing.
        auto path = fs::path(PROJECT_YAML);
        if(!fs::exists(path)) {
                auto f = ofstream(path, ofstream::out);
        } else {
                client::err(Err::ProjectAlreadyInit, std::nullopt);
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
        bool proj_chged = false;

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
                        proj_chged = true;
                } else if(args.create) {
                        // Auto-create target and add module to the project
                        project.targets.insert({args.TARGET, vector<Dependency>{
                                Dependency::from(Dependency::Module, args.MODULE),
                        }});
                        proj_chged = true;
                } else {
                        client::err(Err::TargetNotFound, args.TARGET);
                }
        }

        if(need_spawn) {
                fs::create_directories(src);
                fs::create_directories(inc);
                fs::create_directories(pri);
        }

        if(proj_chged) {
                auto err = project.save();
                if(err != project::Err::None) {
                        client::err(Err::SaveProjectErr, std::nullopt);
                }
        }
}

void client::rm_module(Args& args) {
        // Load project 
        auto project = Project::load();
        auto ref_copy = project.copy();

        // Closure to detect matching dependency against module name
        auto matching_dep = [&](Dependency& d) {
                return d.name == args.MODULE;
        };

        auto matching_target = [&](Dependency& d) {
                return d.name == args.TARGET;
        };
        (void)matching_target;

        // Set wildcard condition
        bool wildcard = (args.MODULE == "*");

        // Check wildcard condition
        int erased = 0;
        if(wildcard && !project.targets.empty()) {
                if(!project.any_depends(args.TARGET, Dependency::Target)) {
                        if(args.all) {
                                project.targets.clear();
                                erased++;
                        } else if(project.targets.contains(args.TARGET)) {
                                project.targets.erase(args.TARGET);
                                erased++;
                        }
                } else {
                        client::err(Err::TargetDepends, args.TARGET);
                }
        // Remove all instances in the project.yaml
        } else if(args.all) {
                for(auto& [target, dep_list]: project.targets) {
                        erased += std::erase_if(dep_list, matching_dep);
                }
        /* args.all == false */
        } else if(project.targets.contains(args.TARGET)) {
                auto& dep_list = project.targets[args.TARGET];
                erased = std::erase_if(dep_list, matching_dep);
        /* args.all == false && project.targets.contains(args.TARGET) == false */
        } else {
                client::err(Err::TargetNotFound, args.TARGET);
        }

        // See if any targets still depend on the module
        bool any_depends = project.any_depends(args.MODULE, Dependency::Module);

        // TODO:
        // ~1. Create an Project::any_depends() method.
        // ~2. Create a ref_copy of the project before nuking it with the wildcard.
        // ~3. When nuking is complete, check each dep in the ref_copy with Project::any_depends(),
        // ~4. Those which any do not depend, remove from the filesystem as well (unless --cached was passed),
        //      Those which are still depended on must stay in the project root.
        // ~5. Add a clause which prevents a wildcard remove of just one target which ensures 
        //      that not other targets depend on that target as a dependency.
        //      Provide a client::err of dependency issue if that's the case.

        // Save project if a module was removed
        if(erased) {
                // Filesystem removal logic 
                if(!args.cached) {
                        if(wildcard && !args.all) {
                                /* assuming wildcard && erased implies the target existed */
                                for(auto& dep: ref_copy.targets[args.TARGET]) {
                                        if(dep.type == Dependency::Module && !project.any_depends(dep.name, Dependency::Module)) {
                                                // Nuke out of FS
                                                fs::remove_all(fs::path(dep.name));
                                        }
                                }
                        } else if(!any_depends) {
                                // Remove from the filesystem
                                fs::remove_all(fs::path(args.MODULE));
                        /* any_depends == true || args.cached == true */
                        } 
                }

                // Remove any empty targets
                auto ref_project = project.copy();
                for(auto& [target, dep_list]: ref_project.targets) {
                        if(dep_list.empty()) {
                                project.targets.erase(target);
                        }
                }

                auto err = project.save();
                if(err != project::Err::None) {
                        client::err(Err::SaveProjectErr, std::nullopt);
                }
        }
}

void client::tree() {
        // Load project 
        auto project = Project::load();

        // Run this function for every module.
        vector<string> args;
        vector<string> printed;
        for(auto& [target, dep_list]: project.targets) {
                for(auto& dep: dep_list) {
                        if(!std::any_of(printed.begin(), printed.end(), [&](string& s) { return dep.name == s; })) {
                                args.push_back(dep.name);
                                printed.push_back(dep.name);
                        }
                }
        }

        // Final shebang
        string argv = "tree";
        std::array<char, 32> buffer;

        for(size_t x = 0; x < args.size(); x++) {
                argv += " " + args[x];
        }
        FILE *fp = popen(argv.c_str(), "r");
        if(fp) {
                while(fgets(buffer.data(), buffer.size(), fp) != NULL) {
                        std::cout << buffer.data();
                }
                std::cout << std::endl;
        } else {
                client::err(Err::TreeCmdFailed, std::nullopt);
        }
}

void client::test() {
        // Load project 
        auto project = Project::load();
        std::cout << project << std::endl;
}
