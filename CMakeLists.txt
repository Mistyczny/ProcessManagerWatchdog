cmake_minimum_required(VERSION 3.16)
project(ProcessManagerWatchdog)
enable_testing()

set(BUILD_TESTS False CACHE STRING "Turn on to build tests")

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.71.0 COMPONENTS system filesystem log REQUIRED)
find_package(Protobuf 3.14 EXACT REQUIRED)
find_package(mongocxx 3.6.1 REQUIRED)
find_package(bsoncxx 3.6.1 REQUIRED)
find_package(spdlog 1.8.2 EXACT REQUIRED)

set(SOURCE_CODE ${CMAKE_CURRENT_SOURCE_DIR}/Source/src)
set(SOURCE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/Source/include)
set(artifactoryDownloader ${CMAKE_SOURCE_DIR}/Toolkits/artifactory_downloader.sh)

add_subdirectory(Protocols)
add_subdirectory(Source)

if(${BUILD_TESTS})
    add_subdirectory(Tests)
endif()