#include "cmake.hpp"

// Standard C++ Libraries 
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>

// Standard C++ using directives 
using std::string;
using std::vector;
using std::unordered_map;

// Project using directives
using cmake::CMakeList;
using cmake::CMakeTarget;
using cmake::CMakeProject;

// Namespace aliases 
namespace fs = std::filesystem;

CMakeTarget CMakeTarget::from(CMakeTarget::Kind kind, std::string &name) {
        return CMakeTarget {
                .kind = kind,
                .name = string(name),
        };
}

CMakeList CMakeList::make() {
        return CMakeList {
                .targets = vector<CMakeTarget>(),
                .links = unordered_map<CMakeTarget, vector<string>>(),
        };
}

CMakeProject CMakeProject::make() {
        return CMakeProject {
                .lists = unordered_map<fs::path, CMakeList>(),
        };
}

CMakeProject CMakeProject::from(project::Project &p) {
        return CMakeProject::make();
}

void CMakeProject::generate() {

}

void CMakeProject::build() {

}
