cmake_minimum_required(VERSION 3.25.0)
project(server VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)

file(GLOB sources "src/*.c" "src/**/*.c")
add_executable(server ${sources})
target_compile_features(server PRIVATE c_std_17)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME mimalloc
  VERSION 2.1.7
  GITHUB_REPOSITORY microsoft/mimalloc
  OPTIONS "BUILD_STATIC_LIBS ON" "BUILD_SHARED_LIBS OFF"
)

CPMAddPackage(
  NAME libuv
  VERSION 1.48.0
  GITHUB_REPOSITORY libuv/libuv
  OPTIONS "BUILD_STATIC_LIBS ON" "BUILD_SHARED_LIBS OFF"
)

CPMAddPackage(
  NAME llhttp
  VERSION 9.2.1
  URL https://github.com/nodejs/llhttp/archive/refs/tags/release/v9.2.1.tar.gz
  OPTIONS "BUILD_STATIC_LIBS ON" "BUILD_SHARED_LIBS OFF"
)

target_link_libraries(server mimalloc-static uv_a llhttp_static)
