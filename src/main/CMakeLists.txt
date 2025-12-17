# ============================
# Generate version header
# ============================
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.hpp"
    @ONLY
)



add_executable(wshell main.cpp)

target_link_libraries(wshell PRIVATE wshell_lib shell_core)

target_include_directories(wshell
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)


set_target_properties(wshell PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
