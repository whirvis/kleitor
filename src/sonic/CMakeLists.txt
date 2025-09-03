cmake_minimum_required(VERSION 3.22)
project(kleitor_sonic_game VERSION 0.0.1)

list(APPEND sonic_game_srcs
    "sonic_game.c" "sonic_game.h"
    "title_state.c"
    "load_state.c"
    "play_state.c")

set(SONIC_GAME_NAME ${PROJECT_NAME} CACHE INTERNAL "")

add_library(${SONIC_GAME_NAME} ${sonic_game_srcs})
target_compile_options(${SONIC_GAME_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(${SONIC_GAME_NAME} PRIVATE ${ENGINE_NAME})
