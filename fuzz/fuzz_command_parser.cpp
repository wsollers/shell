// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "command_parser.h"
#include <algorithm>
#include <cctype>
#include <ranges>

namespace wshell {

std::vector<std::string> CommandParser::tokenize(std::string_view input) {
    std::vector<std::string> tokens;
    
    if (input.empty()) {
        return tokens;
    }
    
    std::string current;
    bool in_quotes = false;
    
    for (char c : input) {
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (std::isspace(static_cast<unsigned char>(c)) && !in_quotes) {
            if (!current.empty()) {
                tokens.push_back(std::move(current));
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        tokens.push_back(std::move(current));
    }
    
    return tokens;
}

std::string_view CommandParser::trim(std::string_view str) noexcept {
    // Find first non-whitespace
    auto start = std::ranges::find_if(str, [](unsigned char c) {
        return !std::isspace(c);
    });
    
    if (start == str.end()) {
        return {};
    }
    
    // Find last non-whitespace
    #if defined(_LIBCPP_VERSION)
    // libc++ with Clang has full support for std::ranges::views::reverse
    auto end = std::ranges::find_if(str | std::ranges::views::reverse, [](unsigned char c) {
        return !std::isspace(c);
    }).base();
    #else
    // libstdc++ or MSVC: iterate backwards manually
    auto end = str.end();
    while (end != start) {
        --end;
        if (!std::isspace(static_cast<unsigned char>(*end))) {
            ++end;  // Move one past the last non-whitespace
            break;
        }
    }
    #endif
    
    return str.substr(std::distance(str.begin(), start), 
                      std::distance(start, end));
}

} // namespace wshell