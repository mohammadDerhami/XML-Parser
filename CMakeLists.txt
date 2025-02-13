cmake_minimum_required(VERSION 3.10)

project(dbm VERSION 1.0)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_BUILD_TYPE Debug)
include_directories(include)
include_directories(/usr/include/rapidjson/include/)

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.cpp")

add_executable(dbm src/main.cpp src/server.cpp src/config.cpp src/parseXml.cpp src/database.cpp )

target_link_libraries(dbm sqlite3 pthread ${LIBXML2_LIBRARIES})

