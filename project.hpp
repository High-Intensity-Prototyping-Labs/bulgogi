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
using TargetID = std::string;
using ModuleID = std::string;
using Dependency = std::variant<TargetID, ModuleID>;

template<class T>
using Vector = std::vector<T>;

using std::unordered_map;

namespace project {
        // Enums
        enum class Target {
                Executable,
                Library,
        };

        enum class Module {
                Normal,
                Executable,
        };

        class Project {
        public:
                unordered_map<TargetID, Target> targets;
                unordered_map<ModuleID, Module> modules;
                unordered_map<TargetID, Vector<Dependency>> deps;
        };
}

