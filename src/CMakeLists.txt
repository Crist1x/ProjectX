# menu
add_library(menu_lib STATIC
        menu/MenuItem.cpp
        menu/MenuCategory.cpp
        menu/Menu.cpp
)
target_include_directories(menu_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

# spdlog
add_library(logger_lib INTERFACE)
target_include_directories(logger_lib INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(logger_lib INTERFACE spdlog::spdlog)

# db
add_library(db_lib STATIC
        db/Database.cpp
        db/UserRepository.cpp
        db/OrderRepository.cpp
        db/MenuRepository.cpp
)
target_include_directories(db_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(db_lib PUBLIC
        SQLite::SQLite3
        menu_lib
        logger_lib
)