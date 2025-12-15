// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "../lib/shell_core.h"
#include "shell/config.hpp"
#include <iostream>
#include <span>
#include <string>

int main(int argc, char* argv[]) {
    // Use std::span for modern C++ argument processing
    std::span<char*> args(argv, static_cast<std::size_t>(argc));
    
    std::cout << "wshell version " << wshell::ShellCore::version() << "\n";
    
    // Load configuration from ~/.wshellrc
    auto config_path = shell::Config::default_config_path();
    auto config_result = shell::Config::load_from_file(config_path);
    
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
        if (config_result.error().code != shell::Config::ErrorCode::FILE_NOT_FOUND) {
            std::cerr << "Warning: Configuration error: " 
                      << config_result.error().message << "\n";
        }
    }
    
    // Process command line arguments using span (skip program name at args[0])
    auto command_args = args.subspan(1);
    
    if (!command_args.empty()) {
        std::string command;
        for (std::size_t i = 0; i < command_args.size(); ++i) {
            if (i > 0) command += " ";
            command += command_args[i];
        }
        
        wshell::ShellCore shell;
        auto result = shell.execute(command);
        
        if (result.has_value()) {
            return result.value();
        } else {
            std::cerr << "Error executing command\n";
            return 1;
        }
    } else {
        std::cout << "Interactive mode not yet implemented\n";
        std::cout << "Usage: wshell <command>\n";
        return 0;
    }
}
