#include "MongoDbEnvironment.hpp"
#include "MongoServicesCollection.hpp"
#include "Types.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Tests modules collection functionalities", "[MongoDatabase]") {
    Mongo::DbEnvironment::initialize("127.0.0.1");
    auto servicesCollectionEntry = Mongo::DbEnvironment::getInstance()->getClient();
    Mongo::ServicesCollection servicesCollection{*servicesCollectionEntry, "ServicesTest"};
    servicesCollection.drop();

    Types::ServiceIdentifier firstIdentifier{Types::toServiceIdentifier(1)};
    Mongo::ServiceRecord firstRecord{};
    firstRecord.identifier = firstIdentifier;
    firstRecord.connectionState = Mongo::ServiceConnectionState::Registered;
    firstRecord.ipAddress = "127.0.0.1";
    REQUIRE(servicesCollection.insertOne(std::move(firstRecord)) == true);

    auto firstGetRecord = servicesCollection.getService(firstIdentifier);
    REQUIRE(firstGetRecord.has_value() == true);
    if (firstGetRecord.has_value() == true) {
        REQUIRE(firstGetRecord->identifier == firstIdentifier);
        REQUIRE(firstGetRecord->connectionState == Mongo::ServiceConnectionState::Registered);
        REQUIRE(firstGetRecord->ipAddress == "127.0.0.1");
    }

    Mongo::ServiceRecord updatingRecord{};
    updatingRecord.identifier = firstIdentifier;
    updatingRecord.connectionState = Mongo::ServiceConnectionState::Disconnected;
    updatingRecord.ipAddress = "127.1.5.1";

    REQUIRE(servicesCollection.updateService(std::move(updatingRecord)) == true);

    auto updatedRecord = servicesCollection.getService(firstIdentifier);
    REQUIRE(updatedRecord.has_value() == true);
    if (updatedRecord.has_value() == true) {
        REQUIRE(updatedRecord->identifier == firstIdentifier);
        REQUIRE(updatedRecord->connectionState == Mongo::ServiceConnectionState::Disconnected);
        REQUIRE(updatedRecord->ipAddress == "127.1.5.1");
    }

    servicesCollection.drop();
}