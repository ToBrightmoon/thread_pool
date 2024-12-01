cmake_minimum_required(VERSION 3.10.0)
project(thread_pool_test)

set(CMAKE_CXX_STANDARD 17)

add_executable(thread_pool_test thread_pool_test.cpp task.cpp worker.cpp)