// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

// execution_policy.hpp - Policy-based command execution with zero-cost abstractions
#pragma once

#include "command_model.hpp"
#include <concepts>
#include <vector>
#include <optional>
#include <string>

namespace wshell {

// ============================================================================
// Execution Result
// ============================================================================

/// Result of command execution
struct ExecutionResult {
    int error_code{0};
    int exit_code{0};
    std::optional<std::string> error_message;
    
    [[nodiscard]] bool is_success() const noexcept {
        return exit_code == platform::EXIT_SUCCESS_STATUS && !error_message.has_value();
    }
    
    [[nodiscard]] bool is_failure() const noexcept {
        return !is_success();
    }
};

// ============================================================================
// Execution Policy Concept
// ============================================================================

/// Concept defining the execution policy interface
/// Policies must implement execute() methods for Command and Pipeline
template<typename T>
concept ExecutionPolicy = requires(T policy, const Command& cmd, const Pipeline& pipeline) {
    { policy.execute(cmd) } -> std::same_as<ExecutionResult>;
    { policy.execute(pipeline) } -> std::same_as<ExecutionResult>;
};

// ============================================================================
// Platform Execution Policy (Real Implementation)
// ============================================================================

/// Platform-specific execution policy
/// Implementation is in exec_posix.cpp (Linux/macOS) or exec_win32.cpp (Windows)
struct PlatformExecutionPolicy {

    /// Execute a single command
    ExecutionResult execute(const Command& cmd) const;
    
    /// Execute a pipeline
    ExecutionResult execute(const Pipeline& pipeline) const;
    
    /// Initialize job control (for interactive shells)
    void init_job_control() const;
};

// ============================================================================
// Fake Execution Policy (For Testing)
// ============================================================================

/// Fake execution policy for unit testing
/// Records all executed commands without actually running them
struct FakeExecutionPolicy {
    // Track executed commands
    mutable std::vector<Command> executed_commands;
    mutable std::vector<Pipeline> executed_pipelines;
    
    // Configurable return values
    int return_code{platform::EXIT_SUCCESS_STATUS};
    std::optional<std::string> error_message;
    
    /// Execute a command (records it but doesn't actually execute)
    ExecutionResult execute(const Command& cmd) const {
        executed_commands.push_back(cmd);
        return ExecutionResult{
            .exit_code = return_code,
            .error_message = error_message
        };
    }
    
    /// Execute a pipeline (records it but doesn't actually execute)
    ExecutionResult execute(const Pipeline& pipeline) const {
        executed_pipelines.push_back(pipeline);
        // Record individual commands from pipeline
        for (const auto& cmd : pipeline.commands) {
            executed_commands.push_back(cmd);
        }
        return ExecutionResult{
            .exit_code = return_code,
            .error_message = error_message
        };
    }
    
    /// Initialize job control (no-op for fake)
    void init_job_control() const {
        // No-op
    }
    
    /// Reset tracking state (useful between tests)
    void reset() {
        executed_commands.clear();
        executed_pipelines.clear();
        return_code = platform::EXIT_SUCCESS_STATUS;
        error_message.reset();
    }
    
    /// Set the return code for subsequent executions
    void set_return_code(int code) {
        return_code = code;
    }
    
    /// Set an error message for subsequent executions
    void set_error(std::string msg) {
        error_message = std::move(msg);
        if (return_code == platform::EXIT_SUCCESS_STATUS) {
            return_code = platform::EXIT_FAILURE_STATUS;
        }
    }
    
    /// Clear error state
    void clear_error() {
        error_message.reset();
        return_code = platform::EXIT_SUCCESS_STATUS;
    }
    
    /// Get number of executed commands
    [[nodiscard]] std::size_t command_count() const noexcept {
        return executed_commands.size();
    }
    
    /// Get number of executed pipelines
    [[nodiscard]] std::size_t pipeline_count() const noexcept {
        return executed_pipelines.size();
    }
    
    /// Check if a specific command was executed
    [[nodiscard]] bool was_executed(const std::string& command_name) const {
        for (const auto& cmd : executed_commands) {
            if (cmd.executable.filename() == command_name) {
                return true;
            }
        }
        return false;
    }
};

// Static assertion to verify FakeExecutionPolicy meets the concept
static_assert(ExecutionPolicy<FakeExecutionPolicy>, 
              "FakeExecutionPolicy must satisfy ExecutionPolicy concept");

// ============================================================================
// Executor Template
// ============================================================================

/// Policy-based executor
/// Uses compile-time polymorphism (zero-cost abstraction)
template<ExecutionPolicy Policy = PlatformExecutionPolicy>
class Executor {
public:
    /// Construct with default policy
    Executor() = default;
    
    /// Construct with specific policy instance
    explicit Executor(Policy policy) : policy_(std::move(policy)) {}
    
    /// Execute a single command
    ExecutionResult execute(const Command& cmd) {
        return policy_.execute(cmd);
    }
    
    /// Execute a pipeline
    ExecutionResult execute(const Pipeline& pipeline) {
        return policy_.execute(pipeline);
    }
    
    /// Execute a job (Command or Pipeline variant)
    ExecutionResult execute(const Job& job) {
        return std::visit([this](const auto& item) {
            return this->execute(item);
        }, job);
    }
    
    /// Initialize job control (for interactive shells)
    void init_job_control() {
        if constexpr (requires { policy_.init_job_control(); }) {
            policy_.init_job_control();
        }
    }
    
    /// Get reference to the underlying policy (for testing/configuration)
    [[nodiscard]] Policy& policy() noexcept { return policy_; }
    [[nodiscard]] const Policy& policy() const noexcept { return policy_; }
    
private:
    Policy policy_;
};

// ============================================================================
// Type Aliases
// ============================================================================

/// Real executor using platform-specific implementation
using PlatformExecutor = Executor<PlatformExecutionPolicy>;

/// Fake executor for unit testing
using FakeExecutor = Executor<FakeExecutionPolicy>;

} // namespace wshell
