cmake_minimum_required(VERSION 3.22)
project(kleitor VERSION 0.0.1)

include(FetchContent)

set(LINUX 0)

if (WIN32)
    message(FATAL_ERROR "Use the Visual Studio solution.")
elseif ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    set(LINUX 1)
else()
    message(FATAL_ERROR "Platform \"${CMAKE_SYSTEM_NAME}\" not supported.")
endif()

add_subdirectory("src")
