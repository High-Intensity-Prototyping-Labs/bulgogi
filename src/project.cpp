/**
 * @file project.cc
 * @brief Project handler
 */

#include "project.hpp"

using project::Target;
using project::Project;
using project::TargetID;
using project::Dependency;

Project Project::make() {
        return (struct Project) {
                .targets = unordered_map<TargetID, Target>(),
                .deps = unordered_map<TargetID, vector<Dependency>>(),
        };
}

Project Project::load() {
        // Load project.yaml
        YAML::Node project = YAML::LoadFile(PROJECT_YAML);
        auto map = project.as<unordered_map<string, vector<string>>>();
        return Project::from(map);
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
                                deps.push_back(Dependency::make(Dependency::Target, dep));
                        // Otherwise, it is a module
                        } else {
                                deps.push_back(Dependency::make(Dependency::Module, dep));
                        }
                }

                // Add the Target/Vec<Dependency> pair to the project targets
                project.deps.insert({target, deps});
        }

        return project;
}

Target Target::make(Target::Kind kind) {
        return Target {
                .type = kind,
        };
}

Dependency Dependency::make(Dependency::Kind kind, string name) {
        return (struct Dependency) {
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
        for(auto it = project.deps.begin(); it != project.deps.end(); it++) {
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
