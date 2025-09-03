cmake_minimum_required(VERSION 3.22)
project(kleitor_engine VERSION 0.0.1)

find_package(glfw3 3.3 REQUIRED)
find_package(GLEW      REQUIRED)
find_package(OpenGL    REQUIRED)
find_package(OpenAL    REQUIRED)

list(APPEND engine_srcs
    "asset.h"
    "audio.c"    "audio.h"
    "common.h"
    "engine.c"   "engine.h"
    "error.h"
    "game.c"     "game.h"
    "graphics.c" "graphics.h"
    "input.c"    "input.h"
    "stream.h"
    "time.h")

if(LINUX)
    list(APPEND engine_srcs
        "platform/glfw_input.c"
        "platform/openal_audio.c"
        "platform/opengl_graphics.c"
        "platform/pc_engine.c")
endif()

set(ENGINE_NAME ${PROJECT_NAME} CACHE INTERNAL "")

add_library(${ENGINE_NAME} ${engine_srcs})
target_compile_options(${ENGINE_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(${ENGINE_NAME} PRIVATE glfw GLEW::GLEW)
target_link_libraries(${ENGINE_NAME} PRIVATE OpenGL::GL ${OPENAL_LIBRARY})
