#pragma once 

// Standard C++ Libraries
#include <vector>
#include <iostream>
#include <filesystem>
#include <unordered_map>

// Project headers
#include "project.hpp"

namespace cmake {
        struct CMakeTarget {
                enum Kind { Library, Executable } kind;
                std::string name;

                static CMakeTarget from(CMakeTarget::Kind, std::string& name);
        };
        std::ostream& operator<<(std::ostream&, CMakeTarget&);

        struct CMakeList {
                std::vector<CMakeTarget> targets;
                std::unordered_map<std::string, std::vector<std::string>> links;

                static CMakeList make(void);
        };
        std::ostream& operator<<(std::ostream&, CMakeList&);

        struct CMakeProject {
                std::unordered_map<std::string, CMakeList> lists;

                static CMakeProject make(void);
                static CMakeProject from(project::Project&);

                void generate(void);
                void build(void);
        };
}
