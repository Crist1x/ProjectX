enable_testing()

add_executable(run_tests
        test_menu.cpp
        test_db.cpp
        test_alg.cpp
        ../src/algorithm/alg.cpp
)

target_include_directories(run_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(run_tests PRIVATE GTest::gtest_main menu_lib db_lib)


include(GoogleTest)
gtest_discover_tests(run_tests)
