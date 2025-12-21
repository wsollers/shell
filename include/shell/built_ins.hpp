#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <cstddef>
#include <utility>
#include <string_view>
#include <array>
#include <set>




namespace wshell {

constexpr std::array<std::string_view, 7> builtinCommands = {
    "cd", "exit", "export", "history", "pwd", "set", "unset"
};

constexpr std::array<std::pair<std::string_view, std::string_view>, 4> builtinVariables = {{
    {"PS1", "8=> "},
    {"PS2", ": "},
    {"HISTORY_SIZE", "10"},
    {"SHELL", "/bin/wshell"}
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
};
}