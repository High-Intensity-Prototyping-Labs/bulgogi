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
#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include <unordered_map>

// Using declarations
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::unordered_map;

namespace project {
        struct Dependency {
                enum Kind { Target, Module } type;
                string name;

                static Dependency make(Dependency::Kind kind, string name);
        };

        struct Project {
                unordered_map<string, vector<Dependency>> targets;

                static Project make(void);
                static Project load(void);
        };
}
