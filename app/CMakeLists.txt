add_executable(bot_app
        main.cpp
        ../include/algorithm/OrderDistributionAlgorithm.h
)

target_include_directories(bot_app PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_include_directories(bot_app PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(bot_app PRIVATE
        menu_lib
        db_lib
        logger_lib
)

set_target_properties(bot_app PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)