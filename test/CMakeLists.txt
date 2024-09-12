# gtests fail on windows
if(WIN32)
    return()
endif()

add_executable(magnesia_test storage.cpp)
target_link_libraries(magnesia_test GTest::gtest_main magnesia_lib)

include(GoogleTest)
gtest_discover_tests(magnesia_test)
