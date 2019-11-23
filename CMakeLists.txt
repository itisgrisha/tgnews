cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER gcc-8)
set(CMAKE_CXX_COMPILER g++-8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined -fno-sanitize-recover=all")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

#add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

project(tgnews)

set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(src/json)

add_executable(tgnews
    src/tgnews.cpp
    src/utils.hpp
    src/html_parse.hpp
    src/debug.hpp)

target_include_directories(tgnews PUBLIC
    src/lexbor/source
    src/cld3/
    src/udpipe/src)

target_link_libraries(tgnews
    ${CMAKE_SOURCE_DIR}/src/lexbor/build/liblexbor_static.a
    ${CMAKE_SOURCE_DIR}/src/udpipe/src/libudpipe.a
    ${CMAKE_SOURCE_DIR} nlohmann_json::nlohmann_json
    ${CMAKE_SOURCE_DIR}/src/cld3/build/libcld3.a
    -lstdc++fs
    -lpthread
    -lprotobuf
    )

