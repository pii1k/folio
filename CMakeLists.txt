cmake_minimum_required(VERSION 3.16)
project(folio LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# geometry
add_library(folio_geometry INTERFACE)
target_include_directories(folio_geometry INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# core
add_library(folio_core INTERFACE)
target_link_libraries(folio_core INTERFACE folio_geometry)

# movement
add_library(folio_movement src/movement/character_controller.cpp)
target_link_libraries(folio_movement PUBLIC folio_core)

# collision
add_library(folio_collision INTERFACE)
target_include_directories(folio_collision INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# combat
add_library(folio_combat INTERFACE)
target_include_directories(folio_combat INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# world
add_library(folio_world src/world/tile_map.cpp)
target_include_directories(folio_world PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Sfml adapter
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
add_library(folio_adapters_sfml adapters/sfml/sfml_input.cpp)
target_include_directories(folio_adapters_sfml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(folio_adapters_sfml
PUBLIC
    SFML::Graphics
    SFML::Window
    SFML::System
PRIVATE
    folio_core
)

# DEMO
add_executable(folio_demo
    apps/demo/main.cpp
    apps/demo/demo_game.cpp
    apps/app_core/game_loop.cpp
)
target_link_libraries(folio_demo
PRIVATE
    folio_core
    folio_geometry
    folio_movement
    folio_collision
    folio_combat
    folio_world
    folio_adapters_sfml
)
