#pragma once
#include "Communication.hpp"
#include "Connection.hpp"
#include "Logging.hpp"
#include "MongoModulesCollection.hpp"
#include "MongoServicesCollection.hpp"
#include "WatchdogModule.pb.h"
#include "WatchdogModuleRequestsHandlers.hpp"
#include "WatchdogService.pb.h"
#include "WatchdogServiceRequestsHandlers.hpp"
#include <boost/asio.hpp>
#include <iostream>

namespace Watchdog {

class ModuleConnection : public Connection::TcpConnection<WatchdogModule::Operation> {
protected:
    uint32_t sequenceCode{};
    ModuleAuthenticationData authenticationData{};
    std::map<std::thread::id, Mongo::ModulesCollection>& modulesCollection;
    boost::asio::deadline_timer timer;
    boost::asio::ip::tcp::endpoint clientEndpoint;

    void onTimerExpiration() override;
    void createMessageResponse(std::unique_ptr<ModuleRequestHandler>, std::string& messageBody);

    std::unique_ptr<ModuleRequestHandler> getRequestHandler(const WatchdogModule::Operation&, Mongo::ModulesCollection&);

public:
    ModuleConnection(boost::asio::io_context& ioContext, std::map<std::thread::id, Mongo::ModulesCollection>&);
    ~ModuleConnection() override { Log::debug("Module connection terminated"); }

    void handleReceivedMessage(std::unique_ptr<Communication::Message<WatchdogModule::Operation>> receivedMessage) override;
    void disconnect() override;

    void setTimerWaitForConnection();
};

class ServiceConnection : public Connection::TcpConnection<WatchdogService::Operation> {
protected:
    std::map<std::thread::id, Mongo::ServicesCollection>& servicesCollection;
    ServiceAuthenticationData serviceAuthenticationData;

    void handleReceivedMessage(std::unique_ptr<Communication::Message<WatchdogService::Operation>> receivedMessage) override;
    void onTimerExpiration() override;

    void createMessageResponse(std::unique_ptr<ServiceRequestHandler>, std::string& messageBody);
    std::unique_ptr<ServiceRequestHandler> getRequestHandler(const WatchdogService::Operation&, Mongo::ServicesCollection&);

public:
    explicit ServiceConnection(boost::asio::io_context& ioContext, std::map<std::thread::id, Mongo::ServicesCollection>&);
    ~ServiceConnection() override = default;
};

} // namespace Watchdog