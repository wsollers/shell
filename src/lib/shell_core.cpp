// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell_core.h"
#include <algorithm>
#include <cctype>

namespace wshell {

std::expected<int, ShellError> ShellCore::execute(std::string_view command) noexcept {
    if (command.empty()) {
        return std::unexpected(ShellError::InvalidCommand);
    }
    
    if (!validateCommand(command)) {
        return std::unexpected(ShellError::InvalidArgument);
    }
    
    return 0;
}

bool ShellCore::validateCommand(std::string_view command) noexcept {
    if (command.empty()) {
        return false;
    }
    
    if (command.find('\0') != std::string_view::npos) {
        return false;
    }
    
    return std::ranges::any_of(command, [](char c) {
        return !std::isspace(static_cast<unsigned char>(c));
    });
}

} // namespace wshell
