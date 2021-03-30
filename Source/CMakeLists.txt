# Copyright 2021, Kacper Waśniowski
# All rights reserved.
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/WatchdogMain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/WatchdogServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/WatchdogAcceptor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/WatchdogConnection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MongoDbEnvironment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MongoModulesCollection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MongoServicesCollection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/WatchdogServiceRequestsHandlers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/WatchdogModuleRequestsHandlers.cpp
)

add_executable(Watchdog ${SOURCES})

target_link_libraries(Watchdog
        PRIVATE
    pthread
    ${Boost_LIBRARIES}
    spdlog
    mongo::mongocxx_shared
    mongo::bsoncxx_shared
    WatchdogModuleProto
    ${PROTOBUF_LIBRARY}
    WatchdogServiceProto
)
target_include_directories(Watchdog
        PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${BOOST_ROOT}
    ${CMAKE_BINARY_DIR}/Protocols
)

install(TARGETS Watchdog DESTINATION /opt/ProcessManager/)
install(FILES ${CMAKE_SOURCE_DIR}/Source/src/Watchdog.service DESTINATION /etc/systemd/system)