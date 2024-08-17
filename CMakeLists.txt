cmake_minimum_required(VERSION 3.10)

project(jsh VERSION 0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Parser
# ------
find_package(BISON)
BISON_TARGET(grammar src/grammar.y ${CMAKE_CURRENT_BINARY_DIR}/grammar.c COMPILE_FLAGS --header=${PROJECT_BINARY_DIR}/grammar.h)

# MAIN EXECUTABLE
# ---------------
add_executable(jsh
  main.c
  src/lexer.c
  ${BISON_grammar_OUTPUTS}
)
configure_file(config.h.in config.h)
target_include_directories(jsh PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(jsh PUBLIC src)

# TESTING
# -------
add_executable(test_stack test/test_stack.c src/stack.c)

enable_testing()

add_test(
  NAME test_stack
  COMMAND $<TARGET_FILE:test_stack>
)
