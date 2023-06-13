project(play)

set(PLAY_SOURCES
  "${CMAKE_SOURCE_DIR}/play/src/play.cpp"
  "${CMAKE_SOURCE_DIR}/play/src/syntax.cpp")

add_executable(play EXCLUDE_FROM_ALL ${PLAY_SOURCES})
target_link_libraries(play PRIVATE alioth-core)
target_include_directories(play PRIVATE "${CMAKE_SOURCE_DIR}/play/include")