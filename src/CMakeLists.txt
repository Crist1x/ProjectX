add_library(menu
        ${CMAKE_SOURCE_DIR}/src/menu/Menu.cpp
        ${CMAKE_SOURCE_DIR}/src/menu/MenuCategory.cpp
        ${CMAKE_SOURCE_DIR}/src/menu/MenuItem.cpp)

target_include_directories(menu PUBLIC ${CMAKE_SOURCE_DIR}/include/menu)

add_library(logger ${CMAKE_SOURCE_DIR}/src/logger/logger.cpp
        ../include/logger/logger.h)
target_include_directories(logger PUBLIC ${CMAKE_SOURCE_DIR}/include/logger)
target_link_libraries(logger PUBLIC spdlog::spdlog)