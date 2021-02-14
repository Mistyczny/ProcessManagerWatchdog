project(Watchdog)

set(SOURCES
        src/WatchdogMain.cpp
        src/WatchdogServer.cpp
        src/WatchdogAcceptor.cpp
        src/WatchdogConnection.cpp
        src/MongoDbEnvironment.cpp
        src/MongoModulesCollection.cpp
        src/MongoServicesCollection.cpp
        src/Types.cpp
        src/WatchdogServiceRequestsHandlers.cpp
        src/WatchdogModuleRequestsHandlers.cpp)

add_executable(Watchdog ${SOURCES})

target_link_libraries(Watchdog pthread ${Boost_LIBRARIES} spdlog mongocxx bsoncxx WatchdogModuleProto ${PROTOBUF_LIBRARY} WatchdogServiceProto)
target_include_directories(Watchdog PRIVATE
        ./include
        ${MongoCxxIncludes}
        ${BOOST_ROOT}
        ${CMAKE_BINARY_DIR}/Protocols)

install(TARGETS Watchdog DESTINATION /opt/ProcessManager/)
install(FILES ${CMAKE_SOURCE_DIR}/Source/src/Watchdog.service DESTINATION /etc/systemd/system)