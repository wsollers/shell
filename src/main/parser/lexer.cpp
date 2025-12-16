// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/lexer.hpp"

namespace shell {

// Lexer implementation
// TODO: Implement lexical analysis
        .text = {},
        .error_msg = "Lexer implementation not yet available - fail-safe stub active"
    };

    return LexResult{
        .toks = {std::move(error_token)},
        .ok = false
    };
}

} // namespace shell