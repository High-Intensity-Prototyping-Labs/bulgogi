#pragma once 

// Standard C++ Libraries
#include <vector>
#include <iostream>
#include <filesystem>
#include <unordered_map>

// Project headers
#include "project.hpp"
#include "inja.hpp"

// Settings
#define TARGET_LIB_DIR "."

namespace cmake {
        struct CMakeTarget {
                enum Kind { Library, Executable } kind;
                std::string name;

                static CMakeTarget from(CMakeTarget::Kind, std::string& name);
        };
        std::ostream& operator<<(std::ostream&, CMakeTarget&);

        using CMakeTargetID = std::string;
        using Subdirectory = std::string;
        struct CMakeList {
                std::vector<CMakeTarget> targets;
                std::unordered_map<CMakeTargetID, std::vector<CMakeTargetID>> links;

                static CMakeList make(void);
                static CMakeList from(std::vector<CMakeTarget>&, std::unordered_map<CMakeTargetID, std::vector<CMakeTargetID>>&);

                template<class T>
                T to(void);

                void generate(Subdirectory&);
        };
        template<>
        inja::json CMakeList::to<inja::json>(void);

        std::ostream& operator<<(std::ostream&, CMakeList&);
        CMakeList operator+(CMakeList&, CMakeList&);
        CMakeList operator+=(CMakeList&, CMakeList&);

        struct CMakeProject {
                std::unordered_map<Subdirectory, CMakeList> lists;

                static CMakeProject make(void);
                static CMakeProject from(project::Project&);
        };
        std::ostream& operator<<(std::ostream&, CMakeProject&);
}
