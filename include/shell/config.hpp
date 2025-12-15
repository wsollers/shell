// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <expected>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>

namespace shell {

/// @brief Configuration and variable management for the shell
/// @details Provides bash-like variable storage with secure parsing and validation
class Config {
public:
    using Variables = std::map<std::string, std::string>;
    
    /// @brief Error types for configuration operations
    enum class ErrorCode {
        FILE_NOT_FOUND,
        PERMISSION_DENIED,
        PARSE_ERROR,
        INVALID_VARIABLE_NAME,
        FILE_TOO_LARGE,
        IO_ERROR
    };
    
    struct Error {
        ErrorCode code;
        std::string message;
        std::size_t line_number{0};
    };

    /// @brief Default constructor with empty variables
    Config() = default;

    /// @brief Load configuration from a file
    /// @param path Path to configuration file
    /// @return Expected with Config or Error
    /// @note Validates file size, permissions, and content for security
    [[nodiscard]] static std::expected<Config, Error> load_from_file(
        std::filesystem::path const& path
    );

    /// @brief Parse configuration from string content
    /// @param content Configuration content in key=value format
    /// @return Expected with Config or Error
    /// @note Performs input validation per AI security guidelines
    [[nodiscard]] static std::expected<Config, Error> parse(std::string_view content);

    /// @brief Get the default config file path for the current user
    /// @return Path to ~/.wshellrc or equivalent
    /// @note Platform-aware (uses HOME on Unix, USERPROFILE on Windows)
    [[nodiscard]] static std::filesystem::path default_config_path();

    /// @brief Get a variable value
    /// @param name Variable name
    /// @return Pointer to value if exists, nullptr otherwise
    /// @warning FIXME: Returns raw pointer to internal map data - unsafe!
    ///          Pointer can dangle after set(), unset(), or clear() operations.
    ///          Should return std::optional<std::string_view> or std::optional<std::string>
    ///          See C++ Core Guidelines F.43, F.45
    [[nodiscard]] std::string const* get(std::string_view name) const;

    /// @brief Set a variable value
    /// @param name Variable name (must be valid identifier)
    /// @param value Variable value
    /// @return true if set successfully, false if invalid name
    bool set(std::string name, std::string value);

    /// @brief Remove a variable
    /// @param name Variable name
    /// @return true if removed, false if didn't exist
    bool unset(std::string_view name);

    /// @brief Get all variables
    /// @return Reference to internal variable map
    [[nodiscard]] Variables const& variables() const { return user_config_; }

    /// @brief Check if a variable exists
    /// @param name Variable name
    /// @return true if variable exists
    [[nodiscard]] bool has(std::string_view name) const;

    /// @brief Clear all variables
    void clear();

private:
    Variables user_config_;

    /// @brief Validate variable name follows bash identifier rules
    /// @param name Variable name to validate
    /// @return true if valid (alphanumeric + underscore, not starting with digit)
    [[nodiscard]] static bool is_valid_variable_name(std::string_view name);

    /// @brief Trim whitespace from both ends
    [[nodiscard]] static std::string_view trim(std::string_view str);
};

} // namespace shell
