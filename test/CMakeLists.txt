add_executable(wshell_tests
    test_shell_core.cpp
    test_command_parser.cpp
)

if(TARGET GTest::gtest)
    target_link_libraries(wshell_tests PRIVATE wshell_lib GTest::gtest GTest::gtest_main)
else()
    target_link_libraries(wshell_tests PRIVATE wshell_lib gtest gtest_main)
endif()

include(GoogleTest)
gtest_discover_tests(wshell_tests)
