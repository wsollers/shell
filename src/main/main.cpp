// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "../lib/shell_core.h"
#include "shell/config.hpp"
#include <iostream>
#include <span>
#include <string>
#include <shell/output_destination.hpp>

int main(int argc, char* argv[]) {
    
    std::cout << "wshell version " << wshell::ShellCore::version() << "\n";
    
    // Load configuration from ~/.wshellrc
    auto config_path = shell::DefaultConfig::default_config_path();
    auto config_result = shell::DefaultConfig::load_from_file(config_path);
    
    if (config_result.has_value()) {
        auto const& vars = config_result->variables();
        std::cout << "Loaded " << vars.size() << " variables from " 
                  << config_path.string() << "\n";
        
        // Optionally display loaded variables
        if (!vars.empty()) {
            std::cout << "Configuration variables:\n";
            for (auto const& [name, value] : vars) {
                std::cout << "  " << name << "=" << value << "\n";
            }
        }
    } else {
        // Config file not found or error - this is OK, just inform the user
        if (config_result.error().code != shell::DefaultConfig::ErrorCode::SOURCE_READ_ERROR) {
            std::cerr << "Warning: Configuration error: " 
                      << config_result.error().message << "\n";
        }
    }
    
    // Use std::span for modern C++ argument processing
    std::span<char*> args(argv, static_cast<std::size_t>(argc));

    // Process command line arguments using span (skip program name at args[0])
    auto command_args = args.subspan(1);
    
    if (!command_args.empty()) {
        //parse args and set flags
    } else {

        shell::StreamInputSource stdin_source(std::cin, "stdin");
        shell::StreamOutputDestination stdout_dest(std::cout, "stdout");
        shell::StreamOutputDestination stderr_dest(std::cerr, "stderr");

        auto prompt = config_result->get("PS1").value_or("wshell> ");

        while (true) {
            // Write prompt
            auto rc = stdout_dest.write(prompt)
                .or_else([&stderr_dest](std::string const& err) -> std::expected<void, std::string> {
                    (void)stderr_dest.write("Error writing prompt: " + err + "\n");
                    return std::unexpected(err);
                });

            if (!rc) {
                break;
            }
            // Read user input (line by line for interactive mode)
            auto input = stdin_source.read_line()
                .or_else([&stderr_dest](std::string const& err) -> std::expected<std::string, std::string> {
                    (void)stderr_dest.write("Error reading input: " + err + "\n");
                    return std::unexpected(err);
                });

            if (!input) {
                break;  // Exit on read error or EOF
            }

            if (*input == "exit") {
                break;  // TODO: Use parser instead of string comparison
            }

            // Echo back what was entered
            auto rrc = stdout_dest.write("You entered: " + *input + "\n")
                .or_else([&stderr_dest](std::string const& err) -> std::expected<void, std::string> {
                    (void)stderr_dest.write("Error writing output: " + err + "\n");
                    return std::unexpected(err);
            });

            if (!rrc) {
                break;
            }
        }
    }
}
