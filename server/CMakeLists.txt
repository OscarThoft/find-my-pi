cmake_minimum_required(VERSION 3.16)

project(UWBGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Server - Gemmer
add_executable(server
    server.cpp
)

# Client - Søger
add_executable(client
    client.cpp
)

# Enable warnings
target_compile_options(server PRIVATE -Wall -Wextra)
target_compile_options(client PRIVATE -Wall -Wextra)