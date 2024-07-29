include(FetchContent)
FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG v1.15.0)
FetchContent_MakeAvailable(googletest)

add_executable(broka_test order_test.cpp order_book_test.cpp)

target_include_directories(broka_test PRIVATE ${CMAKE_SOURCE_DIR}/include/broka)

target_link_libraries(broka_test PRIVATE broka_lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(broka_test)
