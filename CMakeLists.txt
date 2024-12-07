cmake_minimum_required(VERSION 3.10.0)
project(THREAD_POOL)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB CPP_LIST "src/*.cpp")

set(SRC_LIST ${CPP_LIST})

add_executable(base_test test/thread_pool_base_test.cpp ${SRC_LIST})

add_executable(handle_test test/thread_pool_handle_test.cpp ${SRC_LIST})

add_executable(priority_test test/thread_pool_priority_test.cpp ${SRC_LIST})

add_executable(adjust_thread_num test/thread_pool_adjust_thread_test.cpp ${SRC_LIST})