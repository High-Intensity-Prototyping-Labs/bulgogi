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

// External Libraries 
#include "inja.hpp"

// Project Headers
#include "project.hpp"
#include "cmake.hpp"

// Using Declarations
using std::string;
using std::vector;
using std::ofstream;
using std::unordered_map;

using inja::json;
using inja::Environment;

using project::Usage;
using project::Project;
using project::ModuleID;
using project::Dependency;

using cmake::CMakeProject;

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

        auto generate = app.add_subcommand("generate", "Generates the build files")
                ->alias("gen");

        auto build = app.add_subcommand("build", "Builds the project");

        auto clean = app.add_subcommand("clean", "Cleans project build files");

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
        module_add->add_flag<bool>("-x,--exe", args.exe, "Specifices that the module contains an executable component.")->default_val(false);
        module_add->callback([&]() { add_module(args); });

        // Module rm subcommand config 
        module_rm->add_option<string>("MODULE", args.MODULE, "Name of the module to remove")->required(true);
        module_rm->add_option<string>("TARGET", args.TARGET, "Target attached to the module")->default_val("default");
        module_rm->add_flag<bool>("--all", args.all, "Remove all module attachments to targets")->default_val(false);
        module_rm->add_flag<bool>("--cached", args.cached, "Only remove the module's listing the project yaml")->default_val(false);
        module_rm->callback([&]() { rm_module(args); });

        // Tree subcommand config
        tree->callback([]() { client::tree(); });

        // Generate subcommand config 
        generate->add_flag<bool>("--create", args.create, "Create module directories (if missing)")->default_val(false);
        generate->callback([&]() { client::generate(args); });

        // Build subcommand config 
        build->callback([]() { client::build(); });

        // Clean subcommand config 
        clean->add_flag<bool>("--all", args.all, "Also cleans generated CMakeLists.txt")->default_val(false);
        clean->add_flag<bool>("--purge", args.purge, "Permanently deletes module sources (DANGER)")->default_val(false);
        clean->callback([&]() { client::clean(args); });

        // Test command config
        test->callback([]() { client::test(); });
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
        case Err::GenerateFaied:
                std::cout << "Failed to generate project" << std::endl;
                break;
        case Err::PurgeWithoutAll:
                std::cout << "The --purge flag must be passed with --all to CONFIRM permanent removal of sources" << std::endl;
                break;
        case Err::Ambiguity:
                std::cout << "Ambiguity encountered. Consider resolving with an executable indicator (*)." << std::endl;
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

        // Check for duplicates 
        if(project.contains_module(args.MODULE, args.TARGET)) {
                client::err(Err::DuplicateModule, "("+args.MODULE+", "+args.TARGET+")");
                duplicates = true;
        }

        // Check if module directory exists in the FS
        if(client::module_dir_exists(args.MODULE)) {
                dir_exists = true;

                // Validate directory structure...
                valid_tree = client::valid_module_dirs(args.MODULE);
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
                        project.targets[args.TARGET].push_back(Dependency::from(Dependency::Module, args.MODULE, args.exe));
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
                client::create_module_dirs(args.MODULE);
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

        // Check wildcard condition
        int erased = 0;
        // Remove all instances in the project.yaml
        if(args.all) {
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

        // Save project if a module was removed
        if(erased) {
                // Filesystem removal logic 
                if(!args.cached) {
                        if(!any_depends) {
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
        auto args = project.modules();

        // Final shebang
        if(args.size() > 0) {
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
}

void client::generate(Args& args) {
        // Load project 
        auto project = Project::load();

        // Convert to CMakeProject 
        auto cmake = CMakeProject::from(project);

        // Generate the CMakeLists.txt
        bool pass = true;

        // Check for any ambiguity 
        if(cmake.lists.contains(AMBIGUOUS_DIR)) {
                client::err(Err::Ambiguity, std::nullopt);
                pass = false;
        } else {
                for(auto& [subdir, list]: cmake.lists) {
                        // Generate dirs condition
                        if(args.create) {
                                client::create_module_dirs((string&)subdir);
                        }

                        // Normal path 
                        if(client::module_dir_exists((string&)subdir)) {
                                if(subdir == TARGET_LIB_DIR) {
                                        list.generate_proj(cmake.lists);
                                } else {
                                        list.generate_mod((string&)subdir);
                                }

                                pass = true;
                        } else {
                                client::err(Err::ModuleDirMissing, subdir);
                                pass = false;
                        }
                }
        }

        if(!pass) {
                client::err(Err::GenerateFaied, std::nullopt);
        }
}

void client::build() {
}

void client::clean(Args& args) {
        // Clear build files 
        fs::remove_all(BUILD_DIR);

        // If --all 
        if(args.all) {
                // Load project 
                auto project = Project::load();

                // Clear generated CMakeLists.txt
                for(auto& m: project.modules()) {
                        fs::remove(client::module_dir(m) / CMAKE_LIST_TXT);

                        // (DANGER) Purges module sources as well 
                        if(args.purge) {
                                fs::remove_all(client::module_dir(m));
                        }
                }

                // Dont forget project-level
                fs::remove(client::project_dir() / CMAKE_LIST_TXT);
        } else if(args.purge) {
                // Flag --purge passed without --all
                client::err(Err::PurgeWithoutAll, std::nullopt);
        }
}

void client::test() {
        // Load project 
        auto project = Project::load();
        auto cmake = CMakeProject::from(project);

        if(cmake.lists.contains(AMBIGUOUS_DIR)) {
                client::err(Err::Ambiguity, std::nullopt);
        } else {
                std::cout << "Amen brother" << std::endl;
        }
}

fs::path client::project_dir() {
        return fs::path(PROJECT_DIR);
}

fs::path client::module_dir(std::string& m) {
        return fs::path(m);
}

fs::path client::module_src_dir(std::string& m) {
        return client::module_dir(m) / MODULE_SRC_DIR;
}

fs::path client::module_inc_dir(std::string& m) {
        return client::module_dir(m) / MODULE_INC_DIR;
}

fs::path client::module_pri_dir(std::string& m) {
        return client::module_src_dir(m) / MODULE_PRI_DIR;
}

void client::create_module_dirs(string& m) {
        fs::create_directories(client::module_dir(m));
        fs::create_directories(client::module_src_dir(m));
        fs::create_directories(client::module_inc_dir(m));
        fs::create_directories(client::module_pri_dir(m));
}

bool client::valid_module_dirs(std::string& m) {
        bool mod = client::module_dir_exists(m);
        bool src = fs::exists(client::module_src_dir(m));
        bool inc = fs::exists(client::module_inc_dir(m));
        bool pri = fs::exists(client::module_pri_dir(m));

        return (mod && src && inc && pri);
}

bool client::module_dir_exists(std::string& m) {
        return fs::exists(client::module_dir(m));
}
