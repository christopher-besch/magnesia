add_executable(magnesia_test example.cpp)
target_link_libraries(magnesia_test GTest::gtest_main magnesia_lib)

include(GoogleTest)
gtest_discover_tests(magnesia_test)
