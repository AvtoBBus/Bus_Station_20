cmake_minimum_required(VERSION 3.28)
project(lab3)

find_package(Threads REQUIRED)
link_libraries(Threads::Threads)
set(CMAKE_CXX_STANDARD 17)

add_executable(lab3 main.cpp)
