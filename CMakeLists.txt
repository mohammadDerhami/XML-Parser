cmake_minimum_required(VERSION 3.14)
project(dbm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.15)
    cmake_policy(SET CMP0048 NEW)
    cmake_policy(SET CMP0077 NEW)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-class-memaccess)
    add_compile_options(-Wno-error=class-memaccess)
endif()

option(FETCH_MISSING_DEPS "Automatically fetch missing dependencies" ON)

include(FetchContent)
include(ExternalProject)

find_package(nlohmann_json QUIET)
if (NOT nlohmann_json_FOUND)
    if (FETCH_MISSING_DEPS)
        message(STATUS "Fetching nlohmann/json from GitHub...")
        FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.10.5  # Replace with the desired version if needed
        )
        FetchContent_MakeAvailable(nlohmann_json)
    else()
        message(FATAL_ERROR "nlohmann/json not found and FETCH_MISSING_DEPS is disabled")
    endif()
endif()


find_package(LibXml2 REQUIRED HINTS ${OTHER_LIBXML2_DIRS})
if (NOT LibXml2_FOUND)
    message(FATAL_ERROR "LibXML2 not found, ensure it is installed.")
else()
    message(STATUS "Found LibXML2 at ${LibXml2_INCLUDE_DIRS}")
endif()


find_package(SQLite3 QUIET)
if (NOT SQLite3_FOUND AND FETCH_MISSING_DEPS)
    message(STATUS "Fetching SQLite3...")
    FetchContent_Declare(
        sqlite3_src
        URL https://sqlite.org/2024/sqlite-amalgamation-3450200.zip
    )
    FetchContent_MakeAvailable(sqlite3_src)
    add_library(sqlite3 STATIC ${sqlite3_src_SOURCE_DIR}/sqlite3.c)
    target_include_directories(sqlite3 PUBLIC ${sqlite3_src_SOURCE_DIR})
endif()
add_executable(dbm
    src/main.cpp
    src/socket/socket.cpp
    src/socket/client.cpp
    src/config/config.cpp
    src/parser/parser.cpp
    src/parser/tree.cpp
    src/database/database.cpp
)
target_include_directories(dbm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/config
    ${CMAKE_CURRENT_SOURCE_DIR}/include/parser
    ${CMAKE_CURRENT_SOURCE_DIR}/include/database
    ${CMAKE_CURRENT_SOURCE_DIR}/include/socket
    ${LIBXML2_INCLUDE_DIRS}
)


target_link_libraries(dbm PRIVATE
    nlohmann_json::nlohmann_json
    sqlite3
    pthread
    ${LIBXML2_LIBRARIES}
)

target_compile_definitions(dbm PRIVATE
    -DRAPIDJSON_HAS_STDSTRING=1
    -DRAPIDJSON_NOMEMBERITERATORCLASS
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(dbm PRIVATE
        -Wno-class-memaccess
        -Wno-error=class-memaccess
    )
endif()




