cmake_minimum_required(VERSION 3.14)
project(ProjectX VERSION 1.0 LANGUAGES CXX)
include_directories(${CMAKE_SOURCE_DIR}/include)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
    set(CMAKE_FIND_ROOT_PATH /opt/homebrew)
endif()

include(FetchContent)

# Google test
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
        DOWNLOAD_EXTRACT_TIMESTAMP true
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# spdlog
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.11.0
)
FetchContent_MakeAvailable(spdlog)

# TgBot
FetchContent_Declare(
        tgbot
        GIT_REPOSITORY https://github.com/reo7sp/tgbot-cpp.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(tgbot)

# sqlite
find_package(SQLite3 REQUIRED)

add_subdirectory(src)
add_subdirectory(app)
add_subdirectory(tests)
get_target_property(BOT_APP_SOURCES bot_app SOURCES)
if(BOT_APP_SOURCES)
    target_include_directories(bot_app PRIVATE ${CMAKE_SOURCE_DIR}/include)
endif()