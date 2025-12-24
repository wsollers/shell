#pragma once

#include <string_view>

#include <algorithm>
#include <array>
#include <cstddef>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "shell_process_context.h"

namespace wshell {

constexpr std::array<std::string_view, 7> builtinCommands = {
    "cd", "exit", "export", "history", "pwd", "set", "unset"
};

constexpr std::array<std::pair<std::string_view, std::string_view>, 4> builtinVariables = {{
    {"PS1", "8=> "},
    {"PS2", ": "},
    {"HISTORY_SIZE", "10"},
    {"SHELL", "/bin/wshell"},
}};



class BuiltIns {
public:
    BuiltIns() {
        for ( const auto& [name, value] : builtinVariables ) {
            builtInvariables_.emplace(name, value);
        }
        for ( const auto& cmd : builtinCommands ) {
            builtinsCommands_.emplace(cmd);
        }
        //add vars for args
        ShellProcessContext ctx = ShellProcessContext();
        for ( int i = 0; i < ctx.argc; i++) {
            auto currentKey = "?" + std::to_string(i);
            builtInvariables_.emplace(currentKey, ctx.argv[i]);
            unmodifiableBuiltinVariables_[currentKey]= ctx.argv[i];

        }
    }

    [[nodiscard]] bool is_builtin_command(const std::string& cmd) const noexcept {
        return builtinsCommands_.find(cmd) != builtinsCommands_.end();
    }

    [[nodiscard]] std::optional<std::string> get_builtin_variable(const std::string& var) const noexcept {
        auto it = builtInvariables_.find(var);
        if ( it != builtInvariables_.end() ) {
            return it->second;
        }
        return std::nullopt;
    }

private:
    std::unordered_map<std::string, std::string> builtInvariables_;
    std::set<std::string> builtinsCommands_;
    std::map<std::string, std::string> unmodifiableBuiltinVariables_ {};
};
}