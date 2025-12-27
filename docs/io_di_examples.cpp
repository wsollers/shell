// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

// Example: Using InputSource and OutputDestination together for testable I/O
// Demonstrates dependency injection for complete I/O control

#include "shell/input_source.hpp"
#include "shell/output_destination.hpp"

#include <iostream>
#include <sstream>

namespace shell {

/// @brief Simple shell that uses DI for input and output
class TestableShell {
  public:
    TestableShell(IInputSource& input, IOutputDestination& output, IOutputDestination& error)
        : input_(input), output_(output), error_(error) {}

    /// @brief Run the shell: read commands and execute them
    void run() {
        auto content = input_.read();
        if (!content) {
            error_.write("Error reading input: " + content.error() + "\n");
            return;
        }

        output_.write("Welcome to testable shell!\n");
        output_.write("Processing commands from: " + input_.source_name() + "\n\n");

        std::istringstream lines(*content);
        std::string line;
        int line_num = 0;

        while (std::getline(lines, line)) {
            ++line_num;

            if (line.empty() || line[0] == '#') {
                continue;  // Skip empty lines and comments
            }

            if (line == "exit") {
                output_.write("Exiting shell.\n");
                break;
            }

            // Simulate command execution
            output_.write("[" + std::to_string(line_num) + "] Executing: " + line + "\n");

            // Simulate some commands
            if (line.find("error") != std::string::npos) {
                error_.write("ERROR: Command failed: " + line + "\n");
            } else {
                output_.write("  -> Success\n");
            }
        }

        output_.flush();
        error_.flush();
    }

  private:
    IInputSource& input_;
    IOutputDestination& output_;
    IOutputDestination& error_;
};

}  // namespace shell

//==============================================================================
// Examples
//==============================================================================

/// @brief Example 1: Production usage with real stdin/stdout
void example_production() {
    std::cout << "=== Production Example ===\n";
    std::cout << "Type commands (or 'exit' to quit):\n";

    wshell::StreamInputSource input(std::cin, "stdin");
    wshell::StreamOutputDestination output(std::cout, "stdout");
    wshell::StreamOutputDestination error(std::cerr, "stderr");

    wshell::TestableShell shell(input, output, error);
    // shell.run();  // Uncomment for interactive demo
}

/// @brief Example 2: Testing with fake I/O
void example_testing() {
    std::cout << "\n=== Testing Example ===\n";

    // Setup fake input
    wshell::StringInputSource input("# Test script\n"
                                    "ls -la\n"
                                    "cd /tmp\n"
                                    "error command\n"
                                    "echo 'success'\n"
                                    "exit\n",
                                    "test_script");

    // Capture output and errors
    wshell::StringOutputDestination output("test_output");
    wshell::StringOutputDestination error("test_error");

    // Run the shell
    wshell::TestableShell shell(input, output, error);
    shell.run();

    // Verify output
    std::cout << "Captured Output:\n";
    std::cout << output.captured_output() << "\n";

    std::cout << "Captured Errors:\n";
    std::cout << error.captured_output() << "\n";
}

/// @brief Example 3: Logging to file while displaying to console
void example_logging() {
    std::cout << "\n=== Logging Example ===\n";

    // Input from string (simulating user commands)
    wshell::StringInputSource input("command1\n"
                                    "command2\n"
                                    "error command3\n"
                                    "exit\n",
                                    "commands");

    // Output to both console and file
    wshell::StreamOutputDestination console_out(std::cout, "console");
    wshell::FileOutputDestination file_out("/tmp/shell_output.log",
                                           wshell::FileOutputDestination::Mode::Truncate);

    wshell::StreamOutputDestination error_out(std::cerr, "stderr");

    // For this example, just use console
    wshell::TestableShell shell(input, console_out, error_out);
    shell.run();

    std::cout << "\nOutput also logged to: /tmp/shell_output.log\n";
}

/// @brief Example 4: Unit testing helper
class ShellTester {
  public:
    void run_test(std::string const& commands, std::string const& expected_output,
                  std::string const& expected_error) {
        wshell::StringInputSource input(commands, "test");
        wshell::StringOutputDestination output("output");
        wshell::StringOutputDestination error("error");

        wshell::TestableShell shell(input, output, error);
        shell.run();

        std::cout << "\n=== Test Results ===\n";
        std::cout << "Expected output: " << expected_output << "\n";
        std::cout << "Actual output contains: "
                  << (output.captured_output().find(expected_output) != std::string::npos ? "YES ✓"
                                                                                          : "NO ✗")
                  << "\n";

        std::cout << "Expected error: " << expected_error << "\n";
        std::cout << "Actual error contains: "
                  << (error.captured_output().find(expected_error) != std::string::npos ? "YES ✓"
                                                                                        : "NO ✗")
                  << "\n";
    }
};

void example_unit_testing() {
    std::cout << "\n=== Unit Testing Example ===\n";

    ShellTester tester;

    // Test successful command
    tester.run_test("ls\nexit\n", "Executing: ls", "");

    // Test error command
    tester.run_test("error test\nexit\n", "Executing: error test", "ERROR: Command failed");
}

int main() {
    std::cout << "Input/Output Dependency Injection Examples\n";
    std::cout << "==========================================\n";

    example_testing();
    example_unit_testing();

    std::cout << "\nNote: Uncomment example_production() for interactive mode\n";
    // example_production();  // Uncomment for interactive demo

    return 0;
}
