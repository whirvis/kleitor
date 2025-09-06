cmake_minimum_required(VERSION 3.22)
project(kleitor VERSION 0.0.1)

# The cglm project doesn't have a package on the AUR,
# so we will have to download it directly from GitHub
# and build it ourselves. It's a small package, so it
# doesn't take long luckily.
if(LINUX)
    FetchContent_Declare(cglm
        GIT_REPOSITORY https://github.com/recp/cglm
        GIT_TAG v0.9.6
        OVERRIDE_FIND_PACKAGE)
    FetchContent_MakeAvailable(cglm)
endif()

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

list(TRANSFORM runtime_srcs PREPEND "runtime/")

list(APPEND program_srcs ${library_srcs} ${runtime_srcs} "main.c")
add_executable(${PROJECT_NAME} ${program_srcs})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)

# We need to explicitly like the mathematics library on Linux
if(LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${ENGINE_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SONIC_GAME_NAME})
