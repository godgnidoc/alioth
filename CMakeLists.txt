cmake_minimum_required(VERSION 3.22)

project(alioth)

option(BUILD_TESTS "Build tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
include_directories("${CMAKE_SOURCE_DIR}/include")
link_libraries(nlohmann_json nlohmann_json::nlohmann_json fmt::fmt-header-only)

set(CORE_SOURCES
  "${CMAKE_SOURCE_DIR}/src/ast.cpp"
  "${CMAKE_SOURCE_DIR}/src/inspect.cpp"
  "${CMAKE_SOURCE_DIR}/src/parser.cpp"
  "${CMAKE_SOURCE_DIR}/src/strings.cpp"
  "${CMAKE_SOURCE_DIR}/src/document.cpp"
  "${CMAKE_SOURCE_DIR}/src/lexicon.cpp"
  "${CMAKE_SOURCE_DIR}/src/regex.cpp"
  "${CMAKE_SOURCE_DIR}/src/syntax.cpp"
  "${CMAKE_SOURCE_DIR}/src/grammar.cpp"
  "${CMAKE_SOURCE_DIR}/src/skeleton.cpp"
  "${CMAKE_SOURCE_DIR}/src/template.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth.cpp")
add_library(alioth-core OBJECT ${CORE_SOURCES})

add_executable(alioth 
  "${CMAKE_SOURCE_DIR}/main.cpp")
target_link_libraries(alioth PRIVATE alioth-core)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

add_subdirectory(play)