// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include "input_source.hpp"
#include <expected>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>

namespace shell {

//==============================================================================
// Type Aliases for Backward Compatibility
//==============================================================================

/// @brief Alias for configuration input sources (backward compatibility)
using IConfigSource = IInputSource;

/// @brief Alias for file-based configuration source (backward compatibility)
using FileConfigSource = FileInputSource;

/// @brief Alias for stream-based configuration source (backward compatibility)
using StreamConfigSource = StreamInputSource;

/// @brief Alias for string-based configuration source (backward compatibility)
using StringConfigSource = StringInputSource;

//==============================================================================
// Validation Policy (Compile-time customization)
//==============================================================================

/// @brief Default validation policy with bash-like rules
struct DefaultValidationPolicy {
    static constexpr std::size_t MAX_CONFIG_SIZE = 1'048'576;   // 1MB
    static constexpr std::size_t MAX_LINE_LENGTH = 10'000;      // 10KB
    static constexpr std::size_t MAX_VARIABLE_COUNT = 10'000;   // Max variables
    static constexpr std::size_t MAX_NAME_LENGTH = 1'000;       // Max name
    static constexpr std::size_t MAX_VALUE_LENGTH = 100'000;    // Max value
    
    /// @brief Validate variable name (bash rules: alphanumeric + underscore)
    [[nodiscard]] static bool is_valid_name(std::string_view name);
    
    /// @brief Check size limits
    [[nodiscard]] static bool check_limits(std::size_t name_len, 
                                           std::size_t value_len,
                                           std::size_t var_count);
};

/// @brief Strict validation policy for security-critical contexts
struct StrictValidationPolicy {
    static constexpr std::size_t MAX_CONFIG_SIZE = 102'400;     // 100KB
    static constexpr std::size_t MAX_LINE_LENGTH = 1'000;       // 1KB
    static constexpr std::size_t MAX_VARIABLE_COUNT = 1'000;    // Max variables
    static constexpr std::size_t MAX_NAME_LENGTH = 100;         // Max name
    static constexpr std::size_t MAX_VALUE_LENGTH = 10'000;     // Max value
    
    [[nodiscard]] static bool is_valid_name(std::string_view name);
    [[nodiscard]] static bool check_limits(std::size_t name_len,
                                           std::size_t value_len,
                                           std::size_t var_count);
};

//==============================================================================
// Configuration Class (Redesigned with DI)
//==============================================================================

/// @brief Configuration and variable management for the shell
/// @tparam ValidationPolicy Policy for validation rules (default: DefaultValidationPolicy)
/// @details Provides bash-like variable storage with:
///          - Dependency injection for input sources
///          - Policy-based validation
///          - Memory-safe interface (no dangling pointers)
///          - Easy testing with fake/mock sources
template<typename ValidationPolicy = DefaultValidationPolicy>
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
        IO_ERROR,
        SOURCE_READ_ERROR
    };
    
    struct Error {
        ErrorCode code;
        std::string message;
        std::size_t line_number{0};
        std::string source_name;
        std::source_location location;  // No default - must be set via make_error()
    };

    //==========================================================================
    // Construction
    //==========================================================================
    
    /// @brief Default constructor with empty variables
    Config() = default;

    /// @brief Load configuration from an input source (DI)
    /// @param source Pointer to input source (must not be null)
    /// @return Expected with Config or Error
    /// @note Uses dependency injection for testability
    [[nodiscard]] static std::expected<Config, Error> load_from_source(
        std::unique_ptr<IInputSource> source
    );
    
    /// @brief Load configuration from an input source (DI, non-owning)
    /// @param source Reference to input source
    /// @return Expected with Config or Error
    [[nodiscard]] static std::expected<Config, Error> load_from_source(
        IInputSource& source
    );

    /// @brief Convenience: Load from file path
    /// @param path Path to configuration file
    /// @return Expected with Config or Error
    [[nodiscard]] static std::expected<Config, Error> load_from_file(
        std::filesystem::path const& path
    );

    /// @brief Convenience: Parse from string content
    /// @param content Configuration content in key=value format
    /// @return Expected with Config or Error
    [[nodiscard]] static std::expected<Config, Error> parse(std::string_view content);

    /// @brief Get the default config file path for the current user
    /// @return Path to ~/.wshellrc or equivalent
    [[nodiscard]] static std::filesystem::path default_config_path();

    //==========================================================================
    // Variable Access (Memory Safe)
    //==========================================================================
    
    /// @brief Get a variable value
    /// @param name Variable name
    /// @return Optional containing value if exists
    /// @note Fixed: Returns optional instead of raw pointer (memory safe)
    [[nodiscard]] std::optional<std::string> get(std::string_view name) const;
    
    /// @brief Get a variable value as string_view
    /// @param name Variable name
    /// @return Optional containing string_view if exists
    /// @warning View is only valid until next set/unset/clear operation
    [[nodiscard]] std::optional<std::string_view> get_view(std::string_view name) const;

    /// @brief Set a variable value
    /// @param name Variable name (must be valid identifier)
    /// @param value Variable value
    /// @return Expected with void or Error
    [[nodiscard]] std::expected<void, Error> set(std::string name, std::string value);

    /// @brief Remove a variable
    /// @param name Variable name
    /// @return true if removed, false if didn't exist
    bool unset(std::string_view name);

    /// @brief Get all variables
    /// @return Reference to internal variable map
    [[nodiscard]] Variables const& variables() const { return variables_; }

    /// @brief Check if a variable exists
    /// @param name Variable name
    /// @return true if variable exists
    [[nodiscard]] bool has(std::string_view name) const;

    /// @brief Clear all variables
    void clear();

private:
    Variables variables_;

    /// @brief Parse configuration content from string
    /// @param content Content to parse
    /// @param source_name Name of source for error reporting
    /// @return Expected with Config or Error
    [[nodiscard]] static std::expected<Config, Error> parse_impl(
        std::string_view content,
        std::string_view source_name
    );

    /// @brief Create an error with source location
    /// @param code Error code
    /// @param message Error message
    /// @param loc Source location where error was created
    /// @return Error object
    [[nodiscard]] static Error make_error(
        ErrorCode code,
        std::string message,
        std::source_location loc = std::source_location::current()
    ) {
        return Error{
            .code = code,
            .message = std::move(message),
            .line_number = 0,
            .source_name = {},
            .location = loc
        };
    }

    /// @brief Create an error with source location and line number
    /// @param code Error code
    /// @param message Error message
    /// @param line_num Line number in config file
    /// @param source Source name
    /// @param loc Source location where error was created
    /// @return Error object
    [[nodiscard]] static Error make_error(
        ErrorCode code,
        std::string message,
        std::size_t line_num,
        std::string source,
        std::source_location loc = std::source_location::current()
    ) {
        return Error{
            .code = code,
            .message = std::move(message),
            .line_number = line_num,
            .source_name = std::move(source),
            .location = loc
        };
    }

    /// @brief Trim whitespace from both ends
    [[nodiscard]] static std::string_view trim(std::string_view str);
};

//==============================================================================
// Type Aliases for Common Configurations
//==============================================================================

/// @brief Default Config using DefaultValidationPolicy
using DefaultConfig = Config<DefaultValidationPolicy>;

/// @brief Strict Config using StrictValidationPolicy  
using StrictConfig = Config<StrictValidationPolicy>;

} // namespace shell

// Include template implementation
#include "config_impl.hpp"
