#include "Communication.hpp"
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
        if (!Mongo::DbEnvironment::initialize()) {
            throw std::runtime_error("Failed to initialize connection with mongodb");
        } else if (!Mongo::DbEnvironment::isConnected()) {
            throw std::runtime_error("Not connected with mongodb");
        } else {
            auto modulesCollectionEntry = Mongo::DbEnvironment::getInstance()->getClient();
            modulesCollection = std::make_unique<Mongo::ModulesCollection>(*modulesCollectionEntry, "ModulesTest");
            modulesCollection->drop();
            REQUIRE(modulesCollection != nullptr);
        }
    }

    virtual ~MongoDbConnection() { modulesCollection->drop(); }

    std::unique_ptr<Mongo::ModulesCollection>& getModulesCollection() { return this->modulesCollection; }
};

TEST_CASE_METHOD(MongoDbConnection, "Testing watchdog connect functionality", "[WatchdogTests]") {
    srand(time(NULL));
    auto& modulesCollection = *getModulesCollection().get();
    Watchdog::ModuleAuthenticationData moduleAuthenticationData{};
    WatchdogModule::ConnectRequestData connectRequestData{};
    connectRequestData.set_identifier(Types::toModuleIdentifier(1));
    auto setTimer = std::bind([]() { std::cout << "SET TIMER FUNC" << std::endl; });
    auto message = std::make_unique<Communication::Message<WatchdogModule::Operation>>();
    connectRequestData.SerializeToString(&message->body);

    SECTION("Parsing invalid message") {
        std::string invalidMessage{};
        Watchdog::ModuleConnectRequestHandler connectHandler{moduleAuthenticationData, modulesCollection, setTimer};
        REQUIRE_THROWS_AS(connectHandler.createResponse(invalidMessage), Watchdog::ModuleRequestHandlerException);
    }

    SECTION("Module doesnt exists in database") {
        modulesCollection.drop();
        SECTION("Invalid module identifier") {
            connectRequestData.set_identifier(1);
            connectRequestData.SerializeToString(&message->body);

            Watchdog::ModuleConnectRequestHandler connectHandler{moduleAuthenticationData, modulesCollection, setTimer};
            auto response = connectHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogModule::Operation::ConnectResponse);
            WatchdogModule::ConnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ConnectResponseData::NotModuleIdentifier);
            REQUIRE(responseData.has_sequencecode() == false);

            auto checkRecord = modulesCollection.getModule(Types::toModuleIdentifier(1));
            REQUIRE(checkRecord.has_value() == false);
        }

        SECTION("All parameters are valid") {
            Watchdog::ModuleConnectRequestHandler connectHandler{moduleAuthenticationData, modulesCollection, setTimer};
            auto response = connectHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogModule::Operation::ConnectResponse);
            WatchdogModule::ConnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ConnectResponseData::ModuleNotExists);
            REQUIRE(responseData.has_sequencecode() == false);

            auto checkRecord = modulesCollection.getModule(Types::toModuleIdentifier(1));
            REQUIRE(checkRecord.has_value() == false);
        }
        modulesCollection.drop();
    }

    SECTION("Module exists in database - Registered state") {
        modulesCollection.drop();
        Mongo::ModuleRecord record{};
        record.identifier = Types::toModuleIdentifier(1);
        record.connectionState = Mongo::ConnectionState::Registered;
        record.ipAddress = "127.0.0.1";
        modulesCollection.insertOne(std::move(record));

        SECTION("Invalid module identifier") {
            connectRequestData.set_identifier(1);
            connectRequestData.SerializeToString(&message->body);

            Watchdog::ModuleConnectRequestHandler connectHandler{moduleAuthenticationData, modulesCollection, setTimer};
            auto response = connectHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogModule::Operation::ConnectResponse);
            WatchdogModule::ConnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ConnectResponseData::NotModuleIdentifier);
            REQUIRE(responseData.has_sequencecode() == false);

            auto checkRecord = modulesCollection.getModule(Types::toModuleIdentifier(1));
            REQUIRE(checkRecord.has_value() == true);
            modulesCollection.drop();
        }

        SECTION("All parameters are valid") {
            Watchdog::ModuleConnectRequestHandler connectHandler{moduleAuthenticationData, modulesCollection, setTimer};
            auto response = connectHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogModule::Operation::ConnectResponse);
            WatchdogModule::ConnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ConnectResponseData::Success);
            REQUIRE(responseData.has_sequencecode() == true);
            REQUIRE(responseData.sequencecode() != 0);

            auto checkRecord = modulesCollection.getModule(Types::toModuleIdentifier(1));
            REQUIRE(checkRecord.has_value() == true);
            if (checkRecord.has_value()) {
                REQUIRE(checkRecord->identifier == Types::toModuleIdentifier(1));
                REQUIRE(checkRecord->connectionState == Mongo::ConnectionState::Connected);
                REQUIRE(checkRecord->ipAddress == "127.0.0.1");
            }
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
            modulesCollection.drop();
            connectRequestData.set_identifier(1);
            connectRequestData.SerializeToString(&message->body);

            Watchdog::ModuleConnectRequestHandler connectHandler{moduleAuthenticationData, modulesCollection, setTimer};
            auto response = connectHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogModule::Operation::ConnectResponse);
            WatchdogModule::ConnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ConnectResponseData::NotModuleIdentifier);
            REQUIRE(responseData.has_sequencecode() == false);

            auto checkRecord = modulesCollection.getModule(Types::toModuleIdentifier(1));
            REQUIRE(checkRecord.has_value() == false);
            modulesCollection.drop();
        }

        SECTION("All parameters are valid") {
            Watchdog::ModuleConnectRequestHandler connectHandler{moduleAuthenticationData, modulesCollection, setTimer};
            auto response = connectHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogModule::Operation::ConnectResponse);
            WatchdogModule::ConnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ConnectResponseData::InvalidConnectionState);
            REQUIRE(responseData.has_sequencecode() == false);

            auto checkRecord = modulesCollection.getModule(Types::toModuleIdentifier(1));
            REQUIRE(checkRecord.has_value() == true);
            if (checkRecord.has_value()) {
                REQUIRE(checkRecord->identifier == Types::toModuleIdentifier(1));
                REQUIRE(checkRecord->connectionState == Mongo::ConnectionState::Connected);
                REQUIRE(checkRecord->ipAddress == "127.0.0.1");
            }
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
            modulesCollection.drop();
            connectRequestData.set_identifier(1);
            connectRequestData.SerializeToString(&message->body);

            Watchdog::ModuleConnectRequestHandler connectHandler{moduleAuthenticationData, modulesCollection, setTimer};
            auto response = connectHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogModule::Operation::ConnectResponse);
            WatchdogModule::ConnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ConnectResponseData::NotModuleIdentifier);
            REQUIRE(responseData.has_sequencecode() == false);

            auto checkRecord = modulesCollection.getModule(Types::toModuleIdentifier(1));
            REQUIRE(checkRecord.has_value() == false);
            modulesCollection.drop();
        }

        SECTION("All parameters are valid") {
            Watchdog::ModuleConnectRequestHandler connectHandler{moduleAuthenticationData, modulesCollection, setTimer};
            auto response = connectHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogModule::Operation::ConnectResponse);
            WatchdogModule::ConnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ConnectResponseData::InvalidConnectionState);
            REQUIRE(responseData.has_sequencecode() == false);

            auto checkRecord = modulesCollection.getModule(Types::toModuleIdentifier(1));
            REQUIRE(checkRecord.has_value() == true);
            if (checkRecord.has_value()) {
                REQUIRE(checkRecord->identifier == Types::toModuleIdentifier(1));
                REQUIRE(checkRecord->connectionState == Mongo::ConnectionState::Disconnected);
                REQUIRE(checkRecord->ipAddress == "127.0.0.1");
            }
        }

        modulesCollection.drop();
    }
}