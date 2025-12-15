// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#ifndef WSHELL_COMMAND_PARSER_H
#define WSHELL_COMMAND_PARSER_H

#include <expected>
#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
    #ifdef WSHELL_EXPORTS
        #define WSHELL_API __declspec(dllexport)
    #else
        #define WSHELL_API __declspec(dllimport)
    #endif
#else
    #define WSHELL_API __attribute__((visibility("default")))
#endif

namespace wshell {

using Token = std::string;
using Tokens = std::vector<Token>;

/// @brief Command parsing utilities
class WSHELL_API CommandParser {
public:
    /// @brief Tokenize a command string into individual tokens
    /// @param input The input string to tokenize
    /// @return An expected containing tokens or an error message
    static std::expected<Tokens, std::string> tokenize(std::string_view input);

private:
    /// @brief Trim whitespace from both ends of a string
    /// @param str The string to trim
    /// @return The trimmed string
    static Token trim(std::string_view str);
};

} // namespace wshell

#endif // WSHELL_COMMAND_PARSER_H
