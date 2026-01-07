// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

// Example: Using InputSource classes for interactive shell input
// Demonstrates how the refactored input sources can be used beyond config files

#include "shell/input_source.hpp"

#include <iostream>
#include <sstream>

/// @brief Example: Read commands from stdin using StreamInputSource
void example_interactive_shell() {
    std::cout << "=== Interactive Shell Example ===\n";
    std::cout << "Type commands (or 'exit' to quit):\n\n";

    // Wrap stdin in a StreamInputSource
    wshell::StreamInputSource stdin_source(std::cin, "stdin");

    while (true) {
        std::cout << "wshell> ";
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line)) {
            break;  // EOF or error
        }

        if (line == "exit") {
            break;
        }

        // Process the command (placeholder)
        std::cout << "Would execute: " << line << "\n";
    }
}

/// @brief Example: Batch process commands from a file
void example_batch_processing(std::string const& script_path) {
    std::cout << "\n=== Batch Processing Example ===\n";

    // Use FileInputSource to read a script file
    wshell::FileInputSource script_source(script_path);

    auto content = script_source.read();
    if (!content) {
        std::cerr << "Error reading script: " << content.error() << "\n";
        return;
    }

    // Process commands line by line
    std::istringstream lines(*content);
    std::string line;
    int line_num = 0;

    while (std::getline(lines, line)) {
        ++line_num;

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::cout << "[" << line_num << "] Executing: " << line << "\n";
        // Execute command here
    }
}

/// @brief Example: Test shell commands with fake input
void example_testing_with_string_source() {
    std::cout << "\n=== Testing Example ===\n";

    // Simulate user input for testing
    wshell::StringInputSource test_input("ls -la\n"
                                         "cd /tmp\n"
                                         "echo 'Hello, World!'\n"
                                         "exit\n",
                                         "test_commands");

    auto commands = test_input.read();
    if (commands) {
        std::cout << "Processing test commands:\n";
        std::istringstream lines(*commands);
        std::string line;

        while (std::getline(lines, line)) {
            if (!line.empty()) {
                std::cout << "  > " << line << "\n";
            }
        }
    }
}

/// @brief Example: Polymorphic input handling
void process_input_source(shell::IInputSource& source) {
    std::cout << "\n=== Processing input from: " << source.source_name() << " ===\n";

    auto content = source.read();
    if (!content) {
        std::cerr << "Error: " << content.error() << "\n";
        return;
    }

    std::cout << "Read " << content->size() << " bytes\n";
    std::cout << "Content:\n" << *content << "\n";
}

int main() {
    std::cout << "Input Source Examples for Shell Usage\n";
    std::cout << "=====================================\n\n";

    // Example 1: Testing with StringInputSource
    example_testing_with_string_source();

    // Example 2: Polymorphic processing
    wshell::StringInputSource str_src("echo 'test'", "inline_command");
    process_input_source(str_src);

    // Example 3: Stream from stringstream (simulating stdin)
    std::istringstream fake_input("ls\npwd\nexit\n");
    wshell::StreamInputSource stream_src(fake_input, "fake_stdin");
    process_input_source(stream_src);

    std::cout << "\nAll examples completed!\n";
    std::cout << "\nNote: Uncomment example_interactive_shell() to try interactive mode\n";
    // example_interactive_shell();  // Uncomment for interactive demo

    return 0;
}
