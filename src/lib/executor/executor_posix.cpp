// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

// Platform-specific executor implementation for POSIX (Linux/macOS)

#if defined(__linux__) || defined(__APPLE__)

#include "shell/execution_policy.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>

namespace wshell {

ExecutionResult PlatformExecutionPolicy::execute(const Command& cmd) const {
    // Convert command to C-style argv
    std::vector<const char*> argv;
    argv.push_back(cmd.executable.c_str());
    for (const auto& arg : cmd.args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);  // NULL-terminated
    
    // Fork process
    pid_t pid = fork();
    
    if (pid < 0) {
        // Fork failed
        return ExecutionResult{
            .exit_code = platform::EXIT_FAILURE_STATUS,
            .error_message = "Failed to fork process: " + std::string(std::strerror(errno))
        };
    }
    
    if (pid == 0) {
        // Child process - execute command
        execvp(argv[0], const_cast<char* const*>(argv.data()));
        
        // If execvp returns, it failed
        std::cerr << "Failed to execute command: " << std::strerror(errno) << "\n";
        _exit(127);  // Command not found
    }
    
    // Parent process - wait for child
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        return ExecutionResult{
            .exit_code = platform::EXIT_FAILURE_STATUS,
            .error_message = "Failed to wait for child process"
        };
    }
    
    // Extract exit code
    int exit_code = platform::EXIT_FAILURE_STATUS;
    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        exit_code = platform::EXIT_SIGNAL_BASE + WTERMSIG(status);
    }
    
    return ExecutionResult{
        .exit_code = exit_code,
        .error_message = std::nullopt
    };
}

ExecutionResult PlatformExecutionPolicy::execute(const Pipeline& pipeline) const {
    // Phase 1: No pipeline support yet - just execute first command
    if (pipeline.empty()) {
        return ExecutionResult{
            .exit_code = platform::EXIT_FAILURE_STATUS,
            .error_message = "Empty pipeline"
        };
    }
    
    // For now, just execute the first command
    // Phase 2 will add proper pipeline support with pipes
    return execute(pipeline.commands[0]);
}

void PlatformExecutionPolicy::init_job_control() const {
    // TODO: Initialize job control for interactive shell
    // - Set process group
    // - Configure terminal signals
}

} // namespace wshell

#endif // __linux__ || __APPLE__
