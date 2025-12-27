// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

// Platform-specific executor implementation for Windows

#if defined(_WIN32)


    #define _CRT_SECURE_NO_WARNINGS
    #include "shell/execution_policy.hpp"

    #include <sstream>
    #include <windows.h>
    #include <iostream>

namespace wshell {



void printWindowsErrMsg(DWORD& error) {
    error = GetLastError();
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

    // Display the error
    fprintf(stderr, "%s\n", (LPTSTR)lpMsgBuf);

    // Free the buffer allocated by FormatMessage
    LocalFree(lpMsgBuf);
}

ExecutionResult PlatformExecutionPolicy::execute(const Command& cmd) const {
    // Build command line (Windows uses a single string, not argv array)
    std::ostringstream cmdline;
    cmdline << cmd.executable.string();
    for (const auto& arg : cmd.args) {
        cmdline << " " << arg.value;
    }
    std::string cmdline_str = cmdline.str();

    // Setup process startup info
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION pi{};

    // Create process
    // Note: CreateProcessA modifies the command line, so we need a mutable copy
    std::vector<char> cmdline_buf(cmdline_str.begin(), cmdline_str.end());
    cmdline_buf.push_back('\0');

    BOOL success = CreateProcessA(nullptr,             // Application name (use command line)
                                  cmdline_buf.data(),  // Command line
                                  nullptr,             // Process security attributes
                                  nullptr,             // Thread security attributes
                                  TRUE,                // Inherit handles
                                  0,                   // Creation flags
                                  nullptr,             // Environment
                                  nullptr,             // Current directory
                                  &si,                 // Startup info
                                  &pi                  // Process information
    );

    if (!success) {
        DWORD error;

        printWindowsErrMsg(error);

        return ExecutionResult{.exit_code = platform::EXIT_FAILURE_STATUS,
                               .error_message = "Failed to create process (error " +
                                                std::to_string(error) + ")"};
    }

    // Wait for process to complete
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Get exit code
    DWORD exit_code = platform::EXIT_FAILURE_STATUS;
    GetExitCodeProcess(pi.hProcess, &exit_code);

    // Clean up handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return ExecutionResult{.exit_code = static_cast<int>(exit_code), .error_message = std::nullopt};
}

ExecutionResult PlatformExecutionPolicy::execute(const Pipeline& pipeline) const {
    // Phase 1: No pipeline support yet - just execute first command
    if (pipeline.empty()) {
        return ExecutionResult{.exit_code = platform::EXIT_FAILURE_STATUS,
                               .error_message = "Empty pipeline"};
    }

    // For now, just execute the first command
    // Phase 2 will add proper pipeline support
    return execute(pipeline.commands[0]);
}

void PlatformExecutionPolicy::init_job_control() const {
    // Windows doesn't have POSIX-style job control
    // Would use Job Objects here if needed
}

}  // namespace wshell

#endif  // _WIN32
