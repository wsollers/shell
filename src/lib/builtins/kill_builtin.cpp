#include <iostream>
#include <csignal>
#include <cstdlib>

#include "shell/built_ins.hpp"

namespace wshell {

int KillBuiltin::invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) {
    if (args.size() < 2) {
        std::cerr << "kill: missing pid" << std::endl;
        return 1;
    }
    int sig = SIGTERM;
    size_t argi = 1;
    if (args[1].rfind("-", 0) == 0) {
        sig = std::atoi(args[1].c_str() + 1);
        argi = 2;
    }
    if (args.size() <= argi) {
        std::cerr << "kill: missing pid" << std::endl;
        return 1;
    }
    //TODO make this work for Windows
    pid_t pid = std::atoi(args[argi].c_str());
    if (kill(pid, sig) != 0) {
        std::perror("kill");
        return 1;
    }
    return 0;
}

} // namespace wshell
