// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <expected>
#include <filesystem>
#include <istream>
#include <memory>
#include <string>

namespace wshell {

//==============================================================================
// Input Source Abstraction (Dependency Injection Interface)
//==============================================================================

/// @brief Abstract interface for text/content input sources
/// @details Enables dependency injection and testing with fake sources.
///          Can be used for configuration files, user input, scripts, etc.
class IInputSource {
public:
    virtual ~IInputSource() = default;
    
    /// @brief Read content from the source
    /// @return Expected with content string or error message
    [[nodiscard]] virtual std::expected<std::string, std::string> read() = 0;
    
    /// @brief Read a single line from the source (for interactive input)
    /// @return Expected with line string (without newline) or error message
    [[nodiscard]] virtual std::expected<std::string, std::string> read_line() {
        // Default implementation: read entire content (suitable for file/string sources)
        return read();
    }
    
    /// @brief Get source identifier (for error reporting)
    [[nodiscard]] virtual std::string source_name() const = 0;
};

//==============================================================================
// Concrete Input Sources
//==============================================================================

/// @brief File-based input source with security checks
class FileInputSource final : public IInputSource {
public:
    explicit FileInputSource(std::filesystem::path path);
    
    [[nodiscard]] std::expected<std::string, std::string> read() override;
    [[nodiscard]] std::string source_name() const override;

private:
    std::filesystem::path path_;
    static constexpr std::size_t MAX_FILE_SIZE = 1'048'576;  // 1MB
};

/// @brief Stream-based input source (for stdin, testing, etc.)
class StreamInputSource final : public IInputSource {
public:
    explicit StreamInputSource(std::istream& stream, std::string name = "stream");
    
    [[nodiscard]] std::expected<std::string, std::string> read() override;
    [[nodiscard]] std::expected<std::string, std::string> read_line() override;
    [[nodiscard]] std::string source_name() const override;

private:
    std::istream& stream_;
    std::string name_;
    static constexpr std::size_t MAX_STREAM_SIZE = 1'048'576;  // 1MB
};

/// @brief String-based input source (for testing and in-memory content)
class StringInputSource final : public IInputSource {
public:
    explicit StringInputSource(std::string content, std::string name = "string");
    
    [[nodiscard]] std::expected<std::string, std::string> read() override;
    [[nodiscard]] std::string source_name() const override;

private:
    std::string content_;
    std::string name_;
};

} // namespace shell
