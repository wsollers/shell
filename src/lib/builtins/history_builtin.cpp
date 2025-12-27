#include <iostream>
#include <vector>

#include "shell/built_ins.hpp"
#include "shell/history.hpp"


namespace wshell {

HistoryBuiltin::HistoryBuiltin() {}

int HistoryBuiltin::invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) {
    return 1; //placeholder
}

} // namespace wshell
