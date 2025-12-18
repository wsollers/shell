#include "shell/lexer.hpp"
#include "shell/parser.hpp"
#include <cstddef>
#include <cstdint>
#include <string_view>

using namespace shell;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {

  return 0;
}
