cmake_minimum_required (VERSION 2.8.11)

project (ethercatter)

add_executable (ethercatter main.cc)
target_compile_options(ethercatter PUBLIC -std=c++2a)
