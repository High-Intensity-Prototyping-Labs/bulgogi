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

using std::string;
using std::vector;
using std::unordered_map;

namespace project {
        // Type Aliases
        using target_id = std::string;
        using module_id = std::string;

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
                string dep_id;
        };

        class Project {
        public:
                unordered_map<target_id, Target> targets;
                unordered_map<module_id, Module> modules;
                unordered_map<target_id, vector<Dependency>> deps;
        };
}

