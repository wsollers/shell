// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <algorithm>
#include <cctype>
#include <ranges>

#include "config.hpp"


namespace wshell {

//==============================================================================
// Config Template Implementation
//==============================================================================

template<typename ValidationPolicy>
std::expected<Config<ValidationPolicy>, ConfigError>
Config<ValidationPolicy>::loadFromSource(std::unique_ptr<IInputSource> source) {
    if (!source) {
        auto err = make_error(ErrorCode::IO_ERROR, "Null configuration source");
        err.source_name = "null";
        return std::unexpected(err);
    }
    return loadFromSource(*source);
}

template<typename ValidationPolicy>
std::expected<Config<ValidationPolicy>, ConfigError>
Config<ValidationPolicy>::loadFromSource(IInputSource& source) {
    auto content_result = source.read();
    if (!content_result) {
        auto err = make_error(ErrorCode::SOURCE_READ_ERROR, content_result.error());
        err.source_name = source.source_name();
        return std::unexpected(err);
    }
    
    return parse_impl(*content_result, source.source_name());
}


template<typename ValidationPolicy>
std::expected<Config<ValidationPolicy>, ConfigError>
Config<ValidationPolicy>::parse(std::string_view content) {
    auto source = StringInputSource(std::string(content), "string");
    return loadFromSource(source);
}

template<typename ValidationPolicy>
std::expected<Config<ValidationPolicy>, ConfigError>
Config<ValidationPolicy>::parse_impl(std::string_view content, std::string_view source_name) {
    // Security: Validate input size
    if (content.size() > ValidationPolicy::MAX_CONFIG_SIZE) {
        return std::unexpected(make_error(
            ErrorCode::FILE_TOO_LARGE,
            "Configuration content exceeds maximum size",
            0,
            std::string(source_name)
        ));
    }

    Config config;
    std::size_t line_number = 0;
    std::size_t pos = 0;

    while (pos < content.size()) {
        ++line_number;
        
        // Find end of line
        std::size_t line_end = content.find('\n', pos);
        if (line_end == std::string_view::npos) {
            line_end = content.size();
        }

        // Security: Check line length
        std::size_t line_length = line_end - pos;
        if (line_length > ValidationPolicy::MAX_LINE_LENGTH) {
            return std::unexpected(make_error(
                ErrorCode::PARSE_ERROR,
                "Line exceeds maximum length",
                line_number,
                std::string(source_name)
            ));
        }

        std::string_view line = content.substr(pos, line_length);
        line = trim(line);

        // Skip empty lines and comments (bash-style)
        if (!line.empty() && line[0] != '#') {
            // Find the '=' separator
            auto eq_pos = line.find('=');
            
            if (eq_pos == std::string_view::npos) {
                // Allow lines without '=' to be ignored (like bash)
                pos = line_end + 1;
                continue;
            }

            // Extract name and value
            std::string_view name = trim(line.substr(0, eq_pos));
            std::string_view value = trim(line.substr(eq_pos + 1));

            // Security: Validate name length
            if (name.size() > ValidationPolicy::MAX_NAME_LENGTH) {
                return std::unexpected(make_error(
                    ErrorCode::INVALID_VARIABLE_NAME,
                    "Variable name exceeds maximum length",
                    line_number,
                    std::string(source_name)
                ));
            }

            // Security: Validate value length
            if (value.size() > ValidationPolicy::MAX_VALUE_LENGTH) {
                return std::unexpected(make_error(
                    ErrorCode::PARSE_ERROR,
                    "Variable value exceeds maximum length",
                    line_number,
                    std::string(source_name)
                ));
            }

            // Validate variable name
            if (!ValidationPolicy::is_valid_name(name)) {
                return std::unexpected(make_error(
                    ErrorCode::INVALID_VARIABLE_NAME,
                    "Invalid variable name: " + std::string(name),
                    line_number,
                    std::string(source_name)
                ));
            }

            // Handle quoted values (bash-style)
            std::string value_str{value};
            if (value_str.size() >= 2) {
                if ((value_str.front() == '"' && value_str.back() == '"') ||
                    (value_str.front() == '\'' && value_str.back() == '\'')) {
                    value_str = value_str.substr(1, value_str.size() - 2);
                }
            }

            config.variables_[std::string(name)] = value_str;

            // Security: Check variable count limit
            if (config.variables_.size() > ValidationPolicy::MAX_VARIABLE_COUNT) {
                return std::unexpected(make_error(
                    ErrorCode::PARSE_ERROR,
                    "Too many variables",
                    line_number,
                    std::string(source_name)
                ));
            }
        }

        pos = line_end + 1;
    }

    return config;
}

template<typename ValidationPolicy>
std::optional<std::string> Config<ValidationPolicy>::get(std::string_view name) const {
    auto it = variables_.find(std::string(name));
    if (it != variables_.end()) {
        return it->second;
    }
    return std::nullopt;
}

template<typename ValidationPolicy>
std::optional<std::string_view> Config<ValidationPolicy>::get_view(std::string_view name) const {
    auto it = variables_.find(std::string(name));
    if (it != variables_.end()) {
        return it->second;
    }
    return std::nullopt;
}

template<typename ValidationPolicy>
std::expected<void, ConfigError>

Config<ValidationPolicy>::set(std::string name, std::string value) {
    // Security: Validate name
    if (!ValidationPolicy::is_valid_name(name)) {
        auto err = make_error(
            ErrorCode::INVALID_VARIABLE_NAME,
            "Invalid variable name: " + name
        );
        err.source_name = "set";
        return std::unexpected(err);
    }

    // Security: Check limits
    if (!ValidationPolicy::check_limits(name.size(), value.size(), variables_.size())) {
        auto err = make_error(
            ErrorCode::PARSE_ERROR,
            "Variable exceeds size limits or max count"
        );
        err.source_name = "set";
        return std::unexpected(err);
    }

    variables_[std::move(name)] = std::move(value);
    return {};
}

template<typename ValidationPolicy>
bool Config<ValidationPolicy>::unset(std::string_view name) {
    return variables_.erase(std::string(name)) > 0;
}

template<typename ValidationPolicy>
bool Config<ValidationPolicy>::has(std::string_view name) const {
    return variables_.contains(std::string(name));
}

template<typename ValidationPolicy>
void Config<ValidationPolicy>::clear() {
    variables_.clear();
}

template<typename ValidationPolicy>
std::string_view Config<ValidationPolicy>::trim(std::string_view str) {
    auto start = std::ranges::find_if(str, [](unsigned char ch) {
        return std::isspace(ch) == 0;
    });

    if (start == str.end()) {
        return {};
    }

    auto end = std::ranges::find_if(str | std::views::reverse, [](unsigned char ch) {
        return std::isspace(ch) == 0;
    }).base();

    return std::string_view(start, end);
}

} // namespace shell
