/**
 * @headerfile client.hpp "client.hpp"
 * @brief Provides handlers for the CLI.
 */

#pragma once

// Project headers
#include "CLI11.hpp"

// Standard C++ Libraries
#include <string>

namespace client {
        void cli(CLI::App&);
}
