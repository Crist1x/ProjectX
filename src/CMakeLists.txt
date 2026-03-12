add_library(menu_lib STATIC
        menu/MenuItem.cpp
        menu/MenuCategory.cpp
        menu/Menu.cpp
)
target_include_directories(menu_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(logger_lib INTERFACE)
target_include_directories(logger_lib INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(logger_lib INTERFACE spdlog::spdlog)

add_library(logger
        ${CMAKE_SOURCE_DIR}/src/logger/logger.cpp
        ../include/logger/logger.h
)
target_include_directories(logger PUBLIC ${CMAKE_SOURCE_DIR}/include/logger)
target_link_libraries(logger PUBLIC spdlog::spdlog)

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

add_library(algorithm_lib
        algorithm/alg.cpp
        ../include/menu/Order.h
)

target_include_directories(algorithm_lib PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(algorithm_lib PRIVATE menu_lib)