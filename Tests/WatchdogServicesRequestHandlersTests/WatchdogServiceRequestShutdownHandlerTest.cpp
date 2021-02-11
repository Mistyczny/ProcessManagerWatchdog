#include "MongoDbEnvironment.hpp"
#include "MongoServicesCollection.hpp"
#include "WatchdogService.pb.h"
#include "WatchdogServiceRequestsHandlers.hpp"
#include <catch2/catch.hpp>

class MongoDbConnection {
private:
    std::unique_ptr<Mongo::ServicesCollection> servicesCollection{nullptr};

public:
    MongoDbConnection() {
        if (!Mongo::DbEnvironment::initialize("127.0.0.1")) {
            throw std::runtime_error("Failed to initialize connection with mongodb");
        } else if (!Mongo::DbEnvironment::isConnected()) {
            throw std::runtime_error("Not connected with mongodb");
        } else {
            auto modulesCollectionEntry = Mongo::DbEnvironment::getInstance()->getClient();
            servicesCollection = std::make_unique<Mongo::ServicesCollection>(*modulesCollectionEntry, "ModulesTest");
            servicesCollection->drop();
            REQUIRE(servicesCollection != nullptr);
        }
    }

    virtual ~MongoDbConnection() { servicesCollection->drop(); }

    std::unique_ptr<Mongo::ServicesCollection>& getServicesCollection() { return this->servicesCollection; }
};

TEST_CASE_METHOD(MongoDbConnection, "Testing watchdog connect functionality", "[WatchdogTests]") {
    srand(time(NULL));
    auto& servicesCollection = *getServicesCollection().get();
    Watchdog::ServiceAuthenticationData serviceAuthenticationData{};
    auto setTimer = std::bind([]() { std::cout << "SET TIMER FUNC" << std::endl; });

    SECTION("Parsing invalid message") {
        std::string invalidMessage{};
        Watchdog::ServiceShutdownRequestHandler shutdownRequestHandler{serviceAuthenticationData, servicesCollection};
        REQUIRE_THROWS_AS(shutdownRequestHandler.createResponse(invalidMessage), Watchdog::ServiceRequestHandlerException);
    }

    WatchdogService::ShutdownRequestData shutdownRequestData{};
    auto message = std::make_unique<Communication::Message<WatchdogService::Operation>>();
    SECTION("Module doesnt exists in database") {
        SECTION("Invalid module identifier") {
            shutdownRequestData.set_identifier(1);
            shutdownRequestData.SerializeToString(&message->body);

            Watchdog::ServiceShutdownRequestHandler shutdownRequestHandler{serviceAuthenticationData, servicesCollection};
            REQUIRE_THROWS(shutdownRequestHandler.createResponse(message->body));
        }

        SECTION("Invalid module identifier") {
            shutdownRequestData.set_identifier(Types::toServiceIdentifier(1));
            shutdownRequestData.SerializeToString(&message->body);

            Watchdog::ServiceShutdownRequestHandler shutdownRequestHandler{serviceAuthenticationData, servicesCollection};
            REQUIRE_THROWS(shutdownRequestHandler.createResponse(message->body));
        }
    }

    SECTION("Module exists in database - Registered state") {
        Mongo::ServiceRecord record{};
        record.identifier = Types::toServiceIdentifier(1);
        record.connectionState = Mongo::ServiceConnectionState::Registered;
        record.ipAddress = "127.0.0.1";
        servicesCollection.insertOne(std::move(record));

        SECTION("Invalid module identifier") {
            shutdownRequestData.set_identifier(1);
            shutdownRequestData.SerializeToString(&message->body);

            Watchdog::ServiceShutdownRequestHandler shutdownRequestHandler{serviceAuthenticationData, servicesCollection};
            REQUIRE_THROWS(shutdownRequestHandler.createResponse(message->body));
        }

        SECTION("All parameters are valid") {
            shutdownRequestData.set_identifier(Types::toServiceIdentifier(1));
            shutdownRequestData.SerializeToString(&message->body);

            Watchdog::ServiceShutdownRequestHandler shutdownRequestHandler{serviceAuthenticationData, servicesCollection};
            REQUIRE_THROWS(shutdownRequestHandler.createResponse(message->body));

            auto updatedRecord = servicesCollection.getService(Types::toServiceIdentifier(1));
            REQUIRE(updatedRecord.has_value() == true);
            REQUIRE(updatedRecord->connectionState == Mongo::ServiceConnectionState::Registered);
        }
    }

    SECTION("Module exists in database - Connected state") {
        Mongo::ServiceRecord record{};
        record.identifier = Types::toServiceIdentifier(1);
        record.connectionState = Mongo::ServiceConnectionState::Connected;
        record.ipAddress = "127.0.0.1";
        servicesCollection.insertOne(std::move(record));

        SECTION("Invalid module identifier") {
            shutdownRequestData.set_identifier(1);
            shutdownRequestData.SerializeToString(&message->body);

            Watchdog::ServiceShutdownRequestHandler shutdownRequestHandler{serviceAuthenticationData, servicesCollection};
            REQUIRE_THROWS(shutdownRequestHandler.createResponse(message->body));
        }

        SECTION("All parameters are valid") {
            shutdownRequestData.set_identifier(Types::toServiceIdentifier(1));
            shutdownRequestData.SerializeToString(&message->body);

            Watchdog::ServiceShutdownRequestHandler shutdownRequestHandler{serviceAuthenticationData, servicesCollection};
            REQUIRE_THROWS(shutdownRequestHandler.createResponse(message->body));

            auto updatedRecord = servicesCollection.getService(Types::toServiceIdentifier(1));
            REQUIRE(updatedRecord.has_value() == true);
            REQUIRE(updatedRecord->connectionState == Mongo::ServiceConnectionState::Registered);
        }
    }

    SECTION("Module exists in database - Disconnected state") {
        Mongo::ServiceRecord record{};
        record.identifier = Types::toServiceIdentifier(1);
        record.connectionState = Mongo::ServiceConnectionState::Disconnected;
        record.ipAddress = "127.0.0.1";
        servicesCollection.insertOne(std::move(record));

        SECTION("Invalid module identifier") {
            shutdownRequestData.set_identifier(1);
            shutdownRequestData.SerializeToString(&message->body);

            Watchdog::ServiceShutdownRequestHandler shutdownRequestHandler{serviceAuthenticationData, servicesCollection};
            REQUIRE_THROWS(shutdownRequestHandler.createResponse(message->body));
        }

        SECTION("All parameters are valid") {
            shutdownRequestData.set_identifier(Types::toServiceIdentifier(1));
            shutdownRequestData.SerializeToString(&message->body);

            Watchdog::ServiceShutdownRequestHandler shutdownRequestHandler{serviceAuthenticationData, servicesCollection};
            REQUIRE_THROWS(shutdownRequestHandler.createResponse(message->body));

            auto updatedRecord = servicesCollection.getService(Types::toServiceIdentifier(1));
            REQUIRE(updatedRecord.has_value() == true);
            REQUIRE(updatedRecord->connectionState == Mongo::ServiceConnectionState::Registered);
        }
    }
    servicesCollection.drop();
}