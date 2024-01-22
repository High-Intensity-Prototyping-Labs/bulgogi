#include "cmake.hpp"

// Standard C++ Libraries 
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <unordered_map>

// External Libraries 
#include "inja.hpp"

// Project headers
#include "project.hpp"

// Standard C++ using directives 
using std::string;
using std::vector;
using std::ostream;
using std::unordered_map;

// External library using directives 
using inja::json;

// Project using directives
using cmake::CMakeList;
using cmake::CMakeTarget;
using cmake::CMakeProject;
using cmake::Subdirectory;
using cmake::CMakeTargetID;

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

CMakeList CMakeList::from(std::vector<CMakeTarget>& targets, std::unordered_map<CMakeTargetID, std::vector<CMakeTargetID>>& links) {
        return CMakeList {
                .targets = vector<CMakeTarget>(targets),
                .links = unordered_map<CMakeTargetID, vector<CMakeTargetID>>(links),
        };
}

void CMakeList::generate(Subdirectory& subdir) {
        (void)subdir;
        // Load template 
        
        // For each target, populate template 
        
        // For each links, populate template
        
        // Make sure subdir exists 
        
        // Write CMakeList.txt out
}

template<>
json CMakeList::to<json>() {
        auto j = json();
        for(auto& t: targets) {
                // Set executable or not
                if(t.kind == CMakeTarget::Executable) {
                        j["targets"][t.name]["exe"] = true;
                } else {
                        j["targets"][t.name]["exe"] = false;
                }

                // Set links
                j["targets"][t.name]["links"] = this->links[t.name];
        }

        return j;
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

CMakeList cmake::operator+(CMakeList& a, CMakeList& b) {
        auto list = CMakeList(a);
        list.targets.insert(list.targets.end(), b.targets.begin(), b.targets.end());
        list.links.insert(b.links.begin(), b.links.end());
        return list;
}

CMakeList cmake::operator+=(CMakeList& a, CMakeList& b) {
        return a + b;
}

CMakeProject CMakeProject::make() {
        return CMakeProject {
                .lists = unordered_map<string, CMakeList>(),
        };
}

CMakeProject CMakeProject::from(project::Project &p) {
        auto project = CMakeProject::make();

        auto targets = vector<CMakeTarget>();
        auto links = unordered_map<CMakeTargetID, vector<string>>();
        auto subdirectories = unordered_map<Subdirectory, vector<CMakeTarget>>();
        auto libmodules = vector<string>();

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
                        subdirectory = TARGET_LIB_DIR;
                        // (target libraries have no subdirectory (all in parent CMakeList.txt)
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

                // Add libmodules from target dependencies
                for(auto& dep: dep_list) {
                        if(dep.type == project::Dependency::Module 
                                && dep.exe == false
                                && !std::count(libmodules.begin(), libmodules.end(), dep.name)
                        ) {
                                libmodules.push_back(dep.name);
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

        // Add libmodules to the subdirectories 
        for(auto& libmodule: libmodules) {
                subdirectories.insert({
                        libmodule, 
                        vector{
                                CMakeTarget::from(CMakeTarget::Library, libmodule),
                        },
                });
        }

        // Amalgamate all of those lists 
        for(auto& [subdir, targets]: subdirectories) {
                auto list = CMakeList::make();
                for(auto& target: targets) {
                        list.targets.push_back(target);
                        list.links.insert({target.name, links[target.name]});
                        /* assumes every target.name has an entry in links */
                }
                // Catch/prevent any overrides of 2 entries with the same subdirectory
                if(project.lists.contains(subdir)) {
                        // Combine lists
                        project.lists[subdir] += list;
                } else {
                        project.lists.insert({subdir, list});
                }
        }

        return project;
}

// TODO: Make me prettier
ostream& cmake::operator<<(ostream& out, CMakeProject& p) {
        out << "Project {" << std::endl;
        out << "\tlists: {" << std::endl;
        for(auto& [subdir, list]: p.lists) {
                out << "\t\t\t" << subdir << ": " << std::endl;
                out << list << std::endl;
        }
        out << "\t}" << std::endl;
        out << "}" << std::endl;

        return out;
}
