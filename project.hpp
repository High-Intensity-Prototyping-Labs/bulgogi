/**
 * @headerfile project.hpp "project.hpp"
 * @brief Project handler.
 */

#pragma once 

// Standard C++ Libraries
#include <string>
#include <vector>
#include <variant>
#include <unordered_map>

using String = std::string;
using std::unordered_map;

namespace project {
        // Type Aliases
        template<class T>
        using Vector = std::vector<T>;

        using TargetID = std::string;
        using ModuleID = std::string;

        // Enums
        enum class Target {
                Executable,
                Library,
        };

        enum class Module {
                Normal,
                Executable,
        };

        struct Dependency {
                enum { Target, Module } type;
                union id {
                        TargetID target_id;
                        ModuleID module_id;
                };
        };

        class Project {
        public:
                unordered_map<TargetID, Target> targets;
                unordered_map<ModuleID, Module> modules;
                unordered_map<TargetID, Vector<Dependency>> deps;
        };
}

