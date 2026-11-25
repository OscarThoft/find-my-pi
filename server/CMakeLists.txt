cmake_minimum_required(VERSION 3.10)
project(http_test LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# llhttp bruges af RESTinio-serveren
set(LLHTTP_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/llhttp/api.c
    ${CMAKE_CURRENT_SOURCE_DIR}/external/llhttp/http.c
    ${CMAKE_CURRENT_SOURCE_DIR}/external/llhttp/llhttp.c
)

# Byg testserver og testklient som to separate programmer
add_executable(testServer testServer.cpp ${LLHTTP_SRC})
add_executable(testClient testClient.cpp)

# Serverens RESTinio-afhængigheder
target_include_directories(testServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/restinio
    ${CMAKE_CURRENT_SOURCE_DIR}/external/asio
    ${CMAKE_CURRENT_SOURCE_DIR}/external/fmt
    ${CMAKE_CURRENT_SOURCE_DIR}/external/json_dto
    ${CMAKE_CURRENT_SOURCE_DIR}/external/llhttp
    ${CMAKE_CURRENT_SOURCE_DIR}/external/expected-lite
    ${CMAKE_CURRENT_SOURCE_DIR}/external/rapidjson
)

target_compile_definitions(testServer PRIVATE FMT_HEADER_ONLY)
target_link_libraries(testServer PRIVATE Threads::Threads)

# Klienten bruger kun JSON-bibliotekerne
target_include_directories(testClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/json_dto
    ${CMAKE_CURRENT_SOURCE_DIR}/external/rapidjson
)