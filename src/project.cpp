/**
 * @file project.cc
 * @brief Project handler
 */

#include "project.hpp"

using project::Err;
using project::Project;
using project::Dependency;

// Using declarations
using std::ios;
using std::string;
using std::vector;
using std::ofstream;
using std::unordered_map;

Project Project::make() {
        return Project {
                .targets = unordered_map<string, vector<Dependency>>(),
        };
}

Project Project::load() {
        // Load project.yaml
        YAML::Node project = YAML::LoadFile(PROJECT_YAML);
        auto map = project.as<unordered_map<string, vector<string>>>();
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
bool Project::contains_module(string& m) {
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
bool Project::contains_module(string& m, string& t) {
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

Dependency Dependency::from(Dependency::Kind kind, string name) {
        return Dependency {
                .type = kind,
                .name = name,
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
