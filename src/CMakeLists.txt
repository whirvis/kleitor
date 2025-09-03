cmake_minimum_required(VERSION 3.22)
project(kleitor VERSION 0.0.1)

add_subdirectory("engine")
add_subdirectory("sonic")

list(APPEND library_srcs
    "cglm_no_io.h"
    "minimp3.c" "minimp3.h" "minimp3_ex.h"
    "stb_image.c" "stb_image.h"
    "stb_image_resize.c" "stb_image_resize.h"
    "stb_truetype.c" "stb_truetype.h"
    "stb_vorbis.c" "stb_vorbis.h")
list(TRANSFORM library_srcs PREPEND "libraries/")

list(APPEND runtime_srcs
    "asset.c"   "asset.h"
    "common.c"  "common.h"
    "error.c"   "error.h"
                "engine.h"
    "main.c"
    "runtime.c" "runtime.h"
    "stream.c"  "stream.h"
    "thread.c"  "thread.h"
    "time.c"    "time.h")

if(LINUX)
    list(APPEND runtime_srcs
        "platform/pc_asset.c"
        "platform/linux_error.c"
        "platform/linux_thread.c"
        "platform/linux_time.c")
endif()

list(APPEND program_srcs ${library_srcs} ${runtime_srcs})
add_executable(${PROJECT_NAME} ${program_srcs})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)

# We need to explicitly like the mathematics library on Linux
if(LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${ENGINE_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SONIC_GAME_NAME})
