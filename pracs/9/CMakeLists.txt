cmake_minimum_required(VERSION 3.17.0)

project(TCP-Server-Client VERSION 0.1)

# Configure compiler
set(CMAKE_CXX_COMPILER "/usr/bin/g++-11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -g")
set(DIR_INCLUDE "include")
set(DIR_SOURCE "src")

# Server variables
set(SERVER_SRC
    ${DIR_SOURCE}/network_exceptions.cpp
    ${DIR_SOURCE}/server/server.cpp
    ${DIR_SOURCE}/server/client.cpp
)

set(SERVER_HEAD
    ${DIR_INCLUDE}/network_exceptions.hpp
    ${DIR_INCLUDE}/status.hpp
    ${DIR_INCLUDE}/server/server.hpp
    ${DIR_INCLUDE}/server/client.hpp
)

# Client variables
set(CLIENT_SRC
    ${DIR_SOURCE}/network_exceptions.cpp
    ${DIR_SOURCE}/client/client.cpp
)

set(CLIENT_HEAD
    ${DIR_INCLUDE}/status.hpp
    ${DIR_INCLUDE}/client/client.hpp
    ${DIR_INCLUDE}/network_exceptions.hpp
)

add_library(server STATIC ${SERVER_SRC})
add_library(client STATIC ${CLIENT_SRC})

target_include_directories(server PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(client PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(server_example ${DIR_SOURCE}/server/example.cpp)
add_executable(client_example ${DIR_SOURCE}/client/example.cpp)

target_include_directories(server_example PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(client_example PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(server_example server)
target_link_libraries(client_example client)
