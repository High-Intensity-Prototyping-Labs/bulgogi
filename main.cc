/**
 * @file main.cc
 * @brief Main source file for Bulgogi.
 */

// Project headers
#include "client.hpp"

// Dependencies
#include "CLI11.hpp"

int main(int argc, char *argv[]) {
        CLI::App app{"Bulgogi - A build system assistant."};

        client::cli(app);

        CLI11_PARSE(app, argc, argv);

        return 0;
}
