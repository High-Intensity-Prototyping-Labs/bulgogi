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

        // This works
        if(project["default"]) {
                auto dep_list = project["default"].as<vector<string>>();
                for(auto it = dep_list.begin(); it < dep_list.end(); ++it) {
                        std::cout << *it << std::endl;
                }
        }

        // This is kinda nonsense
        auto pp = project.as<vector<std::tuple<string, vector<string>>>>();
        for(auto it = pp.begin(); it < pp.end(); ++it) {
                std::cout << std::get<string>(*it) << ": ";

        }

        return Project::make();
}

Dependency Dependency::make(Dependency::Kind kind, string name) {
        return (struct Dependency) {
                .type = kind,
                .name = name,
        };
}
