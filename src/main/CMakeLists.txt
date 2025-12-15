add_executable(wshell main.cpp)
target_link_libraries(wshell PRIVATE wshell_lib shell_core)
set_target_properties(wshell PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
