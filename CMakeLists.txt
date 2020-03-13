cmake_minimum_required(VERSION 3.10.2)
project(CSCE689-DRS)

set(CMAKE_CXX_STANDARD 17)

INCLUDE(FindPkgConfig)
find_package(OpenCV CONFIG REQUIRED )
message(STATUS "Found OpenCV version ${OpenCV_VERSION}")
find_package(PROTOBUF CONFIG REQUIRED 3)
message(STATUS "Found Protobuf version ${PROTOBUF_VERSION}")
include(FindProtobuf)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE PROTOBUF protobuf/*.proto)
protobuf_generate_cpp(PROTO_SRC PROTO_HDR ${PROTOBUF})

add_library(messages STATIC ${PROTO_SRC} ${PROTO_HDR})

add_executable(DRS main.cpp ${SOURCES} ${HEADERS} ${PROTOBUFS})
add_executable(DRS-Vincent test/Vincent.cpp ${SOURCES} ${HEADERS} ${PROTOBUFS})
add_executable(DRS-Josh test/Josh.cpp ${SOURCES} ${HEADERS} ${PROTOBUFS})

include_directories(PRIVATE src ${CMAKE_BINARY_DIR}/protobuf)
target_link_libraries(DRS pthread messages ${OpenCV_LIBS})
target_link_libraries(DRS-Vincent pthread messages ${OpenCV_LIBS})
target_link_libraries(DRS-Josh pthread messages ${OpenCV_LIBS})
