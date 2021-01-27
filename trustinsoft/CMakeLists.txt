cmake_minimum_required(VERSION 3.4)

# examples
set(examples_out examples.out)

add_executable(${examples_out} examples.c)
target_link_libraries(${examples_out} zip)

set(examples_out ${test_out} PARENT_SCOPE)
