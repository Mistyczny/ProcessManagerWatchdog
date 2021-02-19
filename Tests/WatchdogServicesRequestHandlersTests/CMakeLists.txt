project(WatchdogMessageHandlersTests)

set(WatchdogServiceRequestSources
    ${SOURCE_CODE}/WatchdogServiceRequestsHandlers.cpp
    ${SOURCE_CODE}/MongoServicesCollection.cpp
    ${SOURCE_CODE}/MongoDbEnvironment.cpp
    ${SOURCE_CODE}/Types.cpp
)

add_executable(WatchdogServiceRequestConnectHandlerTest ./WatchdogServiceRequestConnectHandlerTest.cpp ${WatchdogServiceRequestSources})
target_link_libraries(WatchdogServiceRequestConnectHandlerTest 
        PRIVATE 
    pthread
    catchTestMain
    ${Boost_LIBRARIES}
    mongocxx
    bsoncxx
    WatchdogServiceProto
    ${PROTOBUF_LIBRARY}
)
target_include_directories(WatchdogServiceRequestConnectHandlerTest
        PRIVATE
    ${SOURCE_INCLUDE}
    ${BOOST_ROOT}
    ${MongoCxxIncludes}
    ${CMAKE_BINARY_DIR}/Protocols
)

add_executable(WatchdogServiceRequestPingHandlerTest ./WatchdogServiceRequestPingHandlerTest.cpp ${WatchdogServiceRequestSources})
target_link_libraries(WatchdogServiceRequestPingHandlerTest
        PRIVATE 
    pthread
    catchTestMain
    ${Boost_LIBRARIES}
    mongocxx
    bsoncxx
    WatchdogServiceProto
    ${PROTOBUF_LIBRARY}
)
target_include_directories(WatchdogServiceRequestPingHandlerTest
        PRIVATE 
    ${SOURCE_INCLUDE}
    ${BOOST_ROOT}
    ${MongoCxxIncludes}
    ${CMAKE_BINARY_DIR}/Protocols
)

add_executable(WatchdogServiceRequestReconnectHandlerTest ./WatchdogServiceRequestReconnectHandlerTest.cpp ${WatchdogServiceRequestSources})
target_link_libraries(WatchdogServiceRequestReconnectHandlerTest 
        PRIVATE
    pthread
    catchTestMain
    ${Boost_LIBRARIES}
    mongocxx
    bsoncxx
    WatchdogServiceProto
    ${PROTOBUF_LIBRARY}
)
target_include_directories(WatchdogServiceRequestReconnectHandlerTest
        PRIVATE
    ${SOURCE_INCLUDE}
    ${BOOST_ROOT}
    ${MongoCxxIncludes}
    ${CMAKE_BINARY_DIR}/Protocols
)

add_executable(WatchdogServiceRequestShutdownHandlerTest ./WatchdogServiceRequestShutdownHandlerTest.cpp ${WatchdogServiceRequestSources})
target_link_libraries(WatchdogServiceRequestShutdownHandlerTest 
        PRIVATE
    pthread
    catchTestMain
    ${Boost_LIBRARIES}
    mongocxx
    bsoncxx
    WatchdogServiceProto
    ${PROTOBUF_LIBRARY}
)
target_include_directories(WatchdogServiceRequestShutdownHandlerTest
        PRIVATE
    ${SOURCE_INCLUDE}
    ${BOOST_ROOT}
    ${MongoCxxIncludes}
    ${CMAKE_BINARY_DIR}/Protocols
)

# Add tests to run
add_test(NAME WatchdogServiceRequestConnectHandlerTest COMMAND WatchdogServiceRequestConnectHandlerTest)
add_test(NAME WatchdogServiceRequestPingHandlerTest COMMAND WatchdogServiceRequestPingHandlerTest)
add_test(NAME WatchdogServiceRequestReconnectHandlerTest COMMAND WatchdogServiceRequestReconnectHandlerTest)
add_test(NAME WatchdogServiceRequestShutdownHandlerTest COMMAND WatchdogServiceRequestShutdownHandlerTest)