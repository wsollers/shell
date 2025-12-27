#include <iostream>
#include <unistd.h>
#include <cstdlib>

#include "shell/built_ins.hpp"


namespace wshell {

int CdBuiltin::invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) {
    const char* dir = nullptr;
    if (args.size() < 2) {
        dir = std::getenv("HOME");
        if (!dir) dir = ".";
    } else {
        dir = args[1].c_str();
    }
    if (chdir(dir) != 0) {
        std::perror("cd");
        return 1;
    }
    return 0;
}

} // namespace wshell
