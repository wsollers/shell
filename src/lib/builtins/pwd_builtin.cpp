// ...existing code...
#include <iostream>
#include "shell/platform.h"
#include "shell/built_ins.hpp"
namespace wshell {

int PwdBuiltin::invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) {
        (void)args;
        (void)ctx;
    auto cwd = wshell::get_current_directory();
    if (cwd) {
        std::cout << *cwd << std::endl;
        return 0;
    } else {
        std::cerr << "pwd: failed to get current directory\n";
        return 1;
    }
}

} // namespace wshell
