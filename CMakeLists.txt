cmake_minimum_required(VERSION 3.10.2)
project(CSCE689-DRS)

INCLUDE(FindPkgConfig)
find_package(OpenCV REQUIRED )
find_package(Protobuf REQUIRED )
pkg_search_module(Protobuf REQUIRED Protobuf >= 3)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE PROTOBUF protobuf/*.proto)
protobuf_generate_cpp(PROTO_SRC PROTO_HDR ${PROTOBUF})

add_library(messages STATIC ${PROTO_SRC} ${PROTO_HDR})
target_link_libraries(messages ${PROTOBUF_LIBRARY})

add_executable(DRS main.cpp ${SOURCES} ${HEADERS} ${PROTOBUFS})
add_executable(DRS-Vincent test/Vincent.cpp ${SOURCES} ${HEADERS} ${PROTOBUFS})
add_executable(DRS-Josh test/Josh.cpp ${SOURCES} ${HEADERS} ${PROTOBUFS})

include_directories(PRIVATE src ${CMAKE_BINARY_DIR}/protobuf)
target_link_libraries(DRS pthread messages ${OpenCV_LIBS})
target_link_libraries(DRS-Vincent pthread messages ${OpenCV_LIBS})
target_link_libraries(DRS-Josh pthread messages ${OpenCV_LIBS})
