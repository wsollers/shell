// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

// shell_interpreter.hpp - High-level shell interpreter that processes AST
#pragma once

#include <expected>
#include <optional>

#include <iostream>
#include <map>
#include <string>
#include <utility>

#include "ast.hpp"
#include "built_ins.hpp"
#include "command_model.hpp"
#include "execution_policy.hpp"
#include "history.hpp"
#include "output_destination.hpp"

namespace wshell {

template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// ============================================================================
// Shell Interpreter - Processes AST and manages shell state
// ============================================================================

/// High-level interpreter that processes AST nodes and manages variables.
/// Uses Executor<Policy> for actual command execution.
template<ExecutionPolicy Policy = PlatformExecutionPolicy>
class ShellInterpreter {
public:
    /// Construct with output destination for messages
    explicit ShellInterpreter(wshell::IOutputDestination& output,
                              wshell::IOutputDestination& error_output)
        : executor_{},
          variables_{},
          output_(output),
          error_output_(error_output), builtins_{}, history_{} {}

    /// Execute a parsed program (AST)
    [[nodiscard]] int execute_program(const ProgramNode& program) {
        int last_exit_code = platform::EXIT_SUCCESS_STATUS;

        for (const auto& statement : program.statements) {
            auto result = execute_statement(statement);
            if (result) {
                last_exit_code = *result;
            } else {
                (void)error_output_.write("ConfigError: " + result.error() + "\n");
                last_exit_code = platform::EXIT_FAILURE_STATUS;
            }
        }

        return last_exit_code;
    }

    /// Get a variable value
    [[nodiscard]] std::optional<std::string>
    get_variable(const std::string& name) const {
        auto it = variables_.find(name);
        if (it != variables_.end())
            return it->second;
        return std::nullopt;
    }

    /// Set a variable value
    void set_variable(const std::string& name, const std::string& value) {
        variables_[name] = value;
    }

    /// Get all variables
    [[nodiscard]] const std::map<std::string, std::string>&
    variables() const noexcept {
        return variables_;
    }

    /// Clear all variables
    void clear_variables() {
        variables_.clear();
    }

    /// Get reference to underlying executor (for testing)
    [[nodiscard]] Executor<Policy>& executor() noexcept { return executor_; }
    [[nodiscard]] const Executor<Policy>& executor() const noexcept { return executor_; }

    void addToHisttory(std::string command) {
        history_.push(command);
    };

private:
    Executor<Policy> executor_;
    std::map<std::string, std::string> variables_;
    wshell::IOutputDestination& output_;
    wshell::IOutputDestination& error_output_;
    wshell::BuiltIns builtins_;
    wshell::History history_;


    [[nodiscard]] std::string replaceVariables(const std::string& line) {
        //Find occurences of ${} in line and replace with the
        //varable if possible. if not found replace with ""

        //TODO replace variables
        return line;
    }

    /// Execute a single statement
    [[nodiscard]] std::expected<int, std::string>
    execute_statement(const StatementNode& statement) {
        return std::visit(overloaded{

            [&](const CommentNode& node) -> std::expected<int, std::string> {
                return execute_comment(node);
            },

            [&](const AssignmentNode& node) -> std::expected<int, std::string> {
                return execute_assignment(node);
            },

            [&](const CommandNode& node) -> std::expected<int, std::string> {
                return execute_command(node);
            },

            [&](const PipelineNode& node) -> std::expected<int, std::string> {
                return execute_pipeline(node);
            },

            [&](const SequenceNode& node) -> std::expected<int, std::string> {
                return execute_sequence(node);
            }

        }, statement);
    }

    /// Execute a comment (no-op)
    [[nodiscard]] std::expected<int, std::string>
    execute_comment(const CommentNode& node) {
        (void)node; // comments are no-ops
        return platform::EXIT_SUCCESS_STATUS;
    }

    /// Execute an assignment (let var = value)
    [[nodiscard]] std::expected<int, std::string>
    execute_assignment(const AssignmentNode& node) {
        set_variable(node.variable, node.value);
        return platform::EXIT_SUCCESS_STATUS;
    }

    /*
    struct Command {
    std::filesystem::path executable;                 // absolute or relative; resolution is exec-layer policy
    std::optional<std::filesystem::path> work_dir;    // nullopt = inherit current working directory

    // argv[0] policy: keep argv[0] separate (recommended).
    // exec-layer can build argv = {executable.filename().string(), args...} or use provided "argv0".
    std::optional<std::string> argv0;
    Strings args;

    // env policy: if env_inherit is true, overlay "env" onto current environment.
    // if false, use exactly env.
    bool env_inherit{true};
    EnvMap env;

    // stdio endpoints
    IO stdin_  { InheritTarget{} };
    IO stdout_ { InheritTarget{} };
    IO stderr_ { InheritTarget{} };
};

    struct CommandNode {
    std::string command_name;
    std::vector<std::string> arguments;
    std::vector<Redirection> redirections;
    bool background = false;
};
    */
    /// Execute a command
    [[nodiscard]] std::expected<int, std::string>
    execute_command(const CommandNode& node) {
        Command cmd;
        cmd.executable = node.command_name;
        cmd.args       = node.arguments;

        if (!node.redirections.empty()) {
            std::cout << "Processing redirections for command: " << cmd.executable << "\n";
            for (const auto& redir : node.redirections) {
                //cmd.redirections.push_back(redir);
                if (redir.kind == RedirectKind::Input) {
                    std::cout << "  Input redirection from: " << redir.target << "\n";
                    cmd.stdin_ = from_file(redir.target);
                } else if (redir.kind == RedirectKind::OutputTruncate) {
                    std::cout << "  Output redirection to: " << redir.target << "\n";
                    cmd.stdout_ = to_file(redir.target, OpenMode::WriteTruncate);
                } else if (redir.kind == RedirectKind::OutputAppend) {
                    std::cout << "  Output append redirection to: " << redir.target << "\n";
                    cmd.stdout_ = to_file(redir.target, OpenMode::WriteAppend);
                }
            }
        } else {
            std::cout << "No redirections for command: " << cmd.executable << "\n";
        }

        auto result = executor_.execute(cmd);
        std::cout << "Executed command: " << cmd.executable << "\n";
        std::cout << result << "\n";

        return result.exit_code;
    }

    /// Execute a pipeline (currently sequential, left-to-right)
    [[nodiscard]] std::expected<int, std::string>
    execute_pipeline(const PipelineNode& node) {
        int last = platform::EXIT_SUCCESS_STATUS;

        for (const auto& cmd : node.commands) {
            auto result = execute_command(cmd);
            if (!result) {
                return result;
            }
            last = *result;
        }

        return last;
    }

    /// Execute a sequence of statements: A ; B ; C
    [[nodiscard]] std::expected<int, std::string>
    execute_sequence(const SequenceNode& node) {
        int last = platform::EXIT_SUCCESS_STATUS;

        for (const auto& stmt : node.statements) {
            auto result = execute_statement(stmt);
            if (!result) {
                return result;
            }
            last = *result;
        }

        return last;
    }

    [[nodiscard]] std::optional<std::string> lookup_variable(const std::string& name) {
        if (variables_.contains(name)) {
            return variables_[name];
        } else {
            return std::nullopt;
        }
    }

    [[nodiscard]] std::vector<std::string> getHistory() {
        return history_.items();
    };

    std::string expand_variables(std::string_view input) {

    }

};

} // namespace wshell