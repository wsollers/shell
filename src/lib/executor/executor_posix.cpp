// Copyright (c) 2025 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

// Platform-specific executor implementation for POSIX (Linux/macOS)

#if defined(__linux__) || defined(__APPLE__)

    #include "shell/execution_policy.hpp"

    #include <cstring>
    #include <filesystem>
    #include <ios>
    #include <iostream>
    #include <memory>
    #include <mutex>
    #include <string>
    #include <unordered_map>
    #include <vector>

    #include <fcntl.h>
    #include <pwd.h>
    #include <sys/wait.h>
    #include <unistd.h>

extern char** environ;

namespace wshell {

std::optional<std::filesystem::path> get_home_directory() {
    if (const char* home = getenv("HOME")) {
        return home;
    }
    if (auto* pw = getpwuid(getuid())) {
        return pw->pw_dir;
    }
    std::cerr << "Unable to find HOME directory\n";
    return std::nullopt;
}


namespace fs = std::filesystem;

// Function to find the full path of an executable by searching the PATH environment variable
std::string findExecutableInPath(const std::string& executable_name) {
    // Get the PATH environment variable
    const char* path_env = std::getenv("PATH");
    if (!path_env) {
        std::cerr << "PATH environment variable not set." << std::endl;
        return "";
    }

    std::string path_str = path_env;
    std::string current_path;
    std::vector<std::string> directories;

    // Split the PATH string into individual directories
    for (char c : path_str) {
        if (c == ':') {
            directories.push_back(current_path);
            current_path.clear();
        } else {
            current_path += c;
        }
    }
    directories.push_back(current_path); // Add the last directory

    // Search each directory for the executable
    for (const std::string& dir : directories) {
        fs::path full_path = fs::path(dir) / executable_name;

        // Check if the file exists and is executable
        // The X_OK flag in access() checks for execute permission
        if (fs::exists(full_path) && fs::is_regular_file(full_path) && access(full_path.c_str(), X_OK) == 0) {
            return full_path.string();
        }
    }

    // Executable not found in PATH
    return "";
}

class EnvironmentCache {
  public:
    static EnvironmentCache& instance() {
        static EnvironmentCache instance;
        return instance;
    }

    std::string get(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = cache_.find(key);
        if (it != cache_.end()) {
            return it->second;
        }

        // Fallback to getenv for new keys
        if (const char* val = std::getenv(key.c_str())) {
            cache_[key] = val;
            return val;
        }

        return "";
    }

    void refresh() {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.clear();
        load_environment();
    }

    std::unordered_map<std::string, std::string> get_all() {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_;
    }

  private:
    EnvironmentCache() { load_environment(); }

    void load_environment() {
        std::unordered_map<std::string, std::string> env_map;
        for (char** env = environ; *env != nullptr; ++env) {
            const char* entry = *env;
            const char* eq = std::strchr(entry, '=');
            if (eq) {
                std::string key(entry, eq - entry);
                std::string value(eq + 1);
                env_map.emplace(std::move(key), std::move(value));
            }
        }
    }

    std::unordered_map<std::string, std::string> cache_;
    std::mutex mutex_;
};

std::vector<const char*>
PlatformExecutionPolicy::convertEnvironment(const Command& cmd) {
    // Set up environment variables
    std::unordered_map<std::string, std::string> env_map;
    env_map.insert(cmd.env.begin(), cmd.env.end());
    if (cmd.env_inherit) {
        // Add current environment to env
        auto current_env = EnvironmentCache::instance().get_all();
        env_map.insert(current_env.begin(), current_env.end());
    }
    std::vector<const char*> envp;
    for (const auto& arg : env_map) {
        envp.push_back((arg.first+ "=" + arg.second).c_str());
    }
    envp.push_back(nullptr);  // NULL-terminated

    return envp;
}

std::vector<const char*>
PlatformExecutionPolicy::convertArgv(const Command& cmd) {
    // Convert command args to C-style argv
    std::vector<const char*> argv;
    argv.push_back(cmd.executable.c_str());
    for (const auto& arg : cmd.args) {
        argv.push_back(arg.c_str());
    }
    argv.insert(argv.begin(), cmd.executable.filename().c_str());
    argv.push_back(nullptr);  // NULL-terminated

    return argv;
}

ExecutionResult PlatformExecutionPolicy::execute(const Command& cmd) const {
    // Fork process
    pid_t pid = fork();

    if (pid < 0) {
        // Fork failed
        return ExecutionResult{.error_code = errno,
                               .exit_code = platform::EXIT_FAILURE_STATUS,
                               .error_message =
                                   "Failed to fork process: " + std::string(std::strerror(errno))};
    } else if (pid == 0) {
        //
        // Child process
        //
        // change into correct directory if needed
        if (cmd.work_dir.has_value()) {
            if (chdir(cmd.work_dir->c_str()) != 0) {
                perror("chdir");
                std::cerr << "Failed to change directory: " << std::strerror(errno) << "\n";
                _exit(127);  // Exit child process
            }
        }

        // handle redirections here (TODO)
        if (std::holds_alternative<FileTarget>(cmd.stdin_)) {
            // const auto& file_target = std::get<FileTarget>(cmd.stdin_);
            //  Open file and redirect stdin (TODO)
        }
        if (std::holds_alternative<FileTarget>(cmd.stdout_)) {

            const auto& file_target = std::get<FileTarget>(cmd.stdout_);
            std::cout << "Redirecting stdout to file: " << file_target.path.c_str() << std::endl;
            // TODO umask
            //  Open file and redirect stdout
            int fd = open(file_target.path.c_str(),
                          file_target.mode == OpenMode::WriteAppend ? O_WRONLY | O_CREAT | O_APPEND
                                                                    : O_WRONLY | O_CREAT | O_TRUNC,
                          0644);
            if (fd < 0) {
                perror("Open target file for stdout redirection");
                std::cerr << "Failed to open file for stdout redirection: " << std::strerror(errno)
                          << "\n";
                _exit(126);
            }
            if (dup2(fd, STDOUT_FILENO) < 0) {
                perror("dup2 stdout");
                std::cerr << "Failed to redirect stdout: " << std::strerror(errno) << "\n";
                _exit(127);
            }
            close(fd);
        }
        if (std::holds_alternative<FileTarget>(cmd.stderr_)) {
            // const auto& file_target = std::get<FileTarget>(cmd.stderr_);
            //  Open file and redirect stderr (TODO)
        }
        // Set up environment variables
        std::unordered_map<std::string, std::string> env_map;
        if (cmd.env_inherit == false) {
            // Set provided environment variables
            env_map.insert(cmd.env.begin(), cmd.env.end());
        } else {
            // Modify inherited environment
            auto current_env = EnvironmentCache::instance().get_all();
            env_map.insert(current_env.begin(), current_env.end());
            env_map.insert(cmd.env.begin(), cmd.env.end());
        }
        // Convert command args to C-style argv
        std::vector<const char*> argv;
        argv.push_back(cmd.executable.c_str());
        for (const auto& arg : cmd.args) {
            argv.push_back(arg.c_str());
        }
        argv.push_back(nullptr);  // NULL-terminated

        // Convert environment map to C-style envp
        std::vector<std::string> env_storage;
        std::vector<char*> envp;
        for (const auto& [key, value] : env_map) {
            env_storage.push_back(key + "=" + value);
        }
        for (auto& s : env_storage) {
            envp.push_back(s.data());
        }
        envp.push_back(nullptr);  // Null-terminate
                                  // Child process - execute command
        std::string command_path = cmd.executable;
        /*
        std::cout << "Executing command: " << command_path << "\n";
        std::cout << "With arguments:\n";
        for (size_t i = 0; i < argv.size() - 1; ++i) {
            std::cout << "  argv[" << i << "] = " << argv[i] << "\n";
        }
        std::cout << "With environment variables:\n";
        for (const auto& [key, value] : env_map) {
            std::cout << "  " << key << "=" << value << "\n";
        }
        std::cout << std::endl;
        */
        //lookup executable in PATH if not an absolute or relative path
        std::string found_path = findExecutableInPath(cmd.executable);
        auto rc = execve(found_path.c_str(), const_cast<char* const*>(argv.data()),
                         static_cast<char* const*>(envp.data()));
        if (rc < 0) {
            perror("execve");
            // If execvp returns, it failed
            std::cerr << "Failed to execute command: " << std::strerror(errno) << "\n";
            _exit(125);  // Command not fou
        } else {
            _exit(0);  // Should never reach here
        }
    } else if (pid > 0) {
        //
        // Parent process
        //
        // Parent process - wait for child
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return ExecutionResult{.error_code = errno,
                                   .exit_code = platform::EXIT_FAILURE_STATUS,
                                   .error_message = "Failed to wait for child process"};
        }

        // Extract exit code
        int exit_code = platform::EXIT_FAILURE_STATUS;
        if (WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            exit_code = platform::EXIT_SIGNAL_BASE + WTERMSIG(status);
        }
        return ExecutionResult{.exit_code = exit_code, .error_message = std::nullopt};
    }
    return ExecutionResult{.error_code = 0, .error_message = std::nullopt};
}
ExecutionResult PlatformExecutionPolicy::execute(const Pipeline& pipeline) const {
    // Phase 1: No pipeline support yet - just execute first command
    if (pipeline.empty()) {
        return ExecutionResult{.exit_code = platform::EXIT_FAILURE_STATUS,
                               .error_message = "Empty pipeline"};
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

}  // namespace wshell

#endif  // __linux__ || __APPLE__
