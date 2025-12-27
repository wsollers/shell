#include <iostream>
#include <unistd.h>
#include <limits.h>

#include "shell/built_ins.hpp"
namespace wshell {

int PwdBuiltin::invoke(const std::vector<std::string>& args, ShellProcessContext& ctx) {
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf))) {
        std::cout << buf << std::endl;
        return 0;
    } else {
        std::perror("pwd");
        return 1;
    }
}

} // namespace wshell
