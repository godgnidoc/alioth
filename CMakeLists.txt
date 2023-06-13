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
  "${CMAKE_SOURCE_DIR}/src/alioth/ast.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth/inspect.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth/parser.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth/strings.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth/document.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth/lexicon.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth/regex.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth/syntax.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth/grammar.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth/skeleton.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth/template.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth/alioth.cpp")
add_library(alioth-core OBJECT ${CORE_SOURCES})

set(CLI_SOURCES
  "${CMAKE_SOURCE_DIR}/src/alioth-cli/compile.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth-cli/framework.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth-cli/main.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth-cli/render.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth-cli/skeleton.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth-cli/syntax.cpp"
  "${CMAKE_SOURCE_DIR}/src/alioth-cli/tokenize.cpp")
add_executable(alioth ${CLI_SOURCES})
target_link_libraries(alioth PRIVATE alioth-core)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()