// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "command_parser.h"
#include <algorithm>
#include <cctype>
#include <ranges>

namespace wshell {

std::expected<Tokens, std::string> CommandParser::tokenize(std::string_view input) {
    return std::unexpected("Unimplemented: CommandParser::tokenize");
}

Token CommandParser::trim(std::string_view str) {
    auto start = std::ranges::find_if(str, [](unsigned char ch) {
        return !std::isspace(ch);
    });

    if (start == str.end()) {
        return "";
    }

    auto end = std::ranges::find_if(str | std::views::reverse, [](unsigned char ch) {
        return !std::isspace(ch);
    }).base();

    return Token(start, end);
}

} // namespace wshell
