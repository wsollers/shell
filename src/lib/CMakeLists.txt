# Library source files
file(GLOB_RECURSE WSHELL_SOURCES_PARSER
        ${CMAKE_CURRENT_SOURCE_DIR}/parser/*.cpp
)

file(GLOB_RECURSE WSHELL_SOURCES_EXECUTOR
        ${CMAKE_CURRENT_SOURCE_DIR}/executor/*.cpp
)

add_library(wshell_lib STATIC
    ${WSHELL_SOURCES_PARSER}
    ${WSHELL_SOURCES_EXECUTOR}
        ../../include/shell/ast_printer.hpp
        ast/ast_printer.cpp
        executor/shell_process_context.cpp
)


# Platform-specific executor implementations
if(WIN32)
    target_sources(wshell_lib PRIVATE executor/executor_win32.cpp)
elseif(UNIX)
    target_sources(wshell_lib PRIVATE executor/executor_posix.cpp)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

target_include_directories(wshell_lib PUBLIC ../../include)

# Apply security/hardening flags to the new library
target_compile_options(wshell_lib PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# Set library properties
set_target_properties(wshell_lib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME wshell_lib
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Include directories
target_include_directories(wshell_lib
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Export symbols on Windows
if(WIN32)
    target_compile_definitions(wshell_lib PRIVATE WSHELL_EXPORTS)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)