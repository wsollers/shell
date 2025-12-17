# Library source files
add_library(wshell_lib SHARED
    lexer.cpp
    parser.cpp
)

# Platform-specific executor implementations
if(WIN32)
    target_sources(wshell_lib PRIVATE executor_win32.cpp)
elseif(UNIX)
    target_sources(wshell_lib PRIVATE executor_posix.cpp)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Set library properties
set_target_properties(wshell_lib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME wshell
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