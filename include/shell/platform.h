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

// Get home directory
std::optional<std::string> get_home_directory();

} // namespace wshell
