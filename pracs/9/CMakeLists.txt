cmake_minimum_required(VERSION 3.17.0)

project(TCP-Server-Client VERSION 0.1)

# Configure compiler
set(CMAKE_CXX_COMPILER "/usr/bin/g++-11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a")

# Requirements
find_package(Threads REQUIRED)

# Server variables
set(SERVER_SRC
    src/server/network_exceptions.cpp
    src/server/server.cpp
    src/server/client.cpp
    src/server/example.cpp
)

set(SERVER_HEAD
    src/server/network_exceptions.hpp
    src/server/color_codes.hpp
    src/server/server.hpp
    src/server/client.hpp
)

add_executable(server_example ${SERVER_SRC})
