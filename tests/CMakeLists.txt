enable_testing()

add_executable(run_tests
        test_menu.cpp
)

target_include_directories(run_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(run_tests PRIVATE
        gtest_main
        spdlog::spdlog
        menu_lib
        db_lib
        logger_lib
)


include(GoogleTest)
gtest_discover_tests(run_tests)