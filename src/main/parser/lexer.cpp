// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/lexer.hpp"
#include <string_view>
#include <vector>

namespace shell {

/// @brief Fail-safe stub implementation of lexer
/// @details This is a security-first stub that safely rejects all input
/// @param input Input string to tokenize (currently unsupported)
/// @return LexResult with error indicating not implemented
/// @note Following AI coding guidelines: validate input, safe defaults, clear error messages
LexResult Lexer::lex(std::string_view input) const {
    // Input validation: reject null/empty input with clear error
    if (input.empty()) {
        Token error_token{
            .kind = TokKind::ERROR,
            .pos = 0,
            .text = {},
            .error_msg = "Empty input not supported in lexer stub"
        };
        return LexResult{
            .toks = {std::move(error_token)},
            .ok = false
        };
    }

    // Security: Validate input size to prevent potential buffer issues
    // Following AI guidelines: check bounds, prevent overflow
    constexpr std::size_t MAX_INPUT_SIZE = 1'000'000; // 1MB limit
    if (input.size() > MAX_INPUT_SIZE) {
        Token error_token{
            .kind = TokKind::ERROR,
            .pos = 0,
            .text = {},
            .error_msg = "Input too large (exceeds 1MB limit)"
        };
        return LexResult{
            .toks = {std::move(error_token)},
            .ok = false
        };
    }

    // Fail-safe stub: Always return "not implemented" error
    // This ensures no undefined behavior or security vulnerabilities
    Token error_token{
        .kind = TokKind::ERROR,
        .pos = 0,
        .text = {},
        .error_msg = "Lexer implementation not yet available - fail-safe stub active"
    };

    return LexResult{
        .toks = {std::move(error_token)},
        .ok = false
    };
}

} // namespace shell