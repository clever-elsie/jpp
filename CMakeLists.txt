cmake_minimum_required(VERSION 3.16)

project(jpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(test_runner src/test_runner.cpp)
target_include_directories(test_runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

enable_testing()
add_test(NAME json_tests
         COMMAND test_runner
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})


