cmake_minimum_required(VERSION 3.17.0)

project(TCP-Server-Client VERSION 0.1)

# Configure compiler
set(CMAKE_CXX_COMPILER "/usr/bin/g++-11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -g")

if (USE_IPv6)
    add_compile_definitions(USE_IPv6)
    message(WARNING "Building using IPv6 addresses")
endif()

# Requirements
find_package(Threads REQUIRED)

# Server variables
set(SERVER_SRC
    src/network_exceptions.cpp
    src/server/server.cpp
    src/server/client.cpp
    src/server/example.cpp
)

set(SERVER_HEAD
    include/network_exceptions.hpp
    include/status.hpp
    include/server/server.hpp
    include/server/client.hpp
)

# Client variables
set(CLIENT_SRC
    src/network_exceptions.cpp
    src/client/client.cpp
    src/client/example.cpp
)

set(CLIENT_HEAD
    include/status.hpp
    include/client/client.hpp
    include/network_exceptions.hpp
)

add_executable(server_example ${SERVER_SRC})
add_executable(client_example ${CLIENT_SRC})
target_include_directories(server_example PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(client_example PUBLIC ${PROJECT_SOURCE_DIR}/include)
