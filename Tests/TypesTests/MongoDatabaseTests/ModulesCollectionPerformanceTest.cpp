#include "MongoDbEnvironment.hpp"
#include "MongoModulesCollection.hpp"
#include "Types.hpp"
#include <catch2/catch.hpp>
#include <chrono>
#include <iostream>

TEST_CASE("Tests module collection performance", "[MongoDatabase]") {
    // Prepare database
    Mongo::DbEnvironment::initialize("127.0.0.1");
    auto modulesCollectionEntry = Mongo::DbEnvironment::getInstance()->getClient();
    Mongo::ModulesCollection modulesCollection{*modulesCollectionEntry, "ModulesTest"};
    modulesCollection.drop();

    // Insert one
    Types::ModuleIdentifier firstIdentifier{Types::toModuleIdentifier(1)};
    Mongo::ModuleRecord firstRecord{};
    firstRecord.identifier = firstIdentifier;
    firstRecord.connectionState = Mongo::ConnectionState::Registered;
    firstRecord.ipAddress = "127.0.0.1";
    auto start = std::chrono::high_resolution_clock::now();
    REQUIRE(modulesCollection.insertOne(std::move(firstRecord)) == true);
    auto stop = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout<< elapsedTime.count() <<std::endl;

    // clean database before running multi insert
    modulesCollection.drop();
    
    std::vector<Mongo::ModuleRecord> records{};
    records.reserve(500);
    for(auto index = 0; index < 500; index++) {
        Mongo::ModuleRecord record{};
        firstRecord.identifier = Types::toModuleIdentifier(index);
        firstRecord.connectionState = Mongo::ConnectionState::Registered;
        firstRecord.ipAddress = "127.0.0.1";
        records.push_back(std::move(record));
    }

    auto startMulti = std::chrono::high_resolution_clock::now();
    std::for_each(std::begin(records), std::end(records), [&](auto& record){
        modulesCollection.insertOne(std::move(record));
    });
    auto stopMulti = std::chrono::high_resolution_clock::now();
    auto elapsedTimeMulti = std::chrono::duration_cast<std::chrono::microseconds>(stopMulti - startMulti);
    std::cout<< elapsedTimeMulti.count() <<std::endl;

    modulesCollection.drop();
}