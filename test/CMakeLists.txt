if(ENABLE_TESTING)
    add_executable(wshell_tests
        test_command_parser.cpp
            ../src/lib/ast/ast_printer.cpp
    )

    target_include_directories(wshell_tests
            PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../include/shell
    )

    if(TARGET GTest::gtest)
        target_link_libraries(wshell_tests PRIVATE wshell_lib GTest::gtest GTest::gtest_main)
    else()
        target_link_libraries(wshell_tests PRIVATE wshell_lib gtest gtest_main)
    endif()

    target_link_libraries(wshell_tests PRIVATE
            wshell_lib
    )

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
endif()