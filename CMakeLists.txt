set(CMAKE_C_COMPILER gcc-8)
set(CMAKE_CXX_COMPILER g++-8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined -fno-sanitize-recover=all")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

project(tgnews)

add_executable(tgnews
    src/tgnews.cpp
    src/utils.hpp)

target_link_libraries(tgnews -lstdc++fs)
