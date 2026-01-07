// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <expected>
#include <filesystem>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>

namespace wshell {

//==============================================================================
// OutputTruncate Destination Abstraction (Dependency Injection Interface)
//==============================================================================

/// @brief Abstract interface for text/content output destinations
/// @details Enables dependency injection and testing with fake destinations.
///          Can be used for stdout, stderr, files, string buffers, etc.
class IOutputDestination {
public:
    virtual ~IOutputDestination() = default;
    
    /// @brief Write content to the destination
    /// @param content Content to write
    /// @return Expected with void or error message
    [[nodiscard]] virtual std::expected<void, std::string> write(std::string_view content) = 0;
    
    /// @brief Flush any buffered output
    /// @return Expected with void or error message
    [[nodiscard]] virtual std::expected<void, std::string> flush() = 0;
    
    /// @brief Get destination identifier (for error reporting)
    [[nodiscard]] virtual std::string destination_name() const = 0;
};

//==============================================================================
// Concrete OutputTruncate Destinations
//==============================================================================

/// @brief Stream-based output destination (for stdout, stderr, etc.)
class StreamOutputDestination final : public IOutputDestination {
public:
    explicit StreamOutputDestination(std::ostream& stream, std::string name = "stream");
    
    [[nodiscard]] std::expected<void, std::string> write(std::string_view content) override;
    [[nodiscard]] std::expected<void, std::string> flush() override;
    [[nodiscard]] std::string destination_name() const override;

private:
    std::ostream& stream_;
    std::string name_;
};

/// @brief String-based output destination (for testing and capturing output)
class StringOutputDestination final : public IOutputDestination {
public:
    explicit StringOutputDestination(std::string name = "string");
    
    [[nodiscard]] std::expected<void, std::string> write(std::string_view content) override;
    [[nodiscard]] std::expected<void, std::string> flush() override;
    [[nodiscard]] std::string destination_name() const override;
    
    /// @brief Get the captured output
    [[nodiscard]] std::string const& captured_output() const { return buffer_; }
    
    /// @brief Clear the captured output
    void clear() { buffer_.clear(); }

private:
    std::string buffer_;
    std::string name_;
};

/// @brief File-based output destination with security checks
class FileOutputDestination final : public IOutputDestination {
public:
    enum class Mode {
        Append,     ///< OutputAppend to existing file
        Truncate    ///< Overwrite existing file
    };
    
    explicit FileOutputDestination(std::filesystem::path path, Mode mode = Mode::Truncate);
    ~FileOutputDestination() override;
    
    [[nodiscard]] std::expected<void, std::string> write(std::string_view content) override;
    [[nodiscard]] std::expected<void, std::string> flush() override;
    [[nodiscard]] std::string destination_name() const override;

private:
    std::filesystem::path path_;
    std::unique_ptr<std::ostream> stream_;
};

} // namespace shell
