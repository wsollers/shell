// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

// platform_types.hpp - Cross-platform abstractions for OS-specific types
#pragma once

// Platform detection
#ifdef _WIN32
    #define WSHELL_PLATFORM_WINDOWS
    #include <windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
    #define WSHELL_PLATFORM_MACOS
    #include <sys/types.h>
    #include <unistd.h>
#elif defined(__linux__)
    #define WSHELL_PLATFORM_LINUX
    #include <sys/types.h>
    #include <unistd.h>
#else
    #error "Unsupported platform"
#endif

namespace wshell::platform {

// ============================================================================
// Process Handle Abstraction
// ============================================================================

#ifdef WSHELL_PLATFORM_WINDOWS
    /// Windows: Use HANDLE for processes and Job Objects
    using ProcessHandle = HANDLE;
    inline const ProcessHandle INVALID_PROCESS = INVALID_HANDLE_VALUE;
    
    /// Windows uses DWORD for process IDs
    using ProcessId = DWORD;
    inline constexpr ProcessId INVALID_PROCESS_ID = 0;
    
#else  // macOS and Linux (POSIX)
    /// POSIX: Use pid_t for processes
    using ProcessHandle = pid_t;
    inline constexpr ProcessHandle INVALID_PROCESS = -1;
    
    /// POSIX: pid_t for process IDs
    using ProcessId = pid_t;
    inline constexpr ProcessId INVALID_PROCESS_ID = -1;
#endif

// ============================================================================
// Process Group Abstraction (Job Control)
// ============================================================================

/// Cross-platform process group abstraction
/// Windows: Uses Job Objects (HANDLE)
/// POSIX: Uses Process Group IDs (pid_t)
/// Interface is identical across platforms - implementation details are hidden
struct ProcessGroup {
#ifdef WSHELL_PLATFORM_WINDOWS
    HANDLE grpHandle{INVALID_HANDLE_VALUE};
#else
    pid_t grpHandle{-1};
#endif

    // Common interface - identical across all platforms
    
    [[nodiscard]] bool is_valid() const noexcept {
#ifdef WSHELL_PLATFORM_WINDOWS
        return grpHandle != INVALID_HANDLE_VALUE && grpHandle != nullptr;
#else
        return grpHandle > 0;
#endif
    }
    
    void invalidate() noexcept {
#ifdef WSHELL_PLATFORM_WINDOWS
        if (is_valid() && grpHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(grpHandle);
        }
        grpHandle = INVALID_HANDLE_VALUE;
#else
        grpHandle = -1;
#endif
    }
    
    // Platform-specific semantics (Windows: move-only, POSIX: copyable)
#ifdef WSHELL_PLATFORM_WINDOWS
    // Move-only semantics for Windows (HANDLE is a resource)
    ProcessGroup() = default;
    ProcessGroup(const ProcessGroup&) = delete;
    ProcessGroup& operator=(const ProcessGroup&) = delete;
    
    ProcessGroup(ProcessGroup&& other) noexcept 
        : grpHandle(other.grpHandle) {
        other.grpHandle = INVALID_HANDLE_VALUE;
    }
    
    ProcessGroup& operator=(ProcessGroup&& other) noexcept {
        if (this != &other) {
            invalidate();
            grpHandle = other.grpHandle;
            other.grpHandle = INVALID_HANDLE_VALUE;
        }
        return *this;
    }
    
    ~ProcessGroup() {
        invalidate();
    }
#else
    // Copyable for POSIX (pid_t is just an integer)
    ProcessGroup() = default;
    ProcessGroup(const ProcessGroup&) = default;
    ProcessGroup& operator=(const ProcessGroup&) = default;
    ProcessGroup(ProcessGroup&&) noexcept = default;
    ProcessGroup& operator=(ProcessGroup&&) noexcept = default;
    ~ProcessGroup() = default;
#endif
    
    // Factory method - identical signature across platforms
    [[nodiscard]] static ProcessGroup create([[maybe_unused]] ProcessId id = 0) noexcept {
        ProcessGroup pg;
#ifdef WSHELL_PLATFORM_WINDOWS
        // Windows: Create a Job Object (id parameter ignored)
        pg.grpHandle = CreateJobObjectW(nullptr, nullptr);
#else
        // POSIX: Use provided PID or current process
        pg.grpHandle = (id == 0) ? getpid() : id;
#endif
        return pg;
    }
    
    // Get the underlying platform handle (for advanced usage)
    [[nodiscard]] auto native_handle() const noexcept {
        return grpHandle;
    }
    
#ifndef WSHELL_PLATFORM_WINDOWS
    // POSIX-specific: Set process group for a process
    [[nodiscard]] bool set_for_process(pid_t pid) const noexcept {
        return setpgid(pid, grpHandle) == 0;
    }
#endif
};

// ============================================================================
// Common Job Control Constants
// ============================================================================

/// Invalid job ID constant
inline constexpr int INVALID_JOB_ID = -1;

/// Exit status constants
inline constexpr int EXIT_SUCCESS_STATUS = 0;
inline constexpr int EXIT_FAILURE_STATUS = 1;
inline constexpr int EXIT_SIGNAL_BASE = 128;  // Exit code for signals (128 + signal number)

// ============================================================================
// Platform-specific helpers
// ============================================================================

/// Check if current platform supports job control
[[nodiscard]] inline constexpr bool supports_job_control() noexcept {
#ifdef WSHELL_PLATFORM_WINDOWS
    return true;  // Via Job Objects
#else
    return true;  // Via process groups
#endif
}

/// Get current process ID
[[nodiscard]] inline ProcessId get_current_process_id() noexcept {
#ifdef WSHELL_PLATFORM_WINDOWS
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

/// Get parent process ID
[[nodiscard]] inline ProcessId get_parent_process_id() noexcept {
#ifdef WSHELL_PLATFORM_WINDOWS
    // Windows doesn't have a direct getppid() equivalent
    // Would need to use NtQueryInformationProcess or ToolHelp32
    return INVALID_PROCESS_ID;
#else
    return getppid();
#endif
}

} // namespace wshell::platform
