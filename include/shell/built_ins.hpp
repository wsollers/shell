
#pragma once
#include <string_view>

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "shell_process_context.h"

namespace wshell {
// Interface for all builtin functions

// Interface for all builtin functions
class BuiltinFunction {
  public:
    virtual ~BuiltinFunction() = default;
    // args: arguments passed to the builtin
    // ctx: shell process context (for env, cwd, etc)
    // Returns: exit code (0 = success)
    virtual int invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) = 0;
};

// Implementation for 'cd'
class CdBuiltin : public BuiltinFunction {
  public:
    int invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) override;
};

// Implementation for 'pwd'
class PwdBuiltin : public BuiltinFunction {
  public:
    int invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) override;
};

// Implementation for 'exit'
class ExitBuiltin : public BuiltinFunction {
  public:
    int invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) override;
};

// Implementation for 'kill'
class KillBuiltin : public BuiltinFunction {
  public:
    int invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) override;
};

// Forward declaration for History
class History;

// Implementation for 'history'
class HistoryBuiltin : public BuiltinFunction {
  public:
    explicit HistoryBuiltin();
    int invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) override;

};

// Default built-in shell variables and their values
static const std::unordered_map<std::string, std::string> builtinVariablesDefault = {
    {"PS1", "8=> "}, {"PS2", ": "}, {"HISTORY_SIZE", "100"}, {"SHELL", "/bin/wshell"}};

class BuiltIns {
  public:
    BuiltIns(History* history_ptr = nullptr) {
      [[maybeunused]]history_ptr; // Suppress unused parameter warning (MSVC)
        // Initialize built-in variable map with defaults
        builtinVariables_ = builtinVariablesDefault;
        // Register builtin function implementations
        builtinFunctionMap_["cd"] = std::make_unique<CdBuiltin>();
        builtinFunctionMap_["pwd"] = std::make_unique<PwdBuiltin>();
        builtinFunctionMap_["exit"] = std::make_unique<ExitBuiltin>();
        builtinFunctionMap_["kill"] = std::make_unique<KillBuiltin>();
        builtinFunctionMap_["history"] = std::make_unique<HistoryBuiltin>();

    }

    [[nodiscard]] bool is_builtin_command(const std::string& cmd) const noexcept {
        return builtinFunctionMap_.find(cmd) != builtinFunctionMap_.end();
    }

    [[nodiscard]] std::optional<std::string>
    get_builtin_variable(const std::string& var) const noexcept {
        auto it = builtinVariables_.find(var);
        if (it != builtinVariables_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void set_builtin_variable(const std::string& var, const std::string& value) {
        builtinVariables_[var] = value;
    }

    BuiltinFunction* get_builtin_function(const std::string& cmd) const {
        auto it = builtinFunctionMap_.find(cmd);
        if (it != builtinFunctionMap_.end()) {
            return it->second.get();
        }
        return nullptr;
    }

  private:
    std::unordered_map<std::string, std::string> builtinVariables_;
    std::unordered_map<std::string, std::unique_ptr<BuiltinFunction>> builtinFunctionMap_;
};
}  // namespace wshell