// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#ifndef WSHELL_COMMAND_PARSER_H
#define WSHELL_COMMAND_PARSER_H

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

/// @brief Command parsing utilities
class WSHELL_API CommandParser {
public:
    /// @brief Parse a command line into tokens
    /// @param input Command line input
    /// @return Vector of tokens
    [[nodiscard]] static std::vector<std::string> tokenize(std::string_view input);
    
    /// @brief Remove leading and trailing whitespace
    /// @param str String to trim
    /// @return Trimmed string view
    [[nodiscard]] static std::string_view trim(std::string_view str) noexcept;
};

} // namespace wshell

#endif // WSHELL_COMMAND_PARSER_H
