#pragma once

// Standard C++ Libraries
#include <string>
#include <vector>

namespace domfarolino {
        void printDirectoryStructure(std::string dir, std::string prefix, std::vector<std::string>& ignore);
        std::vector<std::string> buildIgnoreVector();
}
