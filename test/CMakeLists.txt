add_executable(wshell_tests
    test_shell_core.cpp
    test_command_parser.cpp
    test_parser.cpp
    test_config.cpp
)

if(TARGET GTest::gtest)
    target_link_libraries(wshell_tests PRIVATE wshell_lib shell_core GTest::gtest GTest::gtest_main)
else()
    target_link_libraries(wshell_tests PRIVATE wshell_lib shell_core gtest gtest_main)
endif()

# Fix MSVC CI: make sure wshell_tests.exe can find wshell.dll during discovery
if (WIN32)
  add_custom_command(TARGET wshell_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:wshell_lib>
            $<TARGET_FILE_DIR:wshell_tests>
    COMMENT "Copy wshell.dll next to wshell_tests.exe for test discovery"
  )
endif()

include(GoogleTest)
gtest_discover_tests(wshell_tests)
