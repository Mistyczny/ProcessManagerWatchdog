project(MongoDatabaseTests)

set(MongoCollectionsSource ${CMAKE_SOURCE_DIR}/Source/src/MongoModulesCollection.cpp
        ${CMAKE_SOURCE_DIR}/Source/src/MongoServicesCollection.cpp
        ${CMAKE_SOURCE_DIR}/Source/src/MongoDbEnvironment.cpp
        ${CMAKE_SOURCE_DIR}/Source/src/Types.cpp)

# Modules collection functionality tests
add_executable(ModulesCollectionTest ./ModulesCollectionTest.cpp ${MongoCollectionsSource})
target_link_libraries(ModulesCollectionTest PRIVATE pthread catchTestMain ${Boost_LIBRARIES} mongocxx bsoncxx)
target_include_directories(ModulesCollectionTest PRIVATE ${CMAKE_SOURCE_DIR}/Source/include
        ${BOOST_ROOT}
        ${Catch2Include}
        ${MongoCxxIncludes})

# Modules collection performance tests
#add_executable(ModulesCollectionPerformanceTest ./ModulesCollectionPerformanceTest.cpp ${ModulesCollectionSource})
#target_link_libraries(ModulesCollectionPerformanceTest PRIVATE pthread catchTestMain ${Boost_LIBRARIES} mongocxx bsoncxx)
#target_include_directories(ModulesCollectionPerformanceTest PRIVATE ${CMAKE_SOURCE_DIR}/Source/include
#    ${BOOST_ROOT}
#    ${Catch2Include}
#    ${MongoCxxIncludeDirectory}/mongocxx/v_noabi
#   ${MongoCxxIncludeDirectory}/bsoncxx/v_noabi)

add_executable(ServicesCollectionTest ./ServicesCollectionTest.cpp ${MongoCollectionsSource})
target_link_libraries(ServicesCollectionTest PRIVATE pthread catchTestMain ${Boost_LIBRARIES} mongocxx bsoncxx)
target_include_directories(ServicesCollectionTest PRIVATE
        ${CMAKE_SOURCE_DIR}/Source/include
        ${BOOST_ROOT}
        ${MongoCxxIncludes})

add_test(NAME ModulesCollectionTest COMMAND ModulesCollectionTest)
add_test(NAME ServicesCollectionTest COMMAND ServicesCollectionTest)

#add_test(NAME ModulesCollectionPerformanceTest COMMAND ModulesCollectionPerformanceTest)