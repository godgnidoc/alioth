project(test)

find_package(GTest CONFIG REQUIRED)

set(TEST_SOURCES
  "${CMAKE_SOURCE_DIR}/test/grammar_test.cpp"
  "${CMAKE_SOURCE_DIR}/test/lexicon_test.cpp"
  "${CMAKE_SOURCE_DIR}/test/parser_test.cpp"
  "${CMAKE_SOURCE_DIR}/test/regex_test.cpp"
  "${CMAKE_SOURCE_DIR}/test/skeleton_test.cpp"
  "${CMAKE_SOURCE_DIR}/test/syntax_test.cpp"
  "${CMAKE_SOURCE_DIR}/test/generic_test.cpp"
  "${CMAKE_SOURCE_DIR}/test/storage_test.cpp"
  "${CMAKE_SOURCE_DIR}/test/cli_test.cpp"
  "${CMAKE_SOURCE_DIR}/test/variable_test.cpp"
  "${CMAKE_SOURCE_DIR}/test/template_test/template_test.cpp")
add_executable(alioth-test ${TEST_SOURCES})

target_link_libraries(alioth-test PRIVATE 
  alioth-core
  aliox
  GTest::gtest
  GTest::gtest_main
  GTest::gmock
  GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(alioth-test)