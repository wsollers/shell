// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/parser.hpp"

namespace shell {

// Parser implementation
// TODO: Implement parsing logic
                .err = ParseError{
                    .pos = toks[i].pos,
                    .msg = "Cannot parse: lexer error - " + toks[i].error_msg,
                    .where = std::source_location::current()
                }
            };
        }
    }

    // Fail-safe stub: Always return "not implemented" error
    // This ensures no undefined behavior or security vulnerabilities
    return ParseResult{
        .arena = Arena{},
        .seq = Sequence{},
        .err = ParseError{
            .pos = 0,
            .msg = "Parser implementation not yet available - fail-safe stub active",
            .where = std::source_location::current()
        }
    };
}

} // namespace shell