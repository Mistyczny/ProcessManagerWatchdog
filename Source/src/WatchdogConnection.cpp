#include "WatchdogConnection.hpp"
#include <functional>
#include <thread>

namespace Watchdog {

constexpr size_t PingSendingIntervalInMilliseconds = 7000;
constexpr size_t PingTimerExpirationIntervalInMilliseconds = 8000;

ModuleConnection::ModuleConnection(boost::asio::io_context& ioContext, std::map<std::thread::id, Mongo::ModulesCollection>& mCollection)
    : Connection::TcpConnection<WatchdogModule::Operation>(ioContext), timer{ioContext}, modulesCollection{mCollection}, myIdentifier{-1} {}

void ModuleConnection::handleReceivedMessage(std::unique_ptr<Communication::Message<WatchdogModule::Operation>> receivedMessage) {
    auto myDbConnection = modulesCollection.find(std::this_thread::get_id());
    if (myDbConnection == std::end(modulesCollection)) {
        Log::critical("WatchdogConnection::handleReceivedMessage(): Not found suitable mongodb client");
    } else {
        auto& collection = myDbConnection->second;

        if (!receivedMessage) {
            Log::error("handleReceivedMessage: receivedMessage is nullptr");
        } else {
            auto& [messageHeader, messageBody] = *receivedMessage;
            auto responseCreator = this->getRequestHandler(messageHeader.operationCode, collection);
            if (responseCreator) {
                this->createMessageResponse(std::move(responseCreator), messageBody);
            }
        }
    }
}

void ModuleConnection::createMessageResponse(std::unique_ptr<ModuleRequestHandler> responseCreator, std::string& messageBody) {
    if (responseCreator) {
        try {
            auto response = responseCreator->createResponse(messageBody);
            this->sendMessage(response);
        } catch (ModuleRequestHandlerException& exception) {

        } catch (std::exception_ptr& exception) {
            // Handle exception
        }
    }
}

void ModuleConnection::disconnect() {
    auto myDbConnection = this->modulesCollection.find(std::this_thread::get_id());
    if (myDbConnection == std::end(modulesCollection)) {
        Log::critical("WatchdogConnection::disconnect(): Not found suitable mongodb client");
    } else if (this->myIdentifier == -1) {
        Log::error("myIdentifier is not set - cannot set to disconnect state");
    } else {
        auto& collection = myDbConnection->second;
        auto record = collection.getModule(this->myIdentifier);
        if (!record.has_value()) {
            Log::critical("No record to update in database");
        } else if (record->connectionState != Mongo::ConnectionState::Connected) {
            Log::trace("Disconnected in invalid state");
        } else {
            Log::trace("Set disconnected state in database");
            record->connectionState = Mongo::ConnectionState::Disconnected;
            collection.updateModule(std::move(*record));
        }
    }

    if (this->socket->is_open()) {
        this->socket->close();
    }
    this->timer.cancel();
}

void ModuleConnection::onTimerExpiration() {
    Log::info("Timer expired properly");
    auto now = boost::posix_time::microsec_clock::local_time();
    if ((now - last_ping).total_milliseconds() >= PingTimerExpirationIntervalInMilliseconds) {
        Log::error("WatchdogConnection::onTimerExpiration(): Not received ping - disconnecting");
        this->disconnect();
    }
    last_ping = boost::posix_time::microsec_clock::local_time();
}

std::unique_ptr<ModuleRequestHandler> ModuleConnection::getRequestHandler(const WatchdogModule::Operation& operationCode,
                                                                          Mongo::ModulesCollection& mCollection) {
    std::unique_ptr<ModuleRequestHandler> requestHandler{nullptr};
    auto setTimer = std::bind([](auto connection) { connection->setTimerExpiration(PingTimerExpirationIntervalInMilliseconds); },
                              this->shared_from_this());
    switch (operationCode) {
    case WatchdogModule::Operation::ConnectRequest:
        requestHandler = std::make_unique<ModuleConnectRequestHandler>(this->authenticationData, mCollection, setTimer);
        break;
    case WatchdogModule::Operation::PingRequest:
        requestHandler = std::make_unique<ModulePingRequestHandler>(this->authenticationData, setTimer);
        break;
    case WatchdogModule::Operation::ReconnectRequest:
        requestHandler = std::make_unique<ModuleReconnectRequestHandler>(this->authenticationData, mCollection, setTimer);
        break;
    case WatchdogModule::Operation::ShutdownRequest:
        requestHandler = std::make_unique<ModuleShutdownRequestHandler>(this->authenticationData, mCollection);
        break;
    default:
        break;
    }
    return requestHandler;
}

void ModuleConnection::setTimerWaitForConnection() { this->setTimerExpiration(3000); }

ServiceConnection::ServiceConnection(boost::asio::io_context& ioContext,
                                     std::map<std::thread::id, Mongo::ServicesCollection>& servicesCollection)
    : Connection::TcpConnection<WatchdogService::Operation>{ioContext}, servicesCollection{servicesCollection} {}

std::unique_ptr<ServiceRequestHandler> ServiceConnection::getRequestHandler(const WatchdogService::Operation& operationCode,
                                                                            Mongo::ServicesCollection& servicesCollection) {
    std::unique_ptr<ServiceRequestHandler> requestHandler{nullptr};
    auto setTimer = std::bind([](auto connection) { connection->setTimerExpiration(PingTimerExpirationIntervalInMilliseconds); },
                              this->shared_from_this());
    switch (operationCode) {
    case WatchdogService::Operation::ConnectRequest:
        requestHandler = std::make_unique<ServiceConnectRequestHandler>(this->serviceAuthenticationData, servicesCollection, setTimer);
        break;
    case WatchdogService::Operation::PingRequest:
        requestHandler = std::make_unique<ServicePingRequestHandler>(this->serviceAuthenticationData, setTimer);
        break;
    case WatchdogService::Operation::ReconnectRequest:
        requestHandler = std::make_unique<ServiceReconnectRequestHandler>(this->serviceAuthenticationData, servicesCollection, setTimer);
        break;
    case WatchdogService::Operation::ShutdownRequest:
        requestHandler = std::make_unique<ServiceShutdownRequestHandler>(this->serviceAuthenticationData, servicesCollection);
        break;
    default:
        break;
    }
    return requestHandler;
}

void ServiceConnection::handleReceivedMessage(std::unique_ptr<Communication::Message<WatchdogService::Operation>> receivedMessage) {
    auto myDbConnection = this->servicesCollection.find(std::this_thread::get_id());
    if (myDbConnection == std::end(servicesCollection)) {
        Log::critical("WatchdogConnection::handleReceivedMessage(): Not found suitable mongodb client");
    } else {
        auto& collection = myDbConnection->second;

        if (!receivedMessage) {
            Log::error("handleReceivedMessage: receivedMessage is nullptr");
        } else {
            auto& [messageHeader, messageBody] = *receivedMessage;
            auto responseCreator = this->getRequestHandler(messageHeader.operationCode, collection);
            if (responseCreator) {
                this->createMessageResponse(std::move(responseCreator), messageBody);
            }
        }
    }
}

void ServiceConnection::createMessageResponse(std::unique_ptr<ServiceRequestHandler> responseCreator, std::string& messageBody) {
    if (responseCreator) {
        try {
            auto response = responseCreator->createResponse(messageBody);
            this->sendMessage(response);
        } catch (ServiceRequestHandlerException& exception) {
            Log::info("Caught ServiceRequestHandlerException exception");
        } catch (std::exception_ptr& exception) {
            Log::error("Caught standard exception");
        } catch (...) {
            Log::critical("Caught unknown exception");
        }
    }
}

void ServiceConnection::onTimerExpiration() {}

} // namespace Watchdog