if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(WARNING "Fuzzing requires Clang compiler, skipping fuzz tests")
  return()
endif()

# Common fuzz flags (target-scoped)
set(FUZZ_SANITIZERS "fuzzer,address,undefined")

function(configure_fuzz_target tgt)
  # C++23 for std::expected and friends
  target_compile_features(${tgt} PRIVATE cxx_std_23)

  # Debug symbols help triage crashes even in Release-ish CI
  target_compile_options(${tgt} PRIVATE -g)

  # Use Clang sanitizers + libFuzzer
  target_compile_options(${tgt} PRIVATE -fsanitize=${FUZZ_SANITIZERS})
  target_link_options(${tgt} PRIVATE -fsanitize=${FUZZ_SANITIZERS})

  # Prefer libc++ with Clang on Linux (matches your workflow install)
  target_compile_options(${tgt} PRIVATE -stdlib=libc++)
  target_link_options(${tgt} PRIVATE -stdlib=libc++ -lc++abi)

  # Avoid PIE for fuzzers (also prevents -pie getting treated as a compile flag somewhere)
  target_link_options(${tgt} PRIVATE -no-pie)

  # Optional: make sanitizer reports more readable
  target_compile_options(${tgt} PRIVATE -fno-omit-frame-pointer)
endfunction()

# Fuzz test for command parser
add_executable(fuzz_command_parser fuzz_command_parser.cpp)
target_link_libraries(fuzz_command_parser PRIVATE wshell_lib)
configure_fuzz_target(fuzz_command_parser)

# Fuzz test for shell core
add_executable(fuzz_shell_core fuzz_shell_core.cpp)
target_link_libraries(fuzz_shell_core PRIVATE wshell_lib)
configure_fuzz_target(fuzz_shell_core)
