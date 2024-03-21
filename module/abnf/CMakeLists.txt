project(abnf)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES} ${PUBLIC_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include")

target_link_libraries(${PROJECT_NAME} PRIVATE
    cli)

target_link_options(${PROJECT_NAME} PRIVATE
    -static
    -static-libgcc
    -static-libstdc++)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR}/usr/bin/)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_BINARY_DIR})
