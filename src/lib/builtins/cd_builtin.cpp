
#include <iostream>
#include "shell/built_ins.hpp"
#include "shell/platform.h"


namespace wshell {

int CdBuiltin::invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) {
        (void)args;
        (void)ctx;
    std::string dir;
    if (args.size() < 2) {
        auto home = wshell::get_home_directory();
        dir = home.value_or(".");
    } else {
        dir = args[1];
    }
    if (!wshell::set_current_directory(dir)) {
        std::cerr << "cd: failed to change directory to '" << dir << "'\n";
        return 1;
    }
    return 0;
}

} // namespace wshell
