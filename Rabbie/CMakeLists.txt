cmake_minimum_required(VERSION 3.16)
project(Rabbie)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Rabbie
        Client.cpp
        Queue.cpp
        Server.cpp)
