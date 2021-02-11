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
            auto servicesCollectionEntry = Mongo::DbEnvironment::getInstance()->getClient();
            servicesCollection = std::make_unique<Mongo::ServicesCollection>(*servicesCollectionEntry, "ModulesTest");
            servicesCollection->drop();
            REQUIRE(servicesCollection != nullptr);
        }
    }

    virtual ~MongoDbConnection() { servicesCollection->drop(); }

    std::unique_ptr<Mongo::ServicesCollection>& getServicesCollection() { return this->servicesCollection; }
};

TEST_CASE_METHOD(MongoDbConnection, "Testing watchdog pinging functionality", "[WatchdogTests]") {
    auto& servicesCollection = *getServicesCollection().get();
    auto setTimer = std::bind([]() { std::cout << "SET TIMER FUNC" << std::endl; });

    SECTION("Parsing invalid message") {
        std::string invalidMessage{"abcd"}; // Just random string
        Watchdog::ServiceAuthenticationData serviceAuthenticationData{};
        Watchdog::ServicePingRequestHandler pingRequestHandler{serviceAuthenticationData, std::move(setTimer)};
        REQUIRE_THROWS_AS(pingRequestHandler.createResponse(invalidMessage), Watchdog::ServiceRequestHandlerException);
    }

    SECTION("Sequence code is valid") {
        std::string messageBody{};
        Watchdog::ServiceAuthenticationData serviceAuthenticationData{};
        serviceAuthenticationData.sequenceCode = 1;
        WatchdogService::PingRequestData pingRequestData{};
        pingRequestData.set_sequencecode(1);
        pingRequestData.SerializeToString(&messageBody);

        Watchdog::ServicePingRequestHandler pingRequestHandler{serviceAuthenticationData, std::move(setTimer)};
        auto response = pingRequestHandler.createResponse(messageBody);
        REQUIRE(response.header.operationCode == WatchdogService::Operation::PingResponse);
        REQUIRE(response.header.size == response.body.size());
        WatchdogService::PingResponseData pingResponseData{};
        pingResponseData.ParseFromString(response.body);
        REQUIRE(pingResponseData.sequencecode() == 1);
    }

    SECTION("Sequence code is invalid") {
        std::string messageBody{};
        Watchdog::ServiceAuthenticationData serviceAuthenticationData{};
        serviceAuthenticationData.sequenceCode = 1;
        WatchdogService::PingRequestData pingRequestData{};
        pingRequestData.set_sequencecode(2);
        pingRequestData.SerializeToString(&messageBody);

        Watchdog::ServicePingRequestHandler pingRequestHandler{serviceAuthenticationData, std::move(setTimer)};
        REQUIRE_THROWS(pingRequestHandler.createResponse(messageBody));
    }
}