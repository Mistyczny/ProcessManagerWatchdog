#include "WatchdogModuleRequestsHandlers.hpp"
#include "Logging.hpp"
#include <utility>

namespace Watchdog {

ModuleRequestHandler::ModuleRequestHandler(ModuleAuthenticationData& authenticationData) : authenticationData{authenticationData} {}

uint32_t ModuleRequestHandler::generateNewSequenceCode(uint32_t oldSequenceCode) {
    uint32_t newSequenceCode = rand() % 1000 + 1;
    while (newSequenceCode == oldSequenceCode) {
        newSequenceCode = rand() % 1000 + 1;
    }
    return newSequenceCode;
}

ModuleConnectRequestHandler::ModuleConnectRequestHandler(ModuleAuthenticationData& authenticationData,
                                                         Mongo::ModulesCollection& modulesCollection, std::function<void()> timerControl)
    : ModuleRequestHandler{authenticationData}, modulesCollection{modulesCollection}, timerControl{std::move(timerControl)} {
    this->responseMessage.header.operationCode = WatchdogModule::Operation::ConnectResponse;
}

Communication::Message<WatchdogModule::Operation> ModuleConnectRequestHandler::createResponse(std::string& receivedMessage) {
    if (!this->connectRequest.ParseFromString(receivedMessage)) {
        Log::error("Failed to parse received module connect request");
        throw ModuleRequestHandlerException{ModuleRequestHandlerException::ErrorCode::FailedToParse};
    }
    this->processConnectRequest();
    this->connectResponse.SerializeToString(&responseMessage.body);
    this->responseMessage.header.size = responseMessage.body.size();
    return this->responseMessage;
}

void ModuleConnectRequestHandler::processConnectRequest() {
    if (!Types::isModuleIdentifier(connectRequest.identifier())) {
        this->connectResponse.set_responsecode(WatchdogModule::ConnectResponseData::NotModuleIdentifier);
    } else {
        const Types::ModuleIdentifier& moduleIdentifier = connectRequest.identifier();
        auto moduleRecord = modulesCollection.getModule(moduleIdentifier);
        if (!moduleRecord.has_value()) {
            this->connectResponse.set_responsecode(WatchdogModule::ConnectResponseData::ModuleNotExists);
        } else if (moduleRecord->connectionState != Mongo::ConnectionState::Registered) {
            this->connectResponse.set_responsecode(WatchdogModule::ConnectResponseData::InvalidConnectionState);
        } else {
            moduleRecord->connectionState = Mongo::ConnectionState::Connected;
            this->authenticationData.identifier = connectRequest.identifier();
            if (modulesCollection.updateModule(std::move(*moduleRecord))) {
                this->authenticationData.sequenceCode = this->generateNewSequenceCode();
                this->connectResponse.set_responsecode(WatchdogModule::ConnectResponseData::Success);
                this->connectResponse.set_sequencecode(this->authenticationData.sequenceCode);
                this->timerControl();
            }
        }
    }
}

ModulePingRequestHandler::ModulePingRequestHandler(ModuleAuthenticationData& authenticationData, std::function<void()> timerControl)
    : ModuleRequestHandler{authenticationData}, timerControl{std::move(timerControl)} {
    this->responseMessage.header.operationCode = WatchdogModule::Operation::PingResponse;
}

Communication::Message<WatchdogModule::Operation> ModulePingRequestHandler::createResponse(std::string& receivedMessage) {
    if (!this->pingRequest.ParseFromString(receivedMessage)) {
        Log::error("Failed to parse received module ping request");
        throw ModuleRequestHandlerException{ModuleRequestHandlerException::ErrorCode::FailedToParse};
    }
    if (pingRequest.sequencecode() == this->authenticationData.sequenceCode) {
        this->pingResponse.set_sequencecode(pingRequest.sequencecode());
        // Start timer to tick
        this->timerControl();
    } else {
        throw ModuleRequestHandlerException(ModuleRequestHandlerException::ErrorCode::Dropped);
    }
    this->pingResponse.SerializeToString(&responseMessage.body);
    this->responseMessage.header.size = responseMessage.body.size();
    return this->responseMessage;
}

ModuleReconnectRequestHandler::ModuleReconnectRequestHandler(ModuleAuthenticationData& authenticationData,
                                                             Mongo::ModulesCollection& modulesCollection,
                                                             std::function<void()> timerControl)
    : ModuleRequestHandler{authenticationData}, modulesCollection{modulesCollection}, timerControl{std::move(timerControl)} {
    this->responseMessage.header.operationCode = WatchdogModule::Operation::ReconnectResponse;
}

Communication::Message<WatchdogModule::Operation> ModuleReconnectRequestHandler::createResponse(std::string& receivedMessage) {
    if (!this->reconnectRequest.ParseFromString(receivedMessage)) {
        Log::error("Failed to parse received module ping request");
        throw ModuleRequestHandlerException{ModuleRequestHandlerException::ErrorCode::FailedToParse};
    }
    this->processReconnectRequest();
    this->reconnectResponse.SerializeToString(&responseMessage.body);
    this->responseMessage.header.size = responseMessage.body.size();
    return this->responseMessage;
}

void ModuleReconnectRequestHandler::processReconnectRequest() {
    if (!Types::isModuleIdentifier(reconnectRequest.identifier())) {
        this->reconnectResponse.set_responsecode(WatchdogModule::ReconnectResponseData::NotModuleIdentifier);
    } else {
        const Types::ModuleIdentifier& moduleIdentifier = reconnectRequest.identifier();
        auto moduleRecord = modulesCollection.getModule(moduleIdentifier);
        if (!moduleRecord.has_value()) {
            this->reconnectResponse.set_responsecode(WatchdogModule::ReconnectResponseData::ModuleNotExists);
        } else if (moduleRecord->connectionState != Mongo::ConnectionState::Disconnected) {
            this->reconnectResponse.set_responsecode(WatchdogModule::ReconnectResponseData::InvalidConnectionState);
        } else {
            moduleRecord->connectionState = Mongo::ConnectionState::Connected;
            if (modulesCollection.updateModule(std::move(*moduleRecord))) {
                this->authenticationData.sequenceCode = this->generateNewSequenceCode(this->authenticationData.sequenceCode);
                this->reconnectResponse.set_sequencecode(this->authenticationData.sequenceCode);
                this->reconnectResponse.set_responsecode(WatchdogModule::ReconnectResponseData::Success);
                this->timerControl();
            }
        }
    }
}

ModuleShutdownRequestHandler::ModuleShutdownRequestHandler(ModuleAuthenticationData& authenticationData,
                                                           Mongo::ModulesCollection& modulesCollection)
    : ModuleRequestHandler{authenticationData}, modulesCollection{modulesCollection} {}

Communication::Message<WatchdogModule::Operation> ModuleShutdownRequestHandler::createResponse(std::string& receivedRequest) {
    if (!this->shutdownRequest.ParseFromString(receivedRequest)) {
        Log::error("Failed to parse received module ping request");
        throw ModuleRequestHandlerException{ModuleRequestHandlerException::ErrorCode::FailedToParse};
    }
    this->processShutdownRequest();
    throw ModuleRequestHandlerException(ModuleRequestHandlerException::ErrorCode::NoResponseRequired);
}

void ModuleShutdownRequestHandler::processShutdownRequest() {
    if (!Types::isModuleIdentifier(shutdownRequest.identifier())) {
        throw ModuleRequestHandlerException(ModuleRequestHandlerException::ErrorCode::Dropped);
    } else {
        const Types::ModuleIdentifier& moduleIdentifier = shutdownRequest.identifier();
        auto moduleRecord = modulesCollection.getModule(moduleIdentifier);
        if (!moduleRecord.has_value()) {
            throw ModuleRequestHandlerException(ModuleRequestHandlerException::ErrorCode::Dropped);
        } else {
            moduleRecord->connectionState = Mongo::ConnectionState::Registered;
            auto moduleUpdated = this->modulesCollection.updateModule(std::move(*moduleRecord));
            if (!moduleUpdated) {
                Log::error("Unable to update module");
            }
        }
    }
}

} // namespace Watchdog