cmake_minimum_required(VERSION 3.14)
project(ConcurrencyInAction)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

include(FetchContent)
fetchcontent_declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
fetchcontent_makeavailable(googletest)

add_executable(test_stack tests/test_thread_safe_stack.cpp)
target_include_directories(test_stack PRIVATE include)

target_link_libraries(test_stack PRIVATE gtest gtest_main Threads::Threads)

include(GoogleTest)
gtest_discover_tests(test_stack)

enable_testing()
