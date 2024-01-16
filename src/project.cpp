/**
 * @file project.cc
 * @brief Project handler
 */

#include "project.hpp"

using project::Project;
using project::Dependency;

Project Project::make() {
        return (struct Project) {
                .targets = unordered_map<string, vector<Dependency>>(),
        };
}

Project Project::load() {
        // Load project.yaml
        YAML::Node project = YAML::LoadFile(PROJECT_YAML);

        if(project["default"]) {
                cout << "Default target found containing : " << project["default"] << endl;
        }

        return Project::make();
}

Dependency Dependency::make(Dependency::Kind kind, string name) {
        return (struct Dependency) {
                .type = kind,
                .name = name,
        };
}
