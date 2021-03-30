# Copyright 2021, Kacper Waśniowski
# All rights reserved.
cmake_minimum_required(VERSION 3.16)
project(ProcessManagerWatchdog)
enable_testing()

set(BUILD_TESTS False CACHE STRING "Turn on to build tests")

set(CMAKE_CXX_STANDARD 20)

set(BoostVersion 1.75.0)
set(ProtobufVersion 3.15.6)
set(MongocVersion 1.17.4)
set(MongocxxVersion 3.6.2)
set(SpdlogVersion 1.8.2)

find_package(Boost ${BoostVersion} COMPONENTS system filesystem log REQUIRED)
find_package(Protobuf ${ProtobufVersion} EXACT REQUIRED)
find_package(mongocxx ${MongocxxVersion} REQUIRED)
find_package(bsoncxx ${MongocxxVersion} REQUIRED)
find_package(spdlog ${SpdlogVersion} EXACT REQUIRED)

set(SOURCE_CODE ${CMAKE_CURRENT_SOURCE_DIR}/Source/src)
set(SOURCE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/Source/include)
set(artifactoryDownloader ${CMAKE_SOURCE_DIR}/Toolkits/artifactory_downloader.sh)

configure_file(Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile)

add_subdirectory(Dockerfiles)
add_subdirectory(Protocols)
add_subdirectory(Source)

if(${BUILD_TESTS})
    add_subdirectory(Tests)
endif()