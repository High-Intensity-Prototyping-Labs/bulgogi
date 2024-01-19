/**
 * @headerfile project.hpp "project.hpp"
 * @brief Project handler.
 */

#pragma once 

// Project Settings 
#define PROJECT_YAML "project.yaml"

// External Dependencies 
#include "yaml-cpp/yaml.h"

// Standard C++ Libraries
#include <tuple>
#include <string>
#include <vector>
#include <variant>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

// Using declarations
using std::ios;
using std::string;
using std::vector;
using std::ofstream;
using std::unordered_map;

namespace project {
        using yaml_map = std::unordered_map<string, vector<string>>;

        struct Dependency {
                enum Kind { Target, Module } type;
                string name;

                static Dependency from(Dependency::Kind kind, string name);
        };
        std::ostream& operator<<(std::ostream& out, Dependency& dep);
        bool operator==(const Dependency& a, Dependency& b);

        struct Project {
                unordered_map<string, vector<Dependency>> targets;

                static Project make(void);
                static Project from(unordered_map<string, vector<string>>);

                template<class T>
                T to(void);
                template<>
                unordered_map<string, vector<string>> to();

                static Project load(void);
                void save(void);

                bool contains_module(string& m);
                bool contains_module(string& m, string& t);
        };
        std::ostream& operator<<(std::ostream& out, Project& project);
}
