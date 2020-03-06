cmake_minimum_required(VERSION 3.10.2)
project(CSCE689-DRS)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)

add_executable(DRS main.cpp ${SOURCES} ${HEADERS})
add_executable(DRS-Vincent test/Vincent.cpp ${SOURCES} ${HEADERS})
add_executable(DRS-Josh test/Josh.cpp ${SOURCES} ${HEADERS})

target_include_directories(DRS PRIVATE src)
target_include_directories(DRS-Vincent PRIVATE src)
target_include_directories(DRS-Josh PRIVATE src)
find_package(OpenCV REQUIRED )

target_link_libraries(DRS pthread ${OpenCV_LIBS})
target_link_libraries(DRS-Vincent pthread ${OpenCV_LIBS})
target_link_libraries(DRS-Josh pthread ${OpenCV_LIBS})
