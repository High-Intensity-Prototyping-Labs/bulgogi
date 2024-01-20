/**
 * @headerfile project.hpp "project.hpp"
 * @brief Project handler.
 */

#pragma once 

// Project Settings 
#include <optional>
#define PROJECT_YAML "project.yaml"

// Standard C++ Libraries
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>

namespace project {
        struct Dependency {
                enum Kind { Target, Module } type;
                std::string name;

                static Dependency from(Dependency::Kind kind, std::string name);
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

                template<class T>
                T to(void);

                template<>
                std::unordered_map<std::string, std::vector<std::string>> to();

                static Project load(void);
                Err save(void);

                bool contains_module(std::string& m);
                bool contains_module(std::string& m, std::string& t);
        };
        std::ostream& operator<<(std::ostream& out, Project& project);
}
