cmake_minimum_required(VERSION 3.0.0)
project(ThreadPool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ThreadPool ThreadPool.cpp)

if (UNIX)
    target_link_libraries(ThreadPool pthread)
endif()

add_executable(TestThreadPool main.cpp)
target_link_libraries(TestThreadPool PRIVATE ThreadPool)
