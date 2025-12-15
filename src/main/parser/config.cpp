// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/config.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <ranges>
#include <sstream>

#ifdef _WIN32
#include <stdlib.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

namespace shell {

namespace {
    // Security limits per AI guidelines
    constexpr std::size_t MAX_CONFIG_SIZE = 1'048'576;  // 1MB
    constexpr std::size_t MAX_LINE_LENGTH = 10'000;     // 10KB per line
    constexpr std::size_t MAX_VARIABLE_COUNT = 10'000;  // Max variables
    constexpr std::size_t MAX_NAME_LENGTH = 1'000;      // Max variable name
    constexpr std::size_t MAX_VALUE_LENGTH = 100'000;   // Max value length
}

std::filesystem::path Config::default_config_path() {
    std::filesystem::path home;
    
#ifdef _WIN32
    char const* userprofile = std::getenv("USERPROFILE");
    if (userprofile) {
        home = userprofile;
    }
#else
    // Try HOME first, fall back to getpwuid
    char const* home_env = std::getenv("HOME");
    if (home_env) {
        home = home_env;
    } else {
        // Use getpwuid as fallback
        passwd* pw = getpwuid(getuid());
        if (pw && pw->pw_dir) {
            home = pw->pw_dir;
        }
    }
#endif

    if (home.empty()) {
        // Last resort fallback
        home = std::filesystem::current_path();
    }

    return home / ".wshellrc";
}

std::expected<Config, Config::Error> Config::load_from_file(
    std::filesystem::path const& path
) {
    // Check file existence
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        return std::unexpected(Error{
            .code = ErrorCode::FILE_NOT_FOUND,
            .message = "Configuration file not found: " + path.string()
        });
    }

    // Security: Check file size before reading
    auto file_size = std::filesystem::file_size(path, ec);
    if (ec) {
        return std::unexpected(Error{
            .code = ErrorCode::IO_ERROR,
            .message = "Cannot determine file size: " + ec.message()
        });
    }

    if (file_size > MAX_CONFIG_SIZE) {
        return std::unexpected(Error{
            .code = ErrorCode::FILE_TOO_LARGE,
            .message = "Configuration file exceeds 1MB limit"
        });
    }

    // Read file content
    std::ifstream file(path);
    if (!file) {
        return std::unexpected(Error{
            .code = ErrorCode::PERMISSION_DENIED,
            .message = "Cannot open configuration file: " + path.string()
        });
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    
    if (file.fail() && !file.eof()) {
        return std::unexpected(Error{
            .code = ErrorCode::IO_ERROR,
            .message = "Error reading configuration file"
        });
    }

    // Parse the content
    return parse(buffer.str());
}

std::expected<Config, Config::Error> Config::parse(std::string_view content) {
    // Security: Validate input size
    if (content.size() > MAX_CONFIG_SIZE) {
        return std::unexpected(Error{
            .code = ErrorCode::FILE_TOO_LARGE,
            .message = "Configuration content exceeds 1MB limit"
        });
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
        if (line_length > MAX_LINE_LENGTH) {
            return std::unexpected(Error{
                .code = ErrorCode::PARSE_ERROR,
                .message = "Line exceeds maximum length (10KB)",
                .line_number = line_number
            });
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
            if (name.size() > MAX_NAME_LENGTH) {
                return std::unexpected(Error{
                    .code = ErrorCode::INVALID_VARIABLE_NAME,
                    .message = "Variable name exceeds maximum length",
                    .line_number = line_number
                });
            }

            // Security: Validate value length
            if (value.size() > MAX_VALUE_LENGTH) {
                return std::unexpected(Error{
                    .code = ErrorCode::PARSE_ERROR,
                    .message = "Variable value exceeds maximum length",
                    .line_number = line_number
                });
            }

            // Validate variable name
            if (!is_valid_variable_name(name)) {
                return std::unexpected(Error{
                    .code = ErrorCode::INVALID_VARIABLE_NAME,
                    .message = "Invalid variable name: " + std::string(name),
                    .line_number = line_number
                });
            }

            // Handle quoted values (bash-style)
            std::string value_str{value};
            if (value_str.size() >= 2) {
                if ((value_str.front() == '"' && value_str.back() == '"') ||
                    (value_str.front() == '\'' && value_str.back() == '\'')) {
                    value_str = value_str.substr(1, value_str.size() - 2);
                }
            }

            config.user_config_[std::string(name)] = value_str;

            // Security: Check variable count limit
            if (config.user_config_.size() > MAX_VARIABLE_COUNT) {
                return std::unexpected(Error{
                    .code = ErrorCode::PARSE_ERROR,
                    .message = "Too many variables (maximum 10,000)",
                    .line_number = line_number
                });
            }
        }

        pos = line_end + 1;
    }

    return config;
}

// FIXME: Memory safety issue - returning raw pointer to internal map data
// This pointer can become dangling if:
// 1. Another set() triggers map rehash
// 2. unset() is called on this variable
// 3. clear() is called
// Should return std::optional<std::string_view> or std::optional<std::string> instead
std::string const* Config::get(std::string_view name) const {
    auto it = user_config_.find(std::string(name));
    if (it != user_config_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool Config::set(std::string name, std::string value) {
    // Security: Validate name
    if (!is_valid_variable_name(name)) {
        return false;
    }

    // Security: Check limits
    if (name.size() > MAX_NAME_LENGTH || value.size() > MAX_VALUE_LENGTH) {
        return false;
    }

    if (user_config_.size() >= MAX_VARIABLE_COUNT && !has(name)) {
        return false;  // Would exceed max variables
    }

    user_config_[std::move(name)] = std::move(value);
    return true;
}

bool Config::unset(std::string_view name) {
    return user_config_.erase(std::string(name)) > 0;
}

bool Config::has(std::string_view name) const {
    return user_config_.contains(std::string(name));
}

void Config::clear() {
    user_config_.clear();
}

bool Config::is_valid_variable_name(std::string_view name) {
    if (name.empty()) {
        return false;
    }

    // Must start with letter or underscore (bash rules)
    if (!std::isalpha(static_cast<unsigned char>(name[0])) && name[0] != '_') {
        return false;
    }

    // Rest must be alphanumeric or underscore
    for (std::size_t i = 1; i < name.size(); ++i) {
        char ch = name[i];
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') {
            return false;
        }
    }

    return true;
}

std::string_view Config::trim(std::string_view str) {
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
