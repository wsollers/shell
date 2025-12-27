// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

// command_model.hpp
#pragma once

#include <optional>
#include <string_view>
#include <variant>

#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "platform_types.hpp"  // Cross-platform process/job control types

namespace wshell {

/// ShellArg with expansion context (for proper quote handling and variable expansion)
struct ShellArg {
    std::string value;
    bool is_quoted{false};        // "foo" vs foo (affects expansion and word splitting)
    bool needs_expansion{true};   // $var, $(cmd), etc. need expansion
    // Convenience constructor
    explicit ShellArg(std::string val, bool quoted = false, bool expand = true)
        : value(std::move(val)), is_quoted(quoted), needs_expansion(expand) {}
};

using String = std::string;
using Strings = std::vector<std::string>;

using EnvironmentVariable = std::pair<std::string, std::string>;
using EnvMap = std::vector<EnvironmentVariable>;  // preserves order, allows duplicates if you want
                                                  // to mirror POSIX envp.

enum class Stream { Stdin, Stdout, Stderr };

enum class OpenMode { Read, WriteTruncate, WriteAppend };

struct FileTarget {
    std::filesystem::path path{};
    OpenMode mode{OpenMode::WriteTruncate};
};

struct PipeTarget {
    // A logical "pipe endpoint". Execution layer can map these to OS handles.
    // If you want explicit pipeline wiring, you can keep this as a token/id.
    std::size_t id{};
};

struct CaptureTarget {
    // Capture stream to memory (or later to a buffer in the exec layer).
    // Execution layer decides where bytes go.
};

struct NullTarget {
    // /dev/null or NUL equivalent.
};

struct InheritTarget {
    // Use parent's stdio handle.
};

// Unified I/O endpoint for any stream.
using IO = std::variant<InheritTarget, FileTarget, PipeTarget, CaptureTarget, NullTarget>;

struct Command {
    std::filesystem::path executable;  // absolute or relative; resolution is exec-layer policy
    std::optional<std::filesystem::path> work_dir;  // nullopt = inherit current working directory

    // argv[0] policy: keep argv[0] separate (recommended).
    // exec-layer can build argv = {executable.filename().string(), args...} or use provided
    // "argv0".
    std::optional<std::string> argv0;
    std::vector<ShellArg> args;  // Arguments as ShellArgs for expansion/quoting

    // env policy: if env_inherit is true, overlay "env" onto current environment.
    // if false, use exactly env.
    bool env_inherit{true};
    EnvMap env;

    // stdio endpoints
    IO stdin_{InheritTarget{}};
    IO stdout_{InheritTarget{}};
    IO stderr_{InheritTarget{}};
};

// A pipeline is just an ordered list of commands.
// The exec layer can automatically wire cmd[i].stdout_ -> cmd[i+1].stdin_ if not already specified.
struct Pipeline {
    std::vector<Command> commands;

    [[nodiscard]] bool empty() const noexcept { return commands.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return commands.size(); }
};

// If you want "a command can be singular or in a pipeline" as one type:
using Job = std::variant<Command, Pipeline>;

//
// Helpers (optional but nice)
//
inline Command make_command(std::filesystem::path exe, std::vector<ShellArg> args = {}) {
    Command c;
    c.executable = std::move(exe);
    c.args = std::move(args);
    return c;
}

// Overload for string arguments (for compatibility)
inline Command make_command(std::filesystem::path exe, Strings args) {
    std::vector<ShellArg> shell_args;
    for (auto& s : args)
        shell_args.emplace_back(std::move(s), false, true);
    return make_command(std::move(exe), std::move(shell_args));
}

inline Pipeline pipe(std::vector<Command> cmds) {
    return Pipeline{.commands = std::move(cmds)};
}

inline IO to_file(std::filesystem::path p, OpenMode m = OpenMode::WriteTruncate) {
    return FileTarget{.path = std::move(p), .mode = m};
}

inline IO from_file(std::filesystem::path p) {
    return FileTarget{.path = std::move(p), .mode = OpenMode::Read};
}

inline IO capture() {
    return CaptureTarget{};
}
inline IO null_io() {
    return NullTarget{};
}
inline IO inherit() {
    return InheritTarget{};
}

// ============================================================================
// EXTENDED MODEL - Control Structures, Job Control, and Runtime Context
// ============================================================================

/// Command type classification (for execution routing and security)
enum class CommandType {
    Builtin,   // Built-in commands (cd, exit, export) - must run in shell process
    External,  // External executables - can fork/exec
    Function,  // User-defined shell functions
    Alias      // Command aliases
};


/// Job with runtime state (for job control: bg, fg, jobs commands)
struct JobWithState {
    Job content;  // Command or Pipeline

    // Job control metadata
    int job_id{platform::INVALID_JOB_ID};  // Job number [1], [2], etc.
    platform::ProcessGroup process_group;  // Process group (POSIX) or Job Object (Windows)
    bool background{false};                // Started with '&'

    // Job lifecycle state
    enum class State {
        Running,    // Currently executing
        Stopped,    // Suspended (Ctrl-Z)
        Done,       // Completed successfully
        Terminated  // Killed or failed
    };
    State state{State::Running};

    // Helper methods
    [[nodiscard]] bool has_valid_process_group() const noexcept { return process_group.is_valid(); }
};

/// Conditional execution (if/then/else/fi)
struct Conditional {
    Job condition;                 // Test command (e.g., "test -f file.txt" or "[ -f file.txt ]")
    std::vector<Job> then_branch;  // Execute if condition returns 0
    std::vector<Job> else_branch;  // Execute if condition returns non-zero (optional)
};

/// While loop (while condition; do body; done)
struct WhileLoop {
    Job condition;          // Loop condition
    std::vector<Job> body;  // Loop body
};

/// For loop (for var in values; do body; done)
struct ForLoop {
    std::string variable;             // Loop variable name
    std::vector<std::string> values;  // Values to iterate over
    std::vector<Job> body;            // Loop body
};

/// User-defined shell function
struct Function {
    std::string name;                     // Function name
    std::vector<std::string> parameters;  // Optional: named parameters (bash doesn't use these)
    std::vector<Job> body;                // Function body statements
};

/// Logical operators for command chaining
enum class JobOperator {
    None,        // Single command
    And,         // cmd1 && cmd2 (execute cmd2 only if cmd1 succeeds)
    Or,          // cmd1 || cmd2 (execute cmd2 only if cmd1 fails)
    Background,  // cmd1 & (run in background)
    Sequential   // cmd1 ; cmd2 (always execute both)
};

/// Job sequence with operators (for chaining: cmd1 && cmd2 || cmd3)
struct JobSequence {
    Job job;
    JobOperator op{JobOperator::None};
    std::unique_ptr<JobSequence> next;  // Recursive chain

    // Helper to build chains
    static std::unique_ptr<JobSequence> make(Job j, JobOperator op = JobOperator::None) {
        auto seq = std::make_unique<JobSequence>();
        seq->job = std::move(j);
        seq->op = op;
        return seq;
    }
};

/// Assignment statement (let var = value, or var=value, or export var=value)
struct Assignment {
    std::string variable;
    std::string value;
    bool is_export{false};  // Should this be exported to environment?
};

/// Comment (for AST completeness)
struct Comment {
    std::string text;
};

/// Statement - anything that can appear at the top level
using Statement = std::variant<Job,          // Simple command or pipeline
                               JobSequence,  // Chained commands with operators
                               Conditional,  // if/then/else
                               WhileLoop,    // while loop
                               ForLoop,      // for loop
                               Function,     // function definition
                               Assignment,   // variable assignment
                               Comment       // comment (for AST preservation)
                               >;

/// Program - collection of statements (script or interactive session)
struct Program {
    std::vector<Statement> statements;
    std::map<std::string, Function> defined_functions;  // Functions defined in this program
};

/// Execution context - runtime state for shell interpreter
struct ExecutionContext {
    // Variables (shell-local, not exported)
    std::map<std::string, std::string> variables;

    // Environment variables (exported to child processes)
    std::map<std::string, std::string> environment;

    // User-defined functions
    std::map<std::string, Function> functions;

    // Job control table
    std::vector<JobWithState> jobs;
    int next_job_id{1};

    // Execution state
    int last_exit_status{platform::EXIT_SUCCESS_STATUS};  // $? - exit status of last command
    std::filesystem::path cwd;                            // Current working directory

    // Shell mode
    bool interactive{true};  // Interactive vs. script mode

    // Process ID (for reference)
    platform::ProcessId shell_pid{platform::INVALID_PROCESS_ID};

    // Helper methods
    [[nodiscard]] int get_exit_status() const noexcept { return last_exit_status; }
    void set_exit_status(int status) noexcept { last_exit_status = status; }

    [[nodiscard]] bool is_success() const noexcept {
        return last_exit_status == platform::EXIT_SUCCESS_STATUS;
    }

    [[nodiscard]] int add_job(JobWithState job) {
        job.job_id = next_job_id++;
        jobs.push_back(std::move(job));
        return job.job_id;
    }

    void remove_job(int job_id) {
        jobs.erase(std::remove_if(jobs.begin(), jobs.end(),
                                  [job_id](const JobWithState& j) { return j.job_id == job_id; }),
                   jobs.end());
    }

    [[nodiscard]] JobWithState* find_job(int job_id) noexcept {
        auto it = std::find_if(jobs.begin(), jobs.end(),
                               [job_id](const JobWithState& j) { return j.job_id == job_id; });
        return it != jobs.end() ? &(*it) : nullptr;
    }

    [[nodiscard]] const JobWithState* find_job(int job_id) const noexcept {
        auto it = std::find_if(jobs.begin(), jobs.end(),
                               [job_id](const JobWithState& j) { return j.job_id == job_id; });
        return it != jobs.end() ? &(*it) : nullptr;
    }
};

// ============================================================================
// Additional helper functions
// ============================================================================

/// Create a simple command with string arguments (convenience)
inline Command make_simple_command(std::string name, std::vector<std::string> args = {}) {
    std::vector<ShellArg> shell_args;
    for (auto& s : args)
        shell_args.emplace_back(std::move(s), false, true);
    Command c;
    c.executable = std::move(name);
    c.args = std::move(shell_args);
    return c;
}

/// Create a conditional
inline Conditional make_conditional(Job condition, std::vector<Job> then_branch,
                                    std::vector<Job> else_branch = {}) {
    return Conditional{.condition = std::move(condition),
                       .then_branch = std::move(then_branch),
                       .else_branch = std::move(else_branch)};
}

/// Create a while loop
inline WhileLoop make_while(Job condition, std::vector<Job> body) {
    return WhileLoop{.condition = std::move(condition), .body = std::move(body)};
}

/// Create a for loop
inline ForLoop make_for(std::string variable, std::vector<std::string> values,
                        std::vector<Job> body) {
    return ForLoop{
        .variable = std::move(variable), .values = std::move(values), .body = std::move(body)};
}

}  // namespace wshell
