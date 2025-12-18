// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

// shell_interpreter.hpp - High-level shell interpreter that processes AST
#pragma once

#include "execution_policy.hpp"
#include "ast.hpp"
#include "command_model.hpp"
#include "output_destination.hpp"
#include <map>
#include <string>
#include <expected>

namespace wshell {

// ============================================================================
// Shell Interpreter - Processes AST and manages shell state
// ============================================================================

/// High-level interpreter that processes AST nodes and manages variables
/// Uses Executor<Policy> for actual command execution
template<ExecutionPolicy Policy = PlatformExecutionPolicy>
class ShellInterpreter {
public:
    /// Construct with output destination for messages
    explicit ShellInterpreter(shell::IOutputDestination& output, 
                             shell::IOutputDestination& error_output)
        : output_(output), error_output_(error_output) {}
    
    /// Execute a parsed program (AST)
    [[nodiscard]] int execute_program(const ProgramNode& program) {
        int last_exit_code = platform::EXIT_SUCCESS_STATUS;
        
        for (const auto& statement : program.statements) {
            auto result = execute_statement(statement);
            if (result) {
                last_exit_code = *result;
            } else {
                // ConfigError executing statement
                (void)error_output_.write("ConfigError: " + result.error() + "\n");
                last_exit_code = platform::EXIT_FAILURE_STATUS;
            }
        }
        
        return last_exit_code;
    }
    
    /// Get a variable value
    [[nodiscard]] std::optional<std::string> get_variable(const std::string& name) const {
        auto it = variables_.find(name);
        if (it != variables_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    /// Set a variable value
    void set_variable(const std::string& name, const std::string& value) {
        variables_[name] = value;
    }
    
    /// Get all variables
    [[nodiscard]] const std::map<std::string, std::string>& variables() const noexcept {
        return variables_;
    }
    
    /// Clear all variables
    void clear_variables() {
        variables_.clear();
    }
    
    /// Get reference to underlying executor (for testing)
    [[nodiscard]] Executor<Policy>& executor() noexcept { return executor_; }
    [[nodiscard]] const Executor<Policy>& executor() const noexcept { return executor_; }
    
private:
    Executor<Policy> executor_;
    std::map<std::string, std::string> variables_;
    shell::IOutputDestination& output_;
    shell::IOutputDestination& error_output_;
    
    /// Execute a single statement
    [[nodiscard]] std::expected<int, std::string> execute_statement(const StatementNode& statement) {
        return std::visit([this](const auto& node) -> std::expected<int, std::string> {
            using T = std::decay_t<decltype(node)>;
            
            if constexpr (std::is_same_v<T, std::unique_ptr<CommentNode>>) {
                return execute_comment(*node);
            } else if constexpr (std::is_same_v<T, std::unique_ptr<AssignmentNode>>) {
                return execute_assignment(*node);
            } else if constexpr (std::is_same_v<T, std::unique_ptr<CommandNode>>) {
                return execute_command(*node);
            } else {
                return std::unexpected("Unknown statement type");
            }
        }, statement);
    }
    
    /// Execute a comment (no-op)
    [[nodiscard]] std::expected<int, std::string> execute_comment(const CommentNode& node) {
        // Comments are no-ops
        (void)node;
        return platform::EXIT_SUCCESS_STATUS;
    }
    
    /// Execute an assignment (let var = value)
    [[nodiscard]] std::expected<int, std::string> execute_assignment(const AssignmentNode& node) {
        set_variable(node.variable, node.value);
        return platform::EXIT_SUCCESS_STATUS;
    }
    
    /// Execute a command
    [[nodiscard]] std::expected<int, std::string> execute_command(const CommandNode& node) {
        // Build Command from command_model
        Command cmd;
        cmd.executable = node.command_name;
        cmd.args = node.arguments;
        
        // Execute command
        auto result = executor_.execute(cmd);
        
        if (result.error_message) {
            return std::unexpected(*result.error_message);
        }
        
        return result.exit_code;
    }
};

} // namespace wshell
