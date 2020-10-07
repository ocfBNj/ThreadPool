cmake_minimum_required(VERSION 3.17.5)
project(ThreadPool)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ThreadPool ThreadPool.cpp)

add_executable(TestThreadPool main.cpp)
target_link_libraries(TestThreadPool PRIVATE ThreadPool)
