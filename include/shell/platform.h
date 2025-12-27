#pragma once

#include <string>
#include <optional>

namespace wshell {

// Change current working directory
bool set_current_directory(const std::string& path);

// Get current working directory
std::optional<std::string> get_current_directory();

// Terminate a process by ID
bool terminate_process(int pid);


// Get home directory as string (platform-specific)
std::optional<std::string> get_home_directory();

// Get home directory as filesystem::path (platform-agnostic)
std::optional<std::filesystem::path> get_home_directory_path();

} // namespace wshell
