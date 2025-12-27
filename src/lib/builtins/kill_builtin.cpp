// ...existing code...
#include <iostream>
#include "shell/platform.h"
#include "shell/built_ins.hpp"

namespace wshell {

int KillBuiltin::invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) {
    if (args.size() < 2) {
        std::cerr << "kill: missing pid" << std::endl;
        return 1;
    }
    int pid = std::atoi(args[1].c_str());
    if (!wshell::terminate_process(pid)) {
        std::cerr << "kill: failed to terminate process " << pid << std::endl;
        return 1;
    }
    return 0;
}

} // namespace wshell
