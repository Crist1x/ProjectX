add_executable(bot_app
        main.cpp
)

target_include_directories(bot_app PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_include_directories(bot_app PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_definitions(bot_app PRIVATE
        PROJECTX_ROOT="${CMAKE_SOURCE_DIR}"
)

target_link_libraries(bot_app PRIVATE
        db_lib
        bot_lib
)

set_target_properties(bot_app PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)