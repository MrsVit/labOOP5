cmake_minimum_required(VERSION 3.20)

project(OOP5 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(memorylist_app body/memorylist.cpp)
target_include_directories(memorylist_app PRIVATE head)


add_executable(test_list tests/test_list.cpp)
target_include_directories(test_list PRIVATE head)
target_link_libraries(test_list PRIVATE GTest::gtest_main)

add_executable(test_memorylist tests/test_Memorylist.cpp)
target_include_directories(test_memorylist PRIVATE head)
target_link_libraries(test_memorylist PRIVATE GTest::gtest_main)

file(GLOB_RECURSE HEADER_FILES "head/*.hpp")
add_library(headers INTERFACE)
target_include_directories(headers INTERFACE head)
target_sources(headers INTERFACE ${HEADER_FILES})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

