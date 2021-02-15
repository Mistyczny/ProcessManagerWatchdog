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

TEST_CASE_METHOD(MongoDbConnection, "Testing watchdog reconnect functionality", "[WatchdogTests]") {
    srand(time(NULL));

    auto& modulesCollection = *getModulesCollection().get();
    modulesCollection.drop();
    auto setTimer = std::bind([]() { std::cout << "SET TIMER FUNC" << std::endl; });
    WatchdogModule::ReconnectRequestData reconnectRequest{};
    Watchdog::ModuleAuthenticationData authenticationData{};
    authenticationData.sequenceCode = 1410;
    std::string message{};

    SECTION("Parsing invalid message") {
        std::string invalidMessage{};
        Watchdog::ModuleReconnectRequestHandler reconnectHandler{authenticationData, modulesCollection, setTimer};
        REQUIRE_THROWS_AS(reconnectHandler.createResponse(invalidMessage), Watchdog::ModuleRequestHandlerException);
    }

    SECTION("Module doesnt exists in database") {
        modulesCollection.drop();
        SECTION("Invalid module identifier") {
            reconnectRequest.set_identifier(1);
            reconnectRequest.SerializeToString(&message);

            Watchdog::ModuleReconnectRequestHandler reconnectHandler{authenticationData, modulesCollection, setTimer};
            auto response = reconnectHandler.createResponse(message);
            WatchdogModule::ReconnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ReconnectResponseData::NotModuleIdentifier);
            REQUIRE(responseData.has_sequencecode() == false);
        }

        SECTION("All parameters are valid") {
            reconnectRequest.set_identifier(Types::toModuleIdentifier(1));
            reconnectRequest.SerializeToString(&message);

            Watchdog::ModuleReconnectRequestHandler reconnectHandler{authenticationData, modulesCollection, setTimer};
            auto response = reconnectHandler.createResponse(message);
            WatchdogModule::ReconnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ReconnectResponseData::ModuleNotExists);
            REQUIRE(responseData.has_sequencecode() == false);
        }
        modulesCollection.drop();
    }

    SECTION("Module exists in database - Registered state") {
        Mongo::ModuleRecord record{};
        record.identifier = Types::toModuleIdentifier(1);
        record.connectionState = Mongo::ConnectionState::Registered;
        record.ipAddress = "127.0.0.1";
        modulesCollection.insertOne(std::move(record));

        SECTION("Invalid module identifier") {
            reconnectRequest.set_identifier(1);
            reconnectRequest.SerializeToString(&message);

            Watchdog::ModuleReconnectRequestHandler reconnectHandler{authenticationData, modulesCollection, setTimer};
            auto response = reconnectHandler.createResponse(message);
            WatchdogModule::ReconnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ReconnectResponseData::NotModuleIdentifier);
            REQUIRE(responseData.has_sequencecode() == false);
            modulesCollection.drop();
        }

        SECTION("All parameters are valid") {
            reconnectRequest.set_identifier(Types::toModuleIdentifier(1));
            reconnectRequest.SerializeToString(&message);

            Watchdog::ModuleReconnectRequestHandler reconnectHandler{authenticationData, modulesCollection, setTimer};
            auto response = reconnectHandler.createResponse(message);
            WatchdogModule::ReconnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ReconnectResponseData::InvalidConnectionState);
            REQUIRE(responseData.has_sequencecode() == false);
            modulesCollection.drop();
        }
    }

    SECTION("Module exists in database - Connected state") {
        Mongo::ModuleRecord record{};
        record.identifier = Types::toModuleIdentifier(1);
        record.connectionState = Mongo::ConnectionState::Connected;
        record.ipAddress = "127.0.0.1";
        modulesCollection.insertOne(std::move(record));

        SECTION("Invalid module identifier") {
            reconnectRequest.set_identifier(1);
            reconnectRequest.SerializeToString(&message);

            Watchdog::ModuleReconnectRequestHandler reconnectHandler{authenticationData, modulesCollection, setTimer};
            auto response = reconnectHandler.createResponse(message);
            WatchdogModule::ReconnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ReconnectResponseData::NotModuleIdentifier);
            REQUIRE(responseData.has_sequencecode() == false);
            modulesCollection.drop();
        }

        SECTION("All parameters are valid") {
            reconnectRequest.set_identifier(Types::toModuleIdentifier(1));
            reconnectRequest.SerializeToString(&message);

            Watchdog::ModuleReconnectRequestHandler reconnectHandler{authenticationData, modulesCollection, setTimer};
            auto response = reconnectHandler.createResponse(message);
            WatchdogModule::ReconnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ReconnectResponseData::InvalidConnectionState);
            REQUIRE(responseData.has_sequencecode() == false);
            modulesCollection.drop();
        }
    }

    SECTION("Module exists in database - Disconnected state") {
        Mongo::ModuleRecord record{};
        record.identifier = Types::toModuleIdentifier(1);
        record.connectionState = Mongo::ConnectionState::Disconnected;
        record.ipAddress = "127.0.0.1";
        modulesCollection.insertOne(std::move(record));

        SECTION("Invalid module identifier") {
            reconnectRequest.set_identifier(1);
            reconnectRequest.SerializeToString(&message);

            Watchdog::ModuleReconnectRequestHandler reconnectHandler{authenticationData, modulesCollection, setTimer};
            auto response = reconnectHandler.createResponse(message);
            WatchdogModule::ReconnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ReconnectResponseData::NotModuleIdentifier);
            REQUIRE(responseData.has_sequencecode() == false);
            modulesCollection.drop();
        }

        SECTION("All parameters are valid - disconnected state") {
            reconnectRequest.set_identifier(Types::toModuleIdentifier(1));
            reconnectRequest.SerializeToString(&message);

            Watchdog::ModuleReconnectRequestHandler reconnectHandler{authenticationData, modulesCollection, setTimer};
            auto response = reconnectHandler.createResponse(message);
            WatchdogModule::ReconnectResponseData responseData{};
            responseData.ParseFromString(response.body);
            REQUIRE(responseData.responsecode() == WatchdogModule::ReconnectResponseData::Success);
            REQUIRE(responseData.has_sequencecode() == true);
            REQUIRE(responseData.sequencecode() != 1410);
            modulesCollection.drop();
        }
    }

    modulesCollection.drop();
}