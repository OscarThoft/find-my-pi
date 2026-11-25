
cmake_minimum_required(VERSION 3.10)
project(FindMyPi LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)


# ==========================================
# SERVER / CLIENT
# ==========================================

set(LLHTTP_SRC
    server/external/llhttp/api.c
    server/external/llhttp/http.c
    server/external/llhttp/llhttp.c
)

add_library(network
    server/server.cpp
    server/client.cpp
    ${LLHTTP_SRC}
)

target_include_directories(network PUBLIC
    server
    server/external
    server/external/restinio
    server/external/asio
    server/external/fmt
    server/external/json_dto
    server/external/llhttp
    server/external/expected-lite
    server/external/rapidjson
)

target_compile_definitions(network PUBLIC FMT_HEADER_ONLY)
target_link_libraries(network PUBLIC Threads::Threads)

if(WIN32)
    target_link_libraries(network PUBLIC ws2_32 mswsock)
endif()


# ==========================================
# HIDER
# ==========================================

add_library(gameData Hider/src/gameData.cpp)
target_include_directories(gameData PUBLIC Hider Hider/inc)

add_library(gameStatus Hider/src/gameStatus.cpp)
target_include_directories(gameStatus PUBLIC Hider Hider/inc)

add_library(gemmerController
    Hider/src/gemmerController.cpp
)

target_include_directories(gemmerController PUBLIC
    Hider
    Hider/inc
)

target_link_libraries(gemmerController PUBLIC
    gameData
    gameStatus
    network
)


# ==========================================
# SEEKER
# ==========================================

add_library(gameSession Seeker/src/game_session.cpp)
target_include_directories(gameSession PUBLIC Seeker Seeker/inc)

add_library(UI Seeker/src/UI.cpp)
target_include_directories(UI PUBLIC Seeker Seeker/inc)

add_library(soegerController
    Seeker/src/soegerController.cpp
)

target_include_directories(soegerController PUBLIC
    Seeker
    Seeker/inc
)

target_link_libraries(soegerController PUBLIC
    gameSession
    UI
    network
)


# ==========================================
# UWB (Linux / Raspberry Pi)
# ==========================================

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    add_library(UWB
        UWB/src/SPIDriver.cpp
        UWB/src/UWBDriver.cpp
        UWB/src/UWBModule.cpp
        UWB/src/UWBRanging.cpp
    )

    target_include_directories(UWB PUBLIC UWB UWB/inc)
    target_link_libraries(UWB PUBLIC Threads::Threads)

    add_executable(testUWB
        UWB/src/uwbdriver_test.cpp
    )

    target_link_libraries(testUWB PRIVATE UWB)

endif()


# ==========================================
# HIDER MODULE TESTS
# ==========================================

add_executable(testGameData
    Hider/modultest/testAfGameData.cpp
)
target_link_libraries(testGameData PRIVATE gameData)


add_executable(testGameStatus
    Hider/modultest/testAfGameStatus.cpp
)
target_link_libraries(testGameStatus PRIVATE gameStatus)


add_executable(testGemmer
    Hider/modultest/testAfGemmerControl.cpp
)
target_link_libraries(testGemmer PRIVATE gemmerController)


# ==========================================
# SEEKER MODULE TESTS
# ==========================================

add_executable(testGameSession
    Seeker/modultest/testAfGameSession.cpp
)
target_link_libraries(testGameSession PRIVATE gameSession)


add_executable(testUI
    Seeker/modultest/testAfUI.cpp
)
target_link_libraries(testUI PRIVATE UI)


# ==========================================
# NETWORK TESTS
# ==========================================

add_executable(testServer server/testServer.cpp)
target_link_libraries(testServer PRIVATE network)

add_executable(testClient server/testClient.cpp)
target_link_libraries(testClient PRIVATE network)
