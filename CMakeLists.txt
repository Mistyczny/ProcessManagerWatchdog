cmake_minimum_required(VERSION 3.16)
project(ProcessManagerWatchdog)
enable_testing()

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.71.0 COMPONENTS system filesystem log REQUIRED)
find_package(Protobuf REQUIRED)
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
find_package(spdlog REQUIRED)
find_package(Catch2 REQUIRED)

set(MongoCxxIncludes    /usr/local/include/mongocxx/v_noabi
                        /usr/local/include/bsoncxx/v_noabi)

set(SOURCE_CODE ${CMAKE_CURRENT_SOURCE_DIR}/Source/src)
set(SOURCE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/Source/include)

add_subdirectory(Protocols)
add_subdirectory(Source)
add_subdirectory(Tests)
include(Packaging/Packaging.cmake)