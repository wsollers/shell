// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

// Example: Using Dependency Injection with Config Class
// This demonstrates how to use fake/mock sources for testing

#include "shell/config.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

// Example 1: Create a fake input source for testing
class FakeInputSource : public wshell::IInputSource {
  public:
    explicit FakeInputSource(std::string content, bool should_fail = false)
        : content_(std::move(content)), should_fail_(should_fail) {}

    std::expected<std::string, std::string> read() override {
        if (should_fail_) {
            return std::unexpected("Simulated read failure");
        }
        return content_;
    }

    std::string source_name() const override { return "fake_source"; }

  private:
    std::string content_;
    bool should_fail_;
};

// Example 2: Using the built-in StringConfigSource
void example_string_source() {
    std::cout << "=== Example: String Input Source ===\n";

    auto source = std::make_unique<shell::StringInputSource>(
        "VAR1=value1\nVAR2=value2\n# Comment\nVAR3=value3");

    auto config = wshell::DefaultConfig::load_from_source(std::move(source));

    if (config) {
        std::cout << "Loaded " << config->variables().size() << " variables\n";
        for (auto const& [name, value] : config->variables()) {
            std::cout << "  " << name << " = " << value << "\n";
        }
    }
}

// Example 3: Using the built-in StreamConfigSource
void example_stream_source() {
    std::cout << "\n=== Example: Stream Input Source ===\n";

    std::istringstream stream("DEBUG=true\nLOG_LEVEL=verbose\n");
    wshell::StreamInputSource source(stream, "test_stream");

    auto config = wshell::DefaultConfig::load_from_source(source);

    if (config) {
        if (auto val = config->get("DEBUG")) {
            std::cout << "DEBUG = " << *val << "\n";
        }
        if (auto val = config->get("LOG_LEVEL")) {
            std::cout << "LOG_LEVEL = " << *val << "\n";
        }
    }
}

// Example 4: Using custom fake source
void example_fake_source() {
    std::cout << "\n=== Example: Custom Fake Source ===\n";

    // Test successful read
    auto source = std::make_unique<FakeInputSource>("TEST=fake_value");
    auto config = wshell::DefaultConfig::load_from_source(std::move(source));

    if (config) {
        std::cout << "Success: " << config->get("TEST").value() << "\n";
    }

    // Test failed read
    source = std::make_unique<FakeInputSource>("", true);
    config = wshell::DefaultConfig::load_from_source(std::move(source));

    if (!config) {
        std::cout << "Expected failure: " << config.error().message << "\n";
    }
}

// Example 5: Using different validation policies
void example_validation_policies() {
    std::cout << "\n=== Example: Validation Policies ===\n";

    // Default policy (generous limits)
    auto default_config = wshell::DefaultConfig::parse("VAR=value");
    std::cout << "Default policy: " << (default_config ? "SUCCESS" : "FAIL") << "\n";

    // Strict policy (tighter limits)
    auto strict_config = wshell::StrictConfig::parse("VAR=value");
    std::cout << "Strict policy: " << (strict_config ? "SUCCESS" : "FAIL") << "\n";
}

// Example 6: Memory-safe API (no dangling pointers)
void example_memory_safety() {
    std::cout << "\n=== Example: Memory-Safe API ===\n";

    wshell::DefaultConfig config;
    config.set("TEST", "value");

    // Old API (UNSAFE - could dangle):
    // auto* ptr = config.get("TEST");  // Returns raw pointer

    // New API (SAFE - returns copy or optional):
    auto value = config.get("TEST");  // Returns std::optional<std::string>
    if (value) {
        std::cout << "Safe value: " << *value << "\n";
    }

    // Alternative: get_view for efficiency (but document lifetime)
    auto view = config.get_view("TEST");  // Returns std::optional<std::string_view>
    if (view) {
        std::cout << "Safe view: " << *view << "\n";
    }
}

// Example 7: Error handling with std::expected and source_location
void example_error_handling() {
    std::cout << "\n=== Example: Error Handling with Source Location ===\n";

    wshell::DefaultConfig config;

    // set() now returns std::expected
    auto result = config.set("VALID_VAR", "value");
    if (result) {
        std::cout << "Set succeeded\n";
    }

    // Invalid variable name
    result = config.set("123invalid", "value");
    if (!result) {
        auto const& err = result.error();
        std::cout << "Set failed: " << err.message << "\n";
        std::cout << "Error code: " << static_cast<int>(err.code) << "\n";

        // C++23 source_location provides detailed debugging info
        std::cout << "Error occurred at:\n";
        std::cout << "  File: " << err.location.file_name() << "\n";
        std::cout << "  Line: " << err.location.line() << "\n";
        std::cout << "  Column: " << err.location.column() << "\n";
        std::cout << "  Function: " << err.location.function_name() << "\n";
    }
}

int main() {
    std::cout << "Config Class Dependency Injection Examples\n";
    std::cout << "==========================================\n\n";

    example_string_source();
    example_stream_source();
    example_fake_source();
    example_validation_policies();
    example_memory_safety();
    example_error_handling();

    std::cout << "\nAll examples completed successfully!\n";
    return 0;
}
