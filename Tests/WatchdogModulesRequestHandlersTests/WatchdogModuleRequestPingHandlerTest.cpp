#include "WatchdogModule.pb.h"
#include "WatchdogModuleRequestsHandlers.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Testing watchdog ping functionality", "[WatchdogTests]") {
    auto setTimer = std::bind([]() { std::cout << "SET TIMER FUNC" << std::endl; });
    Watchdog::ModuleAuthenticationData moduleAuthenticationData;
    std::string messageBody{};
    WatchdogModule::PingRequestData pingRequest{};

    SECTION("Parsing invalid message") {
        std::string invalidMessage{};
        Watchdog::ModulePingRequestHandler pingHandler{moduleAuthenticationData, setTimer};
        REQUIRE_THROWS_AS(pingHandler.createResponse(invalidMessage), Watchdog::ModuleRequestHandlerException);
    }

    SECTION("Sequence code is valid") {
        moduleAuthenticationData.sequenceCode = 1;
        pingRequest.set_sequencecode(1);
        pingRequest.SerializeToString(&messageBody);

        Watchdog::ModulePingRequestHandler pingHandler{moduleAuthenticationData, setTimer};
        auto response = pingHandler.createResponse(messageBody);
        REQUIRE(response.header.operationCode == WatchdogModule::Operation::PingResponse);
        REQUIRE(response.header.size == response.body.size());
        WatchdogModule::PingResponseData pingResponse{};
        pingResponse.ParseFromString(response.body);
        REQUIRE(pingResponse.sequencecode() == 1);
    }

    SECTION("Sequence code is invalid") {
        moduleAuthenticationData.sequenceCode = 2;
        pingRequest.set_sequencecode(1);
        pingRequest.SerializeToString(&messageBody);

        Watchdog::ModulePingRequestHandler pingHandler{moduleAuthenticationData, setTimer};
        REQUIRE_THROWS(pingHandler.createResponse(messageBody));
    }
}