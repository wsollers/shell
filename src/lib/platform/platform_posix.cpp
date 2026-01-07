#if defined(__unix__) || defined(__APPLE__)
#include "shell/platform.h"
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <pwd.h>
#include <limits.h>
#include <cstdlib>
#include <filesystem>

namespace wshell {

bool set_current_directory(const std::string& path) {
    return chdir(path.c_str()) == 0;
}

std::optional<std::string> get_current_directory() {
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf))) {
        return std::string(buf);
    }
    return std::nullopt;
}

bool terminate_process(int pid) {
    return kill(pid, SIGTERM) == 0;
}


std::optional<std::string> get_home_directory() {
    const char* home = getenv("HOME");
    if (home) return std::string(home);
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) return std::string(pw->pw_dir);
    return std::nullopt;
}

std::optional<std::filesystem::path> get_home_directory_path() {
    auto home = get_home_directory();
    if (home) {
        return std::filesystem::path(*home);
    }
    return std::nullopt;
}

} // namespace wshell
#endif
