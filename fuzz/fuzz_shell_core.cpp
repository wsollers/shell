// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "../src/lib/shell_core.h"
#include <cstdint>
#include <cstddef>
#include <string_view>
#include <expected>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    /*
    std::string_view input(reinterpret_cast<const char*>(data), size);
    
    try {
        volatile bool valid = wshell::ShellCore::validateCommand(input);
        (void)valid;
    } catch (...) {
        return -1;
    }
    
    try {
        wshell::ShellCore shell;
        auto result = shell.execute(input);
        volatile bool has_value = result.has_value();
        (void)has_value;
    } catch (...) {
        return -1;
    }
    */
    return 0;
}
