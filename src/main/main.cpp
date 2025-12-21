// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include <iostream>
#include <span>
#include <string>

#include "shell/config.hpp"
#include "shell/parser.hpp"
#include "shell/shell_interpreter.hpp"
#include "version.hpp"
#include <shell/output_destination.hpp>

int main(int argc, char* argv[]) {
    
    std::cout << "wshell version " << wshell::version() << "\n";
    
    // Load configuration from ~/.wshellrc
    auto config_path = wshell::DefaultConfig::default_config_path();
    if (config_path.empty()) {
        std::cerr << "Warning: Could not determine home directory for config file\n";
    }

    wshell::Config config;
    if ( std::filesystem::exists(config_path) ) {
        std::cout << "Loading configuration from " << config_path.string() << "\n";
        wshell::FileInputSource * file_source = new wshell::FileInputSource(config_path);
        auto config_result = config.loadFromSource(std::unique_ptr<wshell::IInputSource>(file_source));
        if (!config_result) {
            std::cerr << "Error loading config: " << config_result.error().message << "\n";
        }
        config.showEnvironmentVariables();
    } else {
        std::cout << "No configuration file found at " << config_path.string() << "\n";
    }

    std::span<char*> args(argv, static_cast<std::size_t>(argc));
    //parse args and set flags
    auto command_args = args.subspan(1);

    if (!command_args.empty()) {
        //parse args and set flags
    } else {

        wshell::StreamInputSource stdin_source(std::cin, "stdin");
        wshell::StreamOutputDestination stdout_dest(std::cout, "stdout");
        wshell::StreamOutputDestination stderr_dest(std::cerr, "stderr");

        auto prompt = config.get("PS1").value_or("wshell> ");
        auto cont_prompt = config.get("PS2").value_or("> ");

        wshell::ShellInterpreter<wshell::PlatformExecutionPolicy> interpreter(stdout_dest, stderr_dest);

        while (true) {

            std::string full_input;

            // --- First prompt (PS1) ---
            if (auto rc = stdout_dest.write(prompt); !rc) {
                (void)stderr_dest.write("Error writing prompt: " + rc.error() + "\n");
                break;
            }

            // Read first line
            auto input = stdin_source.read_line();
            if (!input) {
                (void)stderr_dest.write("Error reading input: " + input.error() + "\n");
                break;
            }

            // Exit command
            if (*input == "exit") {
                break;
            }

            full_input = *input;

            // Skip empty lines
            if (full_input.find_first_not_of(" \t") == std::string::npos) {
                continue;
            }

            // --- Try parsing the line ---
            auto parse_result = wshell::parse_line(full_input);

            // --- Handle continuation ---
            while (!parse_result &&
                   parse_result.error().kind_ == wshell::ParseErrorKind::IncompleteInput)
            {
                // Print continuation prompt (PS2)
                if (auto rc = stdout_dest.write(cont_prompt); !rc) {
                    (void)stderr_dest.write("Error writing prompt: " + rc.error() + "\n");
                    break;
                }

                auto next_line = stdin_source.read_line();
                if (!next_line) {
                    (void)stderr_dest.write("Error reading input: " + next_line.error() + "\n");
                    break;
                }

                // OutputAppend with newline (important for multi-line constructs)
                full_input += "\n";
                full_input += *next_line;

                // Try parsing again
                parse_result = wshell::parse_line(full_input);
            }

            // If still an error (but not incomplete), print it
            if (!parse_result) {
                (void)stderr_dest.write(parse_result.error().to_string() + "\n");
                continue;
            }

            // --- Execute the parsed program ---
            int exit_code = interpreter.execute_program(**parse_result);

            if (exit_code != 0) {
                (void)stderr_dest.write("Command exited with code: " + std::to_string(exit_code) + "\n");
            }
        }
    }
    return EXIT_SUCCESS;
}
