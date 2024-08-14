cmake_minimum_required(VERSION 3.10)

project(jsh VERSION 0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(jsh main.c)

configure_file(config.h.in config.h)

target_include_directories(jsh PUBLIC "${PROJECT_BINARY_DIR}")

# testing binaries
add_executable(test_lexer_next test/test_lexer_next.c src/lexer.c src/stack.c)
add_executable(test_stack test/test_stack.c src/stack.c)

enable_testing()

add_test(
  NAME test_lexer_next
  COMMAND $<TARGET_FILE:test_lexer_next>
  )
add_test(
  NAME test_stack
  COMMAND $<TARGET_FILE:test_stack>
  )
