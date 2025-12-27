#if defined(_WIN32)
#include "shell/platform.h"
#include <windows.h>
#include <string>
#include <optional>
#include <direct.h>

namespace wshell {

bool set_current_directory(const std::string& path) {
    return SetCurrentDirectoryA(path.c_str()) != 0;
}

std::optional<std::string> get_current_directory() {
    char buf[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, buf)) {
        return std::string(buf);
    }
    return std::nullopt;
}

bool terminate_process(int pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (!hProcess) return false;
    BOOL result = TerminateProcess(hProcess, 1);
    CloseHandle(hProcess);
    return result != 0;
}

std::optional<std::string> get_home_directory() {
    char* home = nullptr;
    size_t len = 0;
    if (_dupenv_s(&home, &len, "USERPROFILE") == 0 && home != nullptr) {
        std::string result(home);
        free(home);
        return result;
    }
    return std::nullopt;
}

} // namespace wshell
#endif
