// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/config.hpp"
#include "shell/parser.hpp"
#include "shell/shell_interpreter.hpp"
#include "version.hpp"
#include <iostream>
#include <span>
#include <string>
#include <shell/output_destination.hpp>

int main(int argc, char* argv[]) {
    
    std::cout << "wshell version " << wshell::version() << "\n";
    
    // Load configuration from ~/.wshellrc
    auto config_path = shell::DefaultConfig::default_config_path();
    if (config_path.empty()) {
        std::cerr << "Warning: Could not determine home directory for config file\n";
    }

    shell::Config config;
    if ( std::filesystem::exists(config_path) ) {
        std::cout << "Loading configuration from " << config_path.string() << "\n";
        shell::FileInputSource * file_source = new shell::FileInputSource(config_path);
        auto config_result = config.loadFromSource(std::unique_ptr<shell::IInputSource>(file_source));
        if (!config_result) {
            std::cerr << "Error loading config: " << config_result.error().message << "\n";
        }
        config.showEnvironmentVariables();
    } else {
        std::cout << "No configuration file found at " << config_path.string() << "\n";
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

        auto prompt = config.get("PS1").value_or("wshell> ");
        
        // Create shell interpreter with platform execution
        wshell::ShellInterpreter<> interpreter(stdout_dest, stderr_dest);

        while (true) {
            // Write prompt - Go-style error handling
            if (auto rc = stdout_dest.write(prompt); !rc) {
                (void)stderr_dest.write("Error writing prompt: " + rc.error() + "\n");
                break;
            }
            
            // Read user input
            auto input = stdin_source.read_line();
            if (!input) {
                (void)stderr_dest.write("Error reading input: " + input.error() + "\n");
                break;  // Exit on read error or EOF
            }

            // Check for exit command
            if (*input == "exit") {
                break;
            }
            
            // Skip empty lines
            if (input->empty() || input->find_first_not_of(" \t") == std::string::npos) {
                continue;
            }
            
            // Parse the input line
            auto parse_result = wshell::parse_line(*input);
            if (!parse_result) {
                (void)stderr_dest.write(parse_result.error().to_string() + "\n");
                continue;  // Continue on parse error
            }
            
            // Execute the parsed program
            int exit_code = interpreter.execute_program(**parse_result);
            
            // Optionally display exit code for non-zero results
            if (exit_code != 0) {
                (void)stderr_dest.write("Command exited with code: " + std::to_string(exit_code) + "\n");
            }
        }
    }
    return EXIT_SUCCESS;
}
