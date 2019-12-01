cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER gcc-8)
set(CMAKE_CXX_COMPILER g++-8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -g -fsanitize=address,undefined -fno-sanitize-recover=all")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")


project(tgnews)

set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(src/json)

add_executable(tgnews
    src/tgnews.cpp
    src/utils.hpp
    src/html_parse.hpp
    src/features.hpp
    src/logreg.hpp
    src/news_det.hpp
    src/langrec.hpp
    src/debug.hpp
    src/io.hpp
    src/common.h
)

set(BOOST_ROOT /usr/local/boost_1_65_1)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS thread system filesystem iostreams)
if (NOT Boost_FOUND)
  message(SEND_ERROR "Failed to find boost libraries.")
endif (NOT Boost_FOUND)

target_include_directories(tgnews PUBLIC
    src/lexbor/source
    src/cld3/
    src/udpipe/src
    src/udpipe/src_lib_only
    src/bigartm/src/artm
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(tgnews
    ${CMAKE_SOURCE_DIR}/src/lexbor/build/liblexbor_static.a
    ${CMAKE_SOURCE_DIR}/src/udpipe/src/libudpipe.a
    ${CMAKE_SOURCE_DIR} nlohmann_json::nlohmann_json
    ${CMAKE_SOURCE_DIR}/src/cld3/build/libcld3.a
    ${CMAKE_SOURCE_DIR}/src/bigartm/build/lib/libartm-static.a
    -lstdc++fs
    ${CMAKE_SOURCE_DIR}/src/bigartm/build/lib/libartm-static.a
    ${CMAKE_SOURCE_DIR}/src/bigartm/build/lib/libmessages_proto.a
    ${CMAKE_SOURCE_DIR}/src/bigartm/build/lib/libinternals_proto.a
    ${CMAKE_SOURCE_DIR}/src/bigartm/build/lib/libgflags.a
    ${CMAKE_SOURCE_DIR}/src/bigartm/build/lib/libglog.a
    ${CMAKE_SOURCE_DIR}/src/bigartm/build/lib/libprotoc.a
    ${CMAKE_SOURCE_DIR}/src/bigartm/build/lib/libprotobuf.a
    #-lpthread
    Boost::boost
    Boost::system
    Boost::thread
    Boost::filesystem
    Boost::iostreams
    #-lboost_system
    #-lboost_thread
    #-lboost_filesystem
    #-lboost_iostreams
    #-lprotobuf
)

