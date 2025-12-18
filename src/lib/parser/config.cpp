// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/config.hpp"
#include "shell/input_source.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#ifdef _WIN32
#include <stdlib.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

namespace wshell {

//==============================================================================
// Validation Policy Implementations
//==============================================================================

bool DefaultValidationPolicy::is_valid_name(std::string_view name) {
    if (name.empty()) {
        return false;
    }

    // Must start with letter or underscore (bash rules)
    if (!std::isalpha(static_cast<unsigned char>(name[0])) && name[0] != '_') {
        return false;
    }

    // Rest must be alphanumeric or underscore
    for (std::size_t i = 1; i < name.size(); ++i) {
        char ch = name[i];
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') {
            return false;
        }
    }

    return true;
}

bool DefaultValidationPolicy::check_limits(std::size_t name_len,
                                           std::size_t value_len,
                                           std::size_t var_count) {
    return name_len <= MAX_NAME_LENGTH && 
           value_len <= MAX_VALUE_LENGTH &&
           var_count < MAX_VARIABLE_COUNT;
}

bool StrictValidationPolicy::is_valid_name(std::string_view name) {
    // Use same validation as default policy
    return DefaultValidationPolicy::is_valid_name(name);
}

bool StrictValidationPolicy::check_limits(std::size_t name_len,
                                          std::size_t value_len,
                                          std::size_t var_count) {
    return name_len <= MAX_NAME_LENGTH && 
           value_len <= MAX_VALUE_LENGTH &&
           var_count < MAX_VARIABLE_COUNT;
}

template<typename ValidationPolicy>
void Config<ValidationPolicy>::showEnvironmentVariables() {
    std::ranges::for_each(variables_, [](auto const& kv) {
        std::cout << std::format("{:<20} = {}\n", kv.first, kv.second);
    });
}

//==============================================================================
// Static Helper Functions
//==============================================================================

template<typename ValidationPolicy>
std::filesystem::path Config<ValidationPolicy>::default_config_path() {
    std::filesystem::path home;
    
#ifdef _WIN32
    // Use Windows-safe _dupenv_s
    char* userprofile = nullptr;
    size_t len = 0;
    if (_dupenv_s(&userprofile, &len, "USERPROFILE") == 0 && userprofile) {
        home = userprofile;
        free(userprofile);
    }
#else
    // Try HOME first, fall back to getpwuid
    char const* home_env = std::getenv("HOME");
    if (home_env) {
        home = home_env;
    } else {
        // Use getpwuid as fallback
        passwd* pw = getpwuid(getuid());
        if (pw && pw->pw_dir) {
            home = pw->pw_dir;
        }
    }
#endif

    if (home.empty()) {
        // Last resort fallback
        home = std::filesystem::current_path();
    }

    return home / ".wshellrc";
}

//==============================================================================
// Explicit Template Instantiations
//==============================================================================

// Instantiate the commonly used configurations
template class Config<DefaultValidationPolicy>;
template class Config<StrictValidationPolicy>;

} // namespace shell
