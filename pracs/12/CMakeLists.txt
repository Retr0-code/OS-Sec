cmake_minimum_required(VERSION 3.17.0)

project(HTTP-Server VERSION 0.1)

# Configure compiler
set(CMAKE_CXX_COMPILER "/usr/bin/gcc-11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c17 -g")
set(DIR_INCLUDE "include")
set(DIR_SOURCE "src")

# Server variables
set(SERVER_SRC
    ${DIR_SOURCE}/server/server.c
    ${DIR_SOURCE}/server/client.c
)

set(SERVER_HEAD
    ${DIR_INCLUDE}/network_exceptions.h
    ${DIR_INCLUDE}/status.h
    ${DIR_INCLUDE}/server/server.h
    ${DIR_INCLUDE}/server/client.h
)

add_library(server STATIC ${SERVER_SRC})
target_include_directories(server PUBLIC ${PROJECT_SOURCE_DIR}/include)
add_executable(server_example ${DIR_SOURCE}/server/example.c)

target_include_directories(server_example PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(server_example server)
