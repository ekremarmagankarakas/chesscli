cmake_minimum_required(VERSION 3.20)
project(chesscli CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

file(GLOB_RECURSE SOURCES
    "src/*.cc"
    "src/pieces/*.cc"
)

add_executable(chesscli ${SOURCES})

target_include_directories(chesscli PRIVATE src)
