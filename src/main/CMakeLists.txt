# ============================
# Generate version header
# ============================



add_executable(wshell main.cpp)


target_link_libraries(wshell PRIVATE wshell_lib)

target_include_directories(wshell
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(wshell
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/shell
)


#target_link_libraries(wshell PRIVATE project_version)

set_target_properties(wshell PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
