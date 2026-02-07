cmake_minimum_required(VERSION 4.0)
project(ProjectX)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(ProjectX main.cpp
        src/menu/Menu.cpp
        include/menu/Menu.h
        include/menu/Menu.h
        include/menu/Menu.h
        src/menu/Menu.cpp
        include/menu/MenuCategory.h
        include/menu/MenuItem.h
        src/menu/MenuCategory.cpp
        src/menu/MenuItem.cpp)
