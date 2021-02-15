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
        if (!Mongo::DbEnvironment::initialize()) {
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

TEST_CASE_METHOD(MongoDbConnection, "Testing watchdog reconnect functionality", "[WatchdogTests]") {
    srand(time(NULL));
    auto& servicesCollection = *getServicesCollection().get();
    auto setTimer = std::bind([]() { std::cout << "SET TIMER FUNC" << std::endl; });

    SECTION("Parsing invalid message") {
        std::string invalidMessage{"abcd"}; // Just random string
        Watchdog::ServiceAuthenticationData serviceAuthenticationData{};
        Watchdog::ServiceReconnectRequestHandler serviceReconnectRequestHandler{serviceAuthenticationData, servicesCollection, setTimer};
        REQUIRE_THROWS_AS(serviceReconnectRequestHandler.createResponse(invalidMessage), Watchdog::ServiceRequestHandlerException);
    }

    SECTION("Service doesnt exists in database - invalid service identifier") {
        servicesCollection.drop();
        WatchdogService::ReconnectRequestData reconnectRequestData{};
        Watchdog::ServiceAuthenticationData serviceAuthenticationData{};
        reconnectRequestData.set_identifier(1);

        auto message = std::make_unique<Communication::Message<WatchdogService::Operation>>();
        reconnectRequestData.SerializeToString(&message->body);

        Watchdog::ServiceReconnectRequestHandler serviceReconnectRequestHandler{serviceAuthenticationData, servicesCollection, setTimer};
        auto response = serviceReconnectRequestHandler.createResponse(message->body);
        REQUIRE(response.header.operationCode == WatchdogService::Operation::ReconnectResponse);
        WatchdogService::ReconnectResponseData reconnectResponseData{};
        reconnectResponseData.ParseFromString(response.body);
        REQUIRE(reconnectResponseData.responsecode() == WatchdogService::NotServiceIdentifier);
        REQUIRE(reconnectResponseData.has_sequencecode() == false);

        auto checkRecord = servicesCollection.getService(Types::toServiceIdentifier(1));
        REQUIRE(checkRecord.has_value() == false);
    }

    SECTION("Service doesnt exists in database - valid service identifier") {
        servicesCollection.drop(); // Ensure that database is empty
        Watchdog::ServiceAuthenticationData serviceAuthenticationData{};
        WatchdogService::ReconnectRequestData reconnectRequestData{};
        reconnectRequestData.set_identifier(Types::toServiceIdentifier(1));

        auto message = std::make_unique<Communication::Message<WatchdogService::Operation>>();
        reconnectRequestData.SerializeToString(&message->body);

        Watchdog::ServiceReconnectRequestHandler serviceReconnectRequestHandler{serviceAuthenticationData, servicesCollection, setTimer};
        auto response = serviceReconnectRequestHandler.createResponse(message->body);
        REQUIRE(response.header.operationCode == WatchdogService::Operation::ReconnectResponse);
        WatchdogService::ReconnectResponseData reconnectResponseData{};
        reconnectResponseData.ParseFromString(response.body);
        REQUIRE(reconnectResponseData.responsecode() == WatchdogService::ServiceNotExists);
        REQUIRE(reconnectResponseData.has_sequencecode() == false);

        auto checkRecord = servicesCollection.getService(Types::toServiceIdentifier(1));
        REQUIRE(checkRecord.has_value() == false);
    }

    SECTION("Module exists in database - Registered state") {
        servicesCollection.drop();
        Mongo::ServiceRecord record{};
        record.identifier = Types::toServiceIdentifier(1);
        record.connectionState = Mongo::ServiceConnectionState::Registered;
        record.ipAddress = "127.0.0.1";
        servicesCollection.insertOne(std::move(record));

        SECTION("Invalid service identifier") {
            Watchdog::ServiceAuthenticationData serviceAuthenticationData{};
            WatchdogService::ReconnectRequestData reconnectRequestData{};
            reconnectRequestData.set_identifier(1);

            auto message = std::make_unique<Communication::Message<WatchdogService::Operation>>();
            reconnectRequestData.SerializeToString(&message->body);

            Watchdog::ServiceReconnectRequestHandler serviceReconnectRequestHandler{serviceAuthenticationData, servicesCollection,
                                                                                    setTimer};
            auto response = serviceReconnectRequestHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogService::Operation::ReconnectResponse);
            WatchdogService::ReconnectResponseData reconnectResponseData{};
            reconnectResponseData.ParseFromString(response.body);
            REQUIRE(reconnectResponseData.responsecode() == WatchdogService::NotServiceIdentifier);
            REQUIRE(reconnectResponseData.has_sequencecode() == false);
        }

        SECTION("Valid service identifier") {
            Watchdog::ServiceAuthenticationData serviceAuthenticationData{};
            WatchdogService::ReconnectRequestData reconnectRequestData{};
            reconnectRequestData.set_identifier(Types::toServiceIdentifier(1));

            auto message = std::make_unique<Communication::Message<WatchdogService::Operation>>();
            reconnectRequestData.SerializeToString(&message->body);

            Watchdog::ServiceReconnectRequestHandler serviceReconnectRequestHandler{serviceAuthenticationData, servicesCollection,
                                                                                    setTimer};
            auto response = serviceReconnectRequestHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogService::Operation::ReconnectResponse);
            WatchdogService::ReconnectResponseData reconnectResponseData{};
            reconnectResponseData.ParseFromString(response.body);
            REQUIRE(reconnectResponseData.responsecode() == WatchdogService::InvalidConnectionState);
            REQUIRE(reconnectResponseData.has_sequencecode() == false);

            auto checkRecord = servicesCollection.getService(Types::toServiceIdentifier(1));
            REQUIRE(checkRecord.has_value() == true);
            if (checkRecord.has_value()) {
                REQUIRE(checkRecord->identifier == Types::toServiceIdentifier(1));
                REQUIRE(checkRecord->connectionState == Mongo::ServiceConnectionState::Registered);
                REQUIRE(checkRecord->ipAddress == "127.0.0.1");
            }
        }
    }

    SECTION("Module exists in database - Connected state") {
        servicesCollection.drop();
        Mongo::ServiceRecord record{};
        record.identifier = Types::toServiceIdentifier(1);
        record.connectionState = Mongo::ServiceConnectionState::Connected;
        record.ipAddress = "127.0.0.1";
        servicesCollection.insertOne(std::move(record));

        SECTION("Invalid service identifier") {
            Watchdog::ServiceAuthenticationData serviceAuthenticationData{};
            WatchdogService::ReconnectRequestData reconnectRequestData{};
            reconnectRequestData.set_identifier(1);

            auto message = std::make_unique<Communication::Message<WatchdogService::Operation>>();
            reconnectRequestData.SerializeToString(&message->body);

            Watchdog::ServiceReconnectRequestHandler serviceReconnectRequestHandler{serviceAuthenticationData, servicesCollection,
                                                                                    setTimer};
            auto response = serviceReconnectRequestHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogService::Operation::ReconnectResponse);
            WatchdogService::ReconnectResponseData reconnectResponseData{};
            reconnectResponseData.ParseFromString(response.body);
            REQUIRE(reconnectResponseData.responsecode() == WatchdogService::NotServiceIdentifier);
            REQUIRE(reconnectResponseData.has_sequencecode() == false);
        }

        SECTION("Valid service identifier") {
            Watchdog::ServiceAuthenticationData serviceAuthenticationData{};
            WatchdogService::ReconnectRequestData reconnectRequestData{};
            reconnectRequestData.set_identifier(Types::toServiceIdentifier(1));

            auto message = std::make_unique<Communication::Message<WatchdogService::Operation>>();
            reconnectRequestData.SerializeToString(&message->body);

            Watchdog::ServiceReconnectRequestHandler serviceReconnectRequestHandler{serviceAuthenticationData, servicesCollection,
                                                                                    setTimer};
            auto response = serviceReconnectRequestHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogService::Operation::ReconnectResponse);
            WatchdogService::ReconnectResponseData reconnectResponseData{};
            reconnectResponseData.ParseFromString(response.body);
            REQUIRE(reconnectResponseData.responsecode() == WatchdogService::InvalidConnectionState);
            REQUIRE(reconnectResponseData.has_sequencecode() == false);

            auto checkRecord = servicesCollection.getService(Types::toServiceIdentifier(1));
            REQUIRE(checkRecord.has_value() == true);
            if (checkRecord.has_value()) {
                REQUIRE(checkRecord->identifier == Types::toServiceIdentifier(1));
                REQUIRE(checkRecord->connectionState == Mongo::ServiceConnectionState::Connected);
                REQUIRE(checkRecord->ipAddress == "127.0.0.1");
            }
        }
    }

    SECTION("Module exists in database - Disconnected state") {
        servicesCollection.drop();
        Mongo::ServiceRecord record{};
        record.identifier = Types::toServiceIdentifier(1);
        record.connectionState = Mongo::ServiceConnectionState::Disconnected;
        record.ipAddress = "127.0.0.1";
        servicesCollection.insertOne(std::move(record));

        SECTION("Invalid service identifier") {
            Watchdog::ServiceAuthenticationData serviceAuthenticationData{};
            WatchdogService::ReconnectRequestData reconnectRequestData{};
            reconnectRequestData.set_identifier(1);

            auto message = std::make_unique<Communication::Message<WatchdogService::Operation>>();
            reconnectRequestData.SerializeToString(&message->body);

            Watchdog::ServiceReconnectRequestHandler serviceReconnectRequestHandler{serviceAuthenticationData, servicesCollection,
                                                                                    setTimer};
            auto response = serviceReconnectRequestHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogService::Operation::ReconnectResponse);
            WatchdogService::ReconnectResponseData reconnectResponseData{};
            reconnectResponseData.ParseFromString(response.body);
            REQUIRE(reconnectResponseData.responsecode() == WatchdogService::NotServiceIdentifier);
            REQUIRE(reconnectResponseData.has_sequencecode() == false);
        }

        SECTION("Valid service identifier") {
            Watchdog::ServiceAuthenticationData serviceAuthenticationData{};
            WatchdogService::ReconnectRequestData reconnectRequestData{};
            reconnectRequestData.set_identifier(Types::toServiceIdentifier(1));

            auto message = std::make_unique<Communication::Message<WatchdogService::Operation>>();
            reconnectRequestData.SerializeToString(&message->body);

            Watchdog::ServiceReconnectRequestHandler serviceReconnectRequestHandler{serviceAuthenticationData, servicesCollection,
                                                                                    setTimer};
            auto response = serviceReconnectRequestHandler.createResponse(message->body);
            REQUIRE(response.header.operationCode == WatchdogService::Operation::ReconnectResponse);
            WatchdogService::ReconnectResponseData reconnectResponseData{};
            reconnectResponseData.ParseFromString(response.body);

            REQUIRE(reconnectResponseData.responsecode() == WatchdogService::Success);
            REQUIRE(reconnectResponseData.has_sequencecode() == true);

            auto checkRecord = servicesCollection.getService(Types::toServiceIdentifier(1));
            REQUIRE(checkRecord.has_value() == true);
            if (checkRecord.has_value()) {
                REQUIRE(checkRecord->identifier == Types::toServiceIdentifier(1));
                REQUIRE(checkRecord->connectionState == Mongo::ServiceConnectionState::Connected);
                REQUIRE(checkRecord->ipAddress == "127.0.0.1");
            }
        }
    }
}