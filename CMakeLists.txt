cmake_minimum_required(VERSION 3.14)
project(example_04_lim)

file(GLOB SRC_DYNAMIXEL
     "../src/dynamixel_sdk/*.cpp"
)

add_executable(example_04_lim)

target_sources(example_04_lim PRIVATE
    example_04_lim.cpp
    ${SRC_DYNAMIXEL}
)

target_compile_definitions(example_04_lim PRIVATE -DFMT_HEADER_ONLY=1)
target_compile_options(example_04_lim PRIVATE -Wno-psabi)
target_include_directories(example_04_lim PRIVATE
    ../src
    ../src/dynamixel_sdk
)