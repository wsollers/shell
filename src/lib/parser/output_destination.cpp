// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/output_destination.hpp"

#include <fstream>

namespace wshell {

//==============================================================================
// StreamOutputDestination Implementation
//==============================================================================

StreamOutputDestination::StreamOutputDestination(std::ostream& stream, std::string name)
    : stream_(stream), name_(std::move(name)) {}

std::expected<void, std::string> StreamOutputDestination::write(std::string_view content) {
    stream_ << content;

    if (stream_.fail()) {
        return std::unexpected("Failed to write to stream");
    }

    return {};
}

std::expected<void, std::string> StreamOutputDestination::flush() {
    stream_.flush();

    if (stream_.fail()) {
        return std::unexpected("Failed to flush stream");
    }

    return {};
}

std::string StreamOutputDestination::destination_name() const {
    return name_;
}

//==============================================================================
// StringOutputDestination Implementation
//==============================================================================

StringOutputDestination::StringOutputDestination(std::string name) : name_(std::move(name)) {}

std::expected<void, std::string> StringOutputDestination::write(std::string_view content) {
    try {
        buffer_.append(content);
        return {};
    } catch (std::bad_alloc const&) {
        return std::unexpected("Out of memory writing to string buffer");
    }
}

std::expected<void, std::string> StringOutputDestination::flush() {
    // No-op for string destination (always "flushed")
    return {};
}

std::string StringOutputDestination::destination_name() const {
    return name_;
}

//==============================================================================
// FileOutputDestination Implementation
//==============================================================================

FileOutputDestination::FileOutputDestination(std::filesystem::path path, Mode mode)
    : path_(std::move(path)) {
    auto open_mode = (mode == Mode::Append) ? (std::ios::out | std::ios::app)
                                            : (std::ios::out | std::ios::trunc);

    stream_ = std::make_unique<std::ofstream>(path_, open_mode);
}

FileOutputDestination::~FileOutputDestination() {
    if (stream_) {
        stream_->flush();
    }
}

std::expected<void, std::string> FileOutputDestination::write(std::string_view content) {
    if (!stream_) {
        return std::unexpected("File not open: " + path_.string());
    }

    *stream_ << content;

    if (stream_->fail()) {
        return std::unexpected("Failed to write to file: " + path_.string());
    }

    return {};
}

std::expected<void, std::string> FileOutputDestination::flush() {
    if (!stream_) {
        return std::unexpected("File not open: " + path_.string());
    }

    stream_->flush();

    if (stream_->fail()) {
        return std::unexpected("Failed to flush file: " + path_.string());
    }

    return {};
}

std::string FileOutputDestination::destination_name() const {
    return path_.string();
}

}  // namespace wshell
