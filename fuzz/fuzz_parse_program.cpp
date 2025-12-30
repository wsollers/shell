#include <cstddef>
#include <cstdint>
#include <string_view>
#include <string>
#include "shell/parser.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size == 0) return 0;
    std::string input(reinterpret_cast<const char*>(data), size);
    try {
        auto result = wshell::parse_program(input);
    } catch (...) {
        // Swallow all exceptions
    }
    return 0;
}
