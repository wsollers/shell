#include <cstdlib>

#include "shell/built_ins.hpp"

namespace wshell {

int ExitBuiltin::invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) {
    int code = 0;
    if (args.size() > 1) {
        code = std::atoi(args[1].c_str());
    }
    std::exit(code);
}

} // namespace wshell
