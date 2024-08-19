cmake_minimum_required(VERSION 3.10)

project(jsh VERSION 0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")

# MAIN EXECUTABLE
# ---------------
add_executable(jsh
  main.c
  src/lexer.c
  src/grammar.c
)
configure_file(config.h.in config.h)
target_include_directories(jsh PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(jsh PUBLIC src)

# TESTING
# -------
add_executable(test_stack test/test_stack.c src/stack.c)
add_executable(test_queue test/test_queue.c src/queue.c)

enable_testing()

add_test(
  NAME test_stack
  COMMAND $<TARGET_FILE:test_stack>
)
add_test(
  NAME test_queue
  COMMAND $<TARGET_FILE:test_queue>
)
