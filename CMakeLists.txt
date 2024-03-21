cmake_minimum_required(VERSION 3.22)

project(alioth-project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)
add_compile_definitions(PROJECT_VERSION="$ENV{PROJECT_VERSION}")

include_directories(
    "${CMAKE_SOURCE_DIR}/include")

include_directories(
    SYSTEM "$ENV{SIDE_PROJECT_SYSROOT}/usr/include")

link_directories(
    "$ENV{SIDE_PROJECT_SYSROOT}/usr/lib")

file(GLOB_RECURSE PUBLIC_SOURCES "src/*.cpp")
file(GLOB MODULES "module/*")

foreach(module IN LISTS MODULES)
    if(EXISTS "${module}/CMakeLists.txt")
        add_subdirectory(${module})
    endif()
endforeach()