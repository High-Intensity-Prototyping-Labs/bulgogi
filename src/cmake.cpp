#include "cmake.hpp"

// Standard C++ Libraries 
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <unordered_map>

// Project headers
#include "project.hpp"

// Standard C++ using directives 
using std::string;
using std::vector;
using std::ostream;
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

ostream& cmake::operator<<(ostream& out, CMakeTarget& t) {
        out << "(" << t.name << ", ";
        if(t.kind == CMakeTarget::Executable) {
                out << "CMakeTarget::Executable)";
        } else {
                out << "CMakeTarget::Library)"; 
        }
        return out;
}

CMakeList CMakeList::make() {
        return CMakeList {
                .targets = vector<CMakeTarget>(),
                .links = unordered_map<string, vector<string>>(),
        };
}

ostream& cmake::operator<<(ostream& out, CMakeList& l) {
        std::cout << "CMakeList {" << std::endl;
        std::cout << "\ttargets: {" << std::endl;
        for(auto& target: l.targets) {
                std::cout << "\t\t" << target << "," << std::endl;
        }
        std::cout << "\t}," << std::endl;
        std::cout << "\tlinks: {" << std::endl;
        for(auto& [target, links]: l.links) {
                std::cout << "\t\t" << target<< ": [";
                for(auto& link: links) {
                        std::cout << link << ", ";
                }
                std::cout << "]," << std::endl;
        }
        std::cout << "\t}" << std::endl;
        std::cout << "}" << std::endl;

        return out;
}

CMakeProject CMakeProject::make() {
        return CMakeProject {
                .lists = unordered_map<string, CMakeList>(),
        };
}

CMakeProject CMakeProject::from(project::Project &p) {
        auto targets = vector<CMakeTarget>();
        auto links = unordered_map<string, vector<string>>();
        auto subdirectories = unordered_map<string, vector<CMakeTarget>>();

        for(std::pair<string, vector<project::Dependency>> it: p.targets) {
                auto& [target, dep_list] = it;

                /**
                 * There is a potential conflict here on how targets are determined to be 
                 * libraries or executables.
                 * 
                 * Here the executable marker (*) is only considered when separating links 
                 * from source code. Otherwise it is the dependency of _other_ targets on 
                 * the target in question which determines whether or not is a library.
                 *
                 * Can I conceive of a library which contains an executable main() function?
                 * Maybe...a shared library perhaps? I just don't know how this wouldn't cause 
                 * a conflict down the road in the linker.
                 */

                fs::path subdirectory;
                CMakeTarget new_target;
                vector<string> links_list;
                if(p.any_depends(target, project::Dependency::Target)) {
                        // Create CMake Library target and add every dep as a linkable library
                        new_target = CMakeTarget::from(CMakeTarget::Library, target);
                        for(auto& dep: dep_list) {
                                links_list.push_back(dep.name);
                        }

                        // Target name used for subdirectory
                        subdirectory = target;
                } else {
                        // Create CMake Executable target and add all but executable dep as link.
                        new_target = CMakeTarget::from(CMakeTarget::Executable, target);
                        for(auto& dep: dep_list) {
                                if(dep.exe) {
                                        // Executable dep name used for subdirectory
                                        subdirectory = dep.name;
                                } else {
                                        links_list.push_back(dep.name);
                                }
                        }
                }

                // Add the new CMake target to the list of overall targets
                targets.push_back(new_target);

                // Associate the CMake target to its target link requirements (excludes executable one)
                links.insert({new_target.name, links_list});

                // Combine targets under the same subdirectory (if applicable)
                if(subdirectories.contains(subdirectory)) {
                        subdirectories[subdirectory].push_back(new_target);
                } else {
                        subdirectories.insert({subdirectory, vector{new_target}});
                }
        }

        // DEBUG
        std::cout << "Targets:" << std::endl;
        for(auto& target: targets) {
                std::cout << target << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Links:" << std::endl;
        for(auto& link: links) {
                std::cout << link.first<< ": " << std::endl;
                for(auto& link_list: link.second) {
                        std::cout << "\t" << link_list << ", " << std::endl;
                }
                std::cout << std::endl;
        }
        std::cout << std::endl;

        std::cout << "Subdirectories:" << std::endl;
        for(auto& [subdir, targets]: subdirectories) {
                std::cout << "\t" << subdir << std::endl;
                for(auto& target: targets) {
                        std::cout << "\t\t" << target << "," << std::endl;
                }
                std::cout << std::endl;
        }

        return CMakeProject::make();
}

void CMakeProject::generate() {

}

void CMakeProject::build() {

}
