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
                bool exe_flag;

                static Dependency from(Dependency::Kind kind, std::string& name);
                static Dependency from(Dependency::Kind kind, std::string& name, bool exe);
        };
        std::ostream& operator<<(std::ostream& out, Dependency& dep);
        bool operator==(const Dependency& a, Dependency& b);

        enum class Err {
                None,
                IOError,
        };

        enum class Usage {
                Libmodule,
                Exemodule,
                Ambiguous,
        };
        std::ostream& operator<<(std::ostream&, Usage);

        using TargetID = std::string;
        using ModuleID = std::string;
        using DependID = std::string;
        struct Project {
                std::unordered_map<TargetID, std::vector<Dependency>> targets;

                static Project make(void);
                static Project from(std::unordered_map<std::string, std::vector<std::string>>);
                Project copy(void);

                template<class T>
                T to(void);

                static Project load(void);
                Err save(void);

                bool contains_module(ModuleID& m);
                bool contains_module(ModuleID& m, TargetID& t);

                bool any_depends(DependID& m);
                bool any_depends(DependID& m, Dependency::Kind k);

                Usage get_usage(ModuleID& m);
                Usage get_usage(ModuleID& m, std::unordered_map<ModuleID, Usage>&);
                Usage get_dep_usage(Dependency& d);

                std::vector<ModuleID> modules(void);
                std::vector<ModuleID> modules(TargetID& t);

                std::vector<TargetID> libraries(void);
                std::vector<TargetID> executables(void);
        };
        template<>
        std::unordered_map<std::string, std::vector<std::string>> Project::to();

        std::ostream& operator<<(std::ostream& out, Project& project);
}
