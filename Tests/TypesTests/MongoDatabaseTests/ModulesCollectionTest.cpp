#include "MongoDbEnvironment.hpp"
#include "MongoModulesCollection.hpp"
#include "Types.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Tests modules collection functionalities", "[MongoDatabase]") {
    Mongo::DbEnvironment::initialize("127.0.0.1");
    auto modulesCollectionEntry = Mongo::DbEnvironment::getInstance()->getClient();
    Mongo::ModulesCollection modulesCollection{*modulesCollectionEntry, "ModulesTest"};
    modulesCollection.drop();

    Types::ModuleIdentifier firstIdentifier{Types::toModuleIdentifier(1)};
    REQUIRE(modulesCollection.findOne(firstIdentifier) == false);
    
    Mongo::ModuleRecord firstRecord{};
    firstRecord.identifier = firstIdentifier;
    firstRecord.connectionState = Mongo::ConnectionState::Registered;
    firstRecord.ipAddress = "127.0.0.1";
    REQUIRE(modulesCollection.insertOne(std::move(firstRecord)) == true);

    auto firstGetRecord = modulesCollection.getModule(firstIdentifier);
    REQUIRE(firstGetRecord.has_value() == true);
    if(firstGetRecord.has_value() == true) {
        REQUIRE(firstGetRecord->identifier == firstIdentifier);
        REQUIRE(firstGetRecord->connectionState == Mongo::ConnectionState::Registered);
        REQUIRE(firstGetRecord->ipAddress == "127.0.0.1");
    }

    REQUIRE(modulesCollection.findOne(firstIdentifier) == true);

    Types::ModuleIdentifier secondIdentifier{Types::toModuleIdentifier(2)};
    Mongo::ModuleRecord secondRecord{};
    secondRecord.identifier = secondIdentifier;
    secondRecord.connectionState = Mongo::ConnectionState::Registered;
    secondRecord.ipAddress = "127.0.0.1";
    REQUIRE(modulesCollection.insertOne(std::move(secondRecord)) == true);
    REQUIRE(modulesCollection.findOne(secondIdentifier) == true);

    auto recordsInDatabase = modulesCollection.getAllModules();
    REQUIRE(recordsInDatabase.size() == 2);

    modulesCollection.deleteOne(secondIdentifier);
    REQUIRE(modulesCollection.findOne(secondIdentifier) == false);

    REQUIRE(modulesCollection.setDisconnected(firstIdentifier) == true);
    firstGetRecord = modulesCollection.getModule(firstIdentifier);
    REQUIRE(firstGetRecord.has_value() == true);
    if(firstGetRecord.has_value() == true) {
        REQUIRE(firstGetRecord->identifier == firstIdentifier);
        REQUIRE(firstGetRecord->connectionState == Mongo::ConnectionState::Disconnected);
        REQUIRE(firstGetRecord->ipAddress == "127.0.0.1");
    }

    REQUIRE(modulesCollection.setAllAsRegistered() == true);
    firstGetRecord = modulesCollection.getModule(firstIdentifier);
    REQUIRE(firstGetRecord.has_value() == true);
    if(firstGetRecord.has_value() == true) {
        REQUIRE(firstGetRecord->identifier == firstIdentifier);
        REQUIRE(firstGetRecord->connectionState == Mongo::ConnectionState::Registered);
        REQUIRE(firstGetRecord->ipAddress == "127.0.0.1");
    }

    firstGetRecord->connectionState = Mongo::ConnectionState::Connected;
    REQUIRE(modulesCollection.updateModule(std::move(*firstGetRecord)) == true);

    auto postUpdateGet = modulesCollection.getModule(firstIdentifier);
    REQUIRE(postUpdateGet.has_value() == true);
    if(postUpdateGet.has_value() == true) {
        REQUIRE(postUpdateGet->identifier == firstIdentifier);
        REQUIRE(postUpdateGet->connectionState == Mongo::ConnectionState::Connected);
        REQUIRE(postUpdateGet->ipAddress == "127.0.0.1");
    }

    modulesCollection.deleteOne(firstIdentifier);
    REQUIRE(modulesCollection.findOne(secondIdentifier) == false);

    firstGetRecord = modulesCollection.getModule(firstIdentifier);
    REQUIRE(firstGetRecord.has_value() == false);

    REQUIRE(modulesCollection.setAllAsRegistered() == true);
    REQUIRE(modulesCollection.setDisconnected(firstIdentifier) == true);

    modulesCollection.drop();
}