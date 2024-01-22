/**
 * @file project.cc
 * @brief Project handler
 */

#include "project.hpp"

// Standard C++ Libraries
#include <set>
#include <tuple>
#include <variant>
#include <iostream>
#include <optional>
#include <algorithm>
#include <filesystem>

// External Dependencies 
#include "yaml-cpp/yaml.h"

using project::Err;
using project::Usage;
using project::Project;
using project::TargetID;
using project::ModuleID;
using project::DependID;
using project::Dependency;

// Using declarations
using std::ios;
using std::set;
using std::string;
using std::vector;
using std::ostream;
using std::ofstream;
using std::unordered_map;

namespace fs = std::filesystem;

ostream& project::operator<<(ostream& out, Usage u) {
        switch(u) {
        case Usage::Ambiguous:
                out << "Usage::Ambiguous";
                break;
        case Usage::Exemodule:
                out << "Usage::Exemodule";
                break;
        case Usage::Libmodule:
                out << "Usage::Libmodule";
                break;
        }

        return out;
}

Project Project::make() {
        return Project {
                .targets = unordered_map<string, vector<Dependency>>(),
        };
}

Project Project::load() {
        auto map = unordered_map<string, vector<string>>();

        // Test if file is empty 
        if(!fs::is_empty(PROJECT_YAML)) {
                // Load project.yaml
                YAML::Node project = YAML::LoadFile(PROJECT_YAML);
                map = project.as<unordered_map<string, vector<string>>>();
        }

        return Project::from(map);
}

Err Project::save() {
        // Save project.yaml, proper override 
        YAML::Emitter yaml;
        auto f = ofstream(PROJECT_YAML, ios::out | ios::trunc);
        if(f.is_open()) {
                auto map = this->to<unordered_map<string, vector<string>>>();

                yaml << YAML::BeginMap;
                for(auto it: map) {
                        auto target = it.first;
                        auto str_list = it.second;

                        yaml << YAML::Key << target;
                        yaml << YAML::Value << str_list;
                }
                yaml << YAML::EndMap;

                f << yaml.c_str();

                return Err::None;
        } else {
                return Err::IOError;
        }
}

// Convert project.yaml map into project struct
Project Project::from(unordered_map<string, vector<string>> targets) {
        auto project = Project::make();

        for(auto it = targets.begin(); it != targets.end(); it++) {
                auto target = it->first;
                auto dep_list = it->second;
                auto deps = vector<Dependency>();

                // Convert the list of dep strings into a vec of Dependency
                for(auto it2 = dep_list.begin(); it2 < dep_list.end(); it2++) {
                        auto dep = *it2;
                        // If dep is found among the target names, its dependency type is target
                        if(targets.count(dep)) {
                                deps.push_back(Dependency::from(Dependency::Target, dep));
                        // Otherwise, it is a module
                        } else {
                                deps.push_back(Dependency::from(Dependency::Module, dep));
                        }
                }

                // Add the Target/Vec<Dependency> pair to the project targets
                project.targets.insert({target, deps});
        }

        return project;
}

// Return identical copy of Project 
Project Project::copy() {
        return Project {
                .targets = unordered_map<string, vector<Dependency>>(this->targets),
        };
}

template<>
unordered_map<string, vector<string>> Project::to() {
        auto map = unordered_map<string, vector<string>>();

        for(auto& it: this->targets) {
                auto target = it.first;
                auto dep_list = it.second;

                auto str_list = vector<string>();
                for(auto& it2: dep_list) {
                        auto dep = it2;

                        str_list.push_back(dep.name);
                }
                map.insert({target, str_list});
        }
        
        return map;
}

// Checks whether project contains module at all
bool Project::contains_module(ModuleID& m) {
        bool res = false;

        for(auto it = this->targets.begin(); it != this->targets.end(); it++) {
                auto target = it->first;
                if(this->contains_module(m, target)) {
                        res = true;
                        break;
                }
        }

        return res;
}

// Checks whether a project contains a module in a specific target
bool Project::contains_module(ModuleID& m, TargetID& t) {
        bool res = false;

        if(this->targets.contains(t)) {
                auto dep_list = this->targets[t];
                auto dep = Dependency::from(Dependency::Module, m);
                if(std::find(dep_list.begin(), dep_list.end(), dep) != dep_list.end()) {
                        res = true;
                }
        }

        return res;
}

// Returns whether any targets in the project depend on the specified module 
bool Project::any_depends(DependID& m) {
        bool any_depends = false;

        auto matching_dep = [&](Dependency& d) {
                return d.name == m;
        };

        for(auto& [target, dep_list]: this->targets) {
                if(std::any_of(dep_list.begin(), dep_list.end(), matching_dep)) {
                        any_depends = true;
                        break;
                }
        }

        return any_depends;
}

bool Project::any_depends(DependID& m, Dependency::Kind k) {
        bool any_depends = false;
        
        auto matching_both = [&](Dependency& d) {
                return (d.name == m && d.type == k);
        };

        for(auto& [target, dep_list]: this->targets) {
                if(std::any_of(dep_list.begin(), dep_list.end(), matching_both)) {
                        any_depends = true;
                        break;
                }
        }

        return any_depends;
}

// Assumes that mod is in the list of project.modules()
// Assumes ambiguity can be resolved within 1 pass.
// Assumes the parent call to this function provides a clean instance of the usages vec
Usage Project::get_usage(ModuleID& mod, unordered_map<ModuleID, Usage>& usages) {
        Usage use = Usage::Ambiguous;

        // Go after library targets 
        for(auto& lib: this->libraries()) {
                if(this->contains_module(mod, lib)) {
                        use = Usage::Libmodule;
                        usages.insert({mod, use});
                        return use;
                }
        }
        // NOTE: Deliberate early to prevent uncessarily checking executables

        // Check any executables that it is the sole module 
        for(auto& exe: this->executables()) {
                if(this->contains_module(mod, exe) ) {
                        if(this->modules(exe).size() == 1) {
                                use = Usage::Exemodule;
                                break;
                        } else {
                                // See if the others are unambiguous
                                int other_amb = 0;
                                bool other_exe = false;
                                Usage other_use = Usage::Ambiguous;

                                for(auto& other_m: this->modules(exe)) {
                                        // Skip current module 
                                        if(other_m == mod) {
                                                continue;
                                        }

                                        // Obtain other modules' ambiguity
                                        if(usages.contains(other_m)) {
                                                other_use = usages[other_m];
                                        } else {
                                                // Seeking external conf - temporarily setting 
                                                // this mod as Ambiguous in hope of resolve.
                                                usages.insert({mod, Usage::Ambiguous});

                                                other_use = this->get_usage(other_m, usages);
                                        }

                                        // Test ambiguity
                                        if(other_use == Usage::Ambiguous) {
                                                other_amb++;
                                        } else if(other_use == Usage::Exemodule) {
                                                other_exe = true;
                                        }
                                }

                                // See if ambiguity eliminated 
                                if(other_exe) {
                                        use = Usage::Libmodule;
                                /* other_exe == false */
                                } else if(!other_amb) {
                                        use = Usage::Exemodule;
                                } else {
                                        use = Usage::Ambiguous;
                                }
                                usages.insert({mod, use});
                                /* End of the line */
                        }
                }

                // See if there's an escape 
                if(use != Usage::Ambiguous) {
                        break;
                }
        }

        return use;
}

vector<ModuleID> Project::modules() {
        auto modules = vector<string>();
        auto unique = set<string>();

        for(auto& [_, dep_list]: this->targets) {
                for(auto& d: dep_list) {
                        if(d.type == Dependency::Module) {
                                modules.push_back(d.name);
                        }
                }
        }

        unique.insert(modules.begin(), modules.end());
        modules.assign(unique.begin(), unique.end());

        return modules;
}

vector<ModuleID> Project::modules(string& t) {
        auto modules = vector<string>();

        if(this->targets.contains(t)) {
                for(auto& d: this->targets[t]) {
                        if(d.type == Dependency::Module) {
                                modules.push_back(d.name);
                        }
                }
        }

        return modules;
}

vector<TargetID> Project::libraries() {
        vector<TargetID> libs;

        for(auto& [target, _]: this->targets) {
                if(this->any_depends((string&)target)) {
                        libs.push_back(target);
                }
        }

        return libs;
}

vector<TargetID> Project::executables() {
        vector<TargetID> exes;

        for(auto& [target, _]: this->targets) {
                if(!this->any_depends((string&)target)) {
                        exes.push_back(target);
                }
        }

        return exes;
}

Dependency Dependency::from(Dependency::Kind kind, string& name) {
        auto clean_name = string(name);
        std::erase(clean_name, '*');

        if(kind == Dependency::Module) {
                return Dependency {
                        .type = kind,
                        .name = clean_name,
                        .exe_flag = std::ranges::count(name, '*') > 0,
                };
        } else {
                return Dependency {
                        .type = kind,
                        .name = clean_name,
                        .exe_flag = false,
                };
        }
}

Dependency Dependency::from(Dependency::Kind kind, string& name, bool exe) {
        auto clean_name = string(name);
        std::erase(clean_name, '*');

        return Dependency {
                .type = kind,
                .name = clean_name,
                .exe_flag = exe,
        };
}

std::ostream& project::operator<<(std::ostream& out, Dependency& dep) {
        out << "(";
        if(dep.type == Dependency::Target) {
                out << "Dependency::Target";
        } else {
                out << "Dependency::Module";
        }
        out << ", " << dep.name << ")";

        return out;
}


std::ostream& project::operator<<(std::ostream& out, Project& project) {
        out << "Project {" << std::endl;
        out << "\ttargets: {" << std::endl;
        for(auto it = project.targets.begin(); it != project.targets.end(); it++) {
                auto target = it->first;
                auto dep_list = it->second;

                out << "\t\t" << target << ": {" << std::endl;
                for(auto it2 = dep_list.begin(); it2 != dep_list.end(); it2++) {
                        auto dep = *it2;
                        out << "\t\t\t" << dep << ", " << std::endl;
                }
                out << "\t\t}" << std::endl;
        }
        out << "\t}" << std::endl;
        out << "}" << std::endl;

        return out;
}

bool project::operator==(const Dependency& a, Dependency& b) {
        return a.type == b.type && a.name == b.name;
}
