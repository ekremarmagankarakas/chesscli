cmake_minimum_required(VERSION 3.20)
project(chesscli CXX)

# doctest's CMake declares an older minimum; allow it.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# Engine sources (everything except main.cc).
file(GLOB_RECURSE ENGINE_SOURCES
    "src/*.cc"
    "src/pieces/*.cc"
)
list(REMOVE_ITEM ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc")

add_library(chesscli_engine STATIC ${ENGINE_SOURCES})
target_include_directories(chesscli_engine PUBLIC src)

# Main executable.
add_executable(chesscli src/main.cc)
target_link_libraries(chesscli PRIVATE chesscli_engine)

# Tests (opt-in). doctest fetched via FetchContent.
option(CHESSCLI_BUILD_TESTS "Build unit tests" ON)
if(CHESSCLI_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
  include(FetchContent)
  FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.11
  )
  FetchContent_MakeAvailable(doctest)

  file(GLOB TEST_SOURCES "tests/*.cc")
  add_executable(chesscli_tests ${TEST_SOURCES})
  target_link_libraries(chesscli_tests PRIVATE chesscli_engine doctest::doctest)

  enable_testing()
  add_test(NAME unit COMMAND chesscli_tests)
endif()
