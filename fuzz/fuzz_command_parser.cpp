// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause


#include <cstdint>
#include <cstddef>
#include <string_view>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    /*
    std::string_view input(reinterpret_cast<const char*>(data), size);
    
    try {
        // Fuzz the tokenize function
        auto tokens = wshell::CommandParser::tokenize(input);
        
        // Exercise the result to ensure the parsing is complete
        volatile size_t total_length = 0;
        for (const auto& token : tokens) {
            total_length += token.length();
        }
        (void)total_length;
    } catch (...) {
        return -1;
    }
    
    try {
        // Fuzz the trim function
        auto trimmed = wshell::CommandParser::trim(input);
        
        // Exercise the result
        volatile size_t trimmed_length = trimmed.length();
        (void)trimmed_length;
    } catch (...) {
        return -1;
    }
    */
    return 0;
}