project(abnf)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES} ${PUBLIC_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(${PROJECT_NAME} PRIVATE
    cli)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR})
