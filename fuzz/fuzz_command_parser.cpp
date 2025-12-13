// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "../src/lib/command_parser.h"
#include <cstdint>
#include <cstddef>
#include <string_view>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string_view input(reinterpret_cast<const char*>(data), size);
    
    try {
        auto tokens = wshell::CommandParser::tokenize(input);
        for (const auto& token : tokens) {
            volatile auto len = token.length();
            (void)len;
        }
    } catch (...) {
        return -1;
    }
    
    try {
        auto trimmed = wshell::CommandParser::trim(input);
        volatile auto len = trimmed.length();
        (void)len;
    } catch (...) {
        return -1;
    }
    
    return 0;
}
