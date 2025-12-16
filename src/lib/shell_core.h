// Copyright (c) 2025 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#ifndef WSHELL_SHELL_CORE_H
#define WSHELL_SHELL_CORE_H

#include <string>
#include <string_view>
#include <expected>
#include <vector>

#ifdef _WIN32
    #ifdef WSHELL_EXPORTS
        #define WSHELL_API __declspec(dllexport)
    #else
        #define WSHELL_API __declspec(dllimport)
    #endif
#else
    #define WSHELL_API __attribute__((visibility("default")))
#endif

namespace wshell {

/// @brief Error codes for shell operations
enum class ShellError {
    Success = 0,
    InvalidCommand,
    ExecutionFailed,
    PermissionDenied,
    InvalidArgument
};

/// @brief Core shell interpreter class
class WSHELL_API ShellCore {
public:
    ShellCore() noexcept = default;
    ~ShellCore() = default;
    
    // Disable copy, enable move
    ShellCore(const ShellCore&) = delete;
    ShellCore& operator=(const ShellCore&) = delete;
    ShellCore(ShellCore&&) noexcept = default;
    ShellCore& operator=(ShellCore&&) noexcept = default;
    
    /// @brief Execute a shell command
    /// @param command The command to execute
    /// @return Result containing exit code or error
    [[nodiscard]] std::expected<int, ShellError> execute(std::string_view command) noexcept;
    
    /// @brief Get the shell version
    /// @return Version string
    [[nodiscard]] static constexpr std::string_view version() noexcept {
        return "0.2.1";
    }
    
    /// @brief Validate command syntax
    /// @param command Command to validate
    /// @return true if valid, false otherwise
    [[nodiscard]] static bool validateCommand(std::string_view command) noexcept;
};

} // namespace wshell

#endif // WSHELL_SHELL_CORE_H
