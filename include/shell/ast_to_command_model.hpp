// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include "shell/ast.hpp"
#include "shell/command_model.hpp"

#include <optional>

namespace wshell {

// Convert AST Word to command model ShellArg
inline ShellArg ast_word_to_model(const Word& w) {
    return ShellArg{w.text, w.quoted, w.needs_expansion};
}

// Convert AST Redirection to command model IO (FileTarget)
inline IO ast_redir_to_model(const Redirection& r) {
    // Only file redirections for now
    OpenMode mode = OpenMode::WriteTruncate;
    switch (r.kind) {
        case RedirectKind::Input:
            mode = OpenMode::Read;
            break;
        case RedirectKind::OutputTruncate:
            mode = OpenMode::WriteTruncate;
            break;
        case RedirectKind::OutputAppend:
            mode = OpenMode::WriteAppend;
            break;
    }
    return FileTarget{r.target.text, mode};
}

// Convert AST CommandNode to command model Command
inline Command ast_cmd_to_model(const CommandNode& node) {
    Command cmd;
    cmd.executable = node.command_name.text;
    for (const auto& arg : node.arguments) {
        cmd.args.push_back(ast_word_to_model(arg));
    }
    for (const auto& redir : node.redirections) {
        // Only handle stdout redirection for now
        if (redir.kind == RedirectKind::OutputTruncate ||
            redir.kind == RedirectKind::OutputAppend) {
            cmd.stdout_ = ast_redir_to_model(redir);
        } else if (redir.kind == RedirectKind::Input) {
            cmd.stdin_ = ast_redir_to_model(redir);
        }
    }
    return cmd;
}

}  // namespace wshell
