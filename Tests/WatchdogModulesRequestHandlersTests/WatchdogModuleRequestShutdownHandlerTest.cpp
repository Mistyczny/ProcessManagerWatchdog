#include "MongoDbEnvironment.hpp"
#include "MongoModulesCollection.hpp"
#include "Types.hpp"
#include "WatchdogModule.pb.h"
#include "WatchdogModuleRequestsHandlers.hpp"
#include <catch2/catch.hpp>

class MongoDbConnection {
private:
    std::unique_ptr<Mongo::ModulesCollection> modulesCollection{nullptr};

public:
    MongoDbConnection() {
        Mongo::DbEnvironment::initialize();
        auto modulesCollectionEntry = Mongo::DbEnvironment::getInstance()->getClient();
        modulesCollection = std::make_unique<Mongo::ModulesCollection>(*modulesCollectionEntry, "ModulesTest");
        modulesCollection->drop();
        REQUIRE(modulesCollection != nullptr);
    }

    virtual ~MongoDbConnection() { modulesCollection->drop(); }

    std::unique_ptr<Mongo::ModulesCollection>& getModulesCollection() { return this->modulesCollection; }
};

TEST_CASE_METHOD(MongoDbConnection, "Testing watchdog shutdown functionality", "[WatchdogTests]") {
    auto& modulesCollection = *getModulesCollection().get();

    WatchdogModule::ShutdownRequestData shutdownRequest{};
    Watchdog::ModuleAuthenticationData authenticationData{};
    std::string message{};

    SECTION("Parsing invalid message") {
        std::string invalidMessage{};
        Watchdog::ModuleShutdownRequestHandler shutdownHandler{authenticationData, modulesCollection};
        REQUIRE_THROWS_AS(shutdownHandler.createResponse(invalidMessage), Watchdog::ModuleRequestHandlerException);
    }

    SECTION("Module doesnt exists in database") {
        SECTION("Invalid module identifier") {
            shutdownRequest.set_identifier(1);
            shutdownRequest.SerializeToString(&message);

            Watchdog::ModuleShutdownRequestHandler shutdownHandler{authenticationData, modulesCollection};
            REQUIRE_THROWS(shutdownHandler.createResponse(message));
        }

        SECTION("Invalid module identifier") {
            shutdownRequest.set_identifier(Types::toModuleIdentifier(1));
            shutdownRequest.SerializeToString(&message);

            Watchdog::ModuleShutdownRequestHandler shutdownHandler{authenticationData, modulesCollection};
            REQUIRE_THROWS(shutdownHandler.createResponse(message));
        }
    }

    SECTION("Module exists in database - Registered state") {
        modulesCollection.drop();
        Mongo::ModuleRecord record{};
        record.identifier = Types::toModuleIdentifier(1);
        record.connectionState = Mongo::ConnectionState::Registered;
        record.ipAddress = "127.0.0.1";
        modulesCollection.insertOne(std::move(record));

        SECTION("Invalid module identifier") {
            shutdownRequest.set_identifier(1);
            shutdownRequest.SerializeToString(&message);

            Watchdog::ModuleShutdownRequestHandler shutdownHandler{authenticationData, modulesCollection};
            REQUIRE_THROWS(shutdownHandler.createResponse(message));
        }

        SECTION("All parameters are valid") {
            shutdownRequest.set_identifier(Types::toModuleIdentifier(1));
            shutdownRequest.SerializeToString(&message);

            Watchdog::ModuleShutdownRequestHandler shutdownHandler{authenticationData, modulesCollection};
            REQUIRE_THROWS(shutdownHandler.createResponse(message));

            auto updatedRecord = modulesCollection.getModule(Types::toModuleIdentifier(1));
            REQUIRE(updatedRecord.has_value() == true);
            REQUIRE(updatedRecord->connectionState == Mongo::ConnectionState::Registered);
        }
        modulesCollection.drop();
    }

    SECTION("Module exists in database - Connected state") {
        modulesCollection.drop();
        Mongo::ModuleRecord record{};
        record.identifier = Types::toModuleIdentifier(1);
        record.connectionState = Mongo::ConnectionState::Connected;
        record.ipAddress = "127.0.0.1";
        modulesCollection.insertOne(std::move(record));

        SECTION("Invalid module identifier") {
            shutdownRequest.set_identifier(1);
            shutdownRequest.SerializeToString(&message);

            Watchdog::ModuleShutdownRequestHandler shutdownHandler{authenticationData, modulesCollection};
            REQUIRE_THROWS(shutdownHandler.createResponse(message));
        }

        SECTION("All parameters are valid") {
            shutdownRequest.set_identifier(Types::toModuleIdentifier(1));
            shutdownRequest.SerializeToString(&message);

            Watchdog::ModuleShutdownRequestHandler shutdownHandler{authenticationData, modulesCollection};
            REQUIRE_THROWS(shutdownHandler.createResponse(message));

            auto updatedRecord = modulesCollection.getModule(Types::toModuleIdentifier(1));
            REQUIRE(updatedRecord.has_value() == true);
            REQUIRE(updatedRecord->connectionState == Mongo::ConnectionState::Registered);
        }
        modulesCollection.drop();
    }

    SECTION("Module exists in database - Disconnected state") {
        modulesCollection.drop();
        Mongo::ModuleRecord record{};
        record.identifier = Types::toModuleIdentifier(1);
        record.connectionState = Mongo::ConnectionState::Disconnected;
        record.ipAddress = "127.0.0.1";
        modulesCollection.insertOne(std::move(record));

        SECTION("Invalid module identifier") {
            shutdownRequest.set_identifier(1);
            shutdownRequest.SerializeToString(&message);

            Watchdog::ModuleShutdownRequestHandler shutdownHandler{authenticationData, modulesCollection};
            REQUIRE_THROWS(shutdownHandler.createResponse(message));
        }

        SECTION("All parameters are valid") {
            shutdownRequest.set_identifier(Types::toModuleIdentifier(1));
            shutdownRequest.SerializeToString(&message);

            Watchdog::ModuleShutdownRequestHandler shutdownHandler{authenticationData, modulesCollection};
            REQUIRE_THROWS(shutdownHandler.createResponse(message));

            auto updatedRecord = modulesCollection.getModule(Types::toModuleIdentifier(1));
            REQUIRE(updatedRecord.has_value() == true);
            REQUIRE(updatedRecord->connectionState == Mongo::ConnectionState::Registered);
        }
        modulesCollection.drop();
    }
}