// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/input_source.hpp"

#include <fstream>
#include <sstream>

namespace wshell {

//==============================================================================
// FileInputSource Implementation
//==============================================================================

FileInputSource::FileInputSource(std::filesystem::path path) : path_(std::move(path)) {}

std::expected<std::string, std::string> FileInputSource::read() {
    // Check file existence
    std::error_code ec;
    if (!std::filesystem::exists(path_, ec)) {
        return std::unexpected("File not found: " + path_.string());
    }

    // Security: Check file size before reading
    auto file_size = std::filesystem::file_size(path_, ec);
    if (ec) {
        return std::unexpected("Cannot determine file size: " + ec.message());
    }

    if (file_size > MAX_FILE_SIZE) {
        return std::unexpected("File exceeds maximum size (1MB)");
    }

    // Read file content
    std::ifstream file(path_);
    if (!file) {
        return std::unexpected("Cannot open file: " + path_.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    if (file.fail() && !file.eof()) {
        return std::unexpected("Error reading file");
    }

    return buffer.str();
}

std::string FileInputSource::source_name() const {
    return path_.string();
}

//==============================================================================
// StreamInputSource Implementation
//==============================================================================

StreamInputSource::StreamInputSource(std::istream& stream, std::string name)
    : stream_(stream), name_(std::move(name)) {}

std::expected<std::string, std::string> StreamInputSource::read() {
    std::stringstream buffer;

    // Read with size limit
    std::size_t total_read = 0;
    char chunk[4096];

    while (stream_.read(chunk, sizeof(chunk)) || stream_.gcount() > 0) {
        auto count = stream_.gcount();
        total_read += count;

        if (total_read > MAX_STREAM_SIZE) {
            return std::unexpected("Stream exceeds maximum size (1MB)");
        }

        buffer.write(chunk, count);
    }

    if (stream_.bad()) {
        return std::unexpected("Error reading from stream");
    }

    return buffer.str();
}

std::expected<std::string, std::string> StreamInputSource::read_line() {
    std::string line;

    if (!std::getline(stream_, line)) {
        if (stream_.eof()) {
            return std::unexpected("End of input");
        }
        if (stream_.bad()) {
            return std::unexpected("Error reading line from stream");
        }
        return std::unexpected("Failed to read line");
    }

    // Security: Check line length
    static constexpr std::size_t MAX_LINE_SIZE = 10'240;  // 10KB per line
    if (line.size() > MAX_LINE_SIZE) {
        return std::unexpected("Line exceeds maximum size (10KB)");
    }

    return line;
}

std::string StreamInputSource::source_name() const {
    return name_;
}

//==============================================================================
// StringInputSource Implementation
//==============================================================================

StringInputSource::StringInputSource(std::string content, std::string name)
    : content_(std::move(content)), name_(std::move(name)) {}

std::expected<std::string, std::string> StringInputSource::read() {
    return content_;
}

std::string StringInputSource::source_name() const {
    return name_;
}

}  // namespace wshell
