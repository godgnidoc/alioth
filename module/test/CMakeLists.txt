project(test)

find_package(GTest REQUIRED)
enable_testing()

file(GLOB_RECURSE SOURCES *.cpp)
add_executable(${PROJECT_NAME} ${PUBLIC_SOURCES} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::Main)

gtest_discover_tests(${PROJECT_NAME})