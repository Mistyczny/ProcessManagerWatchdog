project(WatchdogMessageHandlersTests)

set(WatchdogModuleRequestsSource    ${SOURCE_CODE}/WatchdogModuleRequestsHandlers.cpp
                                    ${SOURCE_CODE}/MongoModulesCollection.cpp
                                    ${SOURCE_CODE}/MongoDbEnvironment.cpp
                                    ${SOURCE_CODE}/Types.cpp)

add_executable(WatchdogModuleRequestConnectHandlerTest WatchdogModuleRequestConnectHandlerTest.cpp ${WatchdogModuleRequestsSource})
target_link_libraries(WatchdogModuleRequestConnectHandlerTest PRIVATE pthread catchTestMain ${Boost_LIBRARIES} mongocxx bsoncxx WatchdogModuleProto ${PROTOBUF_LIBRARY})
target_include_directories(WatchdogModuleRequestConnectHandlerTest PRIVATE ${SOURCE_INCLUDE}
        ${BOOST_ROOT}
        ${MongoCxxIncludes}
        ${CMAKE_BINARY_DIR}/Protocols)

add_executable(WatchdogModuleRequestPingHandlerTest WatchdogModuleRequestPingHandlerTest.cpp ${WatchdogModuleRequestsSource})
target_link_libraries(WatchdogModuleRequestPingHandlerTest PRIVATE pthread catchTestMain ${Boost_LIBRARIES} mongocxx bsoncxx WatchdogModuleProto ${PROTOBUF_LIBRARY})
target_include_directories(WatchdogModuleRequestPingHandlerTest PRIVATE ${SOURCE_INCLUDE}
        ${BOOST_ROOT}
        ${MongoCxxIncludes}
        ${CMAKE_BINARY_DIR}/Protocols)

add_executable(WatchdogModuleRequestReconnectHandlerTest WatchdogModuleRequestReconnectHandlerTest.cpp ${WatchdogModuleRequestsSource})
target_link_libraries(WatchdogModuleRequestReconnectHandlerTest PRIVATE pthread catchTestMain ${Boost_LIBRARIES} mongocxx bsoncxx WatchdogModuleProto ${PROTOBUF_LIBRARY})
target_include_directories(WatchdogModuleRequestReconnectHandlerTest PRIVATE ${SOURCE_INCLUDE}
        ${BOOST_ROOT}
        ${MongoCxxIncludes}
        ${CMAKE_BINARY_DIR}/Protocols)

add_executable(WatchdogModuleRequestShutdownHandlerTest WatchdogModuleRequestShutdownHandlerTest.cpp ${WatchdogModuleRequestsSource})
target_link_libraries(WatchdogModuleRequestShutdownHandlerTest PRIVATE pthread catchTestMain ${Boost_LIBRARIES} mongocxx bsoncxx WatchdogModuleProto ${PROTOBUF_LIBRARY})
target_include_directories(WatchdogModuleRequestShutdownHandlerTest PRIVATE ${SOURCE_INCLUDE}
        ${BOOST_ROOT}
        ${MongoCxxIncludes}
        ${CMAKE_BINARY_DIR}/Protocols)

add_test(NAME WatchdogModuleRequestConnectHandlerTest COMMAND WatchdogModuleRequestConnectHandlerTest)
add_test(NAME WatchdogModuleRequestPingHandlerTest COMMAND WatchdogModuleRequestPingHandlerTest)
add_test(NAME WatchdogModuleRequestReconnectHandlerTest COMMAND WatchdogModuleRequestReconnectHandlerTest)
add_test(NAME WatchdogModuleRequestShutdownHandlerTest COMMAND WatchdogModuleRequestShutdownHandlerTest)