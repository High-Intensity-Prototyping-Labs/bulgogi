/**
 * @headerfile project.hpp "project.hpp"
 * @brief Project handler.
 */

#pragma once 

// Project Settings 
#define PROJECT_YAML "project.yaml"

// Standard C++ Libraries
#include <string>
#include <vector>
#include <fstream>
#include <optional>
#include <unordered_map>

namespace project {
        struct Dependency {
                enum Kind { Target, Module } type;
                std::string name;
                bool exe;

                static Dependency from(Dependency::Kind kind, std::string& name);
                static Dependency from(Dependency::Kind kind, std::string& name, bool exe);
        };
        std::ostream& operator<<(std::ostream& out, Dependency& dep);
        bool operator==(const Dependency& a, Dependency& b);

        enum class Err {
                None,
                IOError,
        };

        struct Project {
                std::unordered_map<std::string, std::vector<Dependency>> targets;

                static Project make(void);
                static Project from(std::unordered_map<std::string, std::vector<std::string>>);
                Project copy(void);

                template<class T>
                T to(void);

                static Project load(void);
                Err save(void);

                bool contains_module(std::string& m);
                bool contains_module(std::string& m, std::string& t);

                bool any_depends(std::string& m);
                bool any_depends(std::string& m, Dependency::Kind k);

                std::vector<std::string> modules(void);
        };
        template<>
        std::unordered_map<std::string, std::vector<std::string>> Project::to();

        std::ostream& operator<<(std::ostream& out, Project& project);
}
