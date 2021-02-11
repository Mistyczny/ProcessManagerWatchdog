#include "WatchdogServiceRequestsHandlers.hpp"
#include "Logging.hpp"

namespace Watchdog {

ServiceRequestHandler::ServiceRequestHandler(ServiceAuthenticationData& authenticationData) : authenticationData{authenticationData} {}

uint32_t ServiceRequestHandler::generateNewSequenceCode(uint32_t oldSequenceCode) {
    uint32_t newSequenceCode = rand() % 1000 + 1;
    while (newSequenceCode == oldSequenceCode) {
        newSequenceCode = rand() % 1000 + 1;
    }
    return newSequenceCode;
}

ServiceConnectRequestHandler::ServiceConnectRequestHandler(ServiceAuthenticationData& authorizationData,
                                                           Mongo::ServicesCollection& servicesCollection,
                                                           std::function<void()> timerControl)
    : ServiceRequestHandler{authorizationData}, servicesCollection{servicesCollection}, timerControl{std::move(timerControl)} {
    this->responseMessage.header.operationCode = WatchdogService::Operation::ConnectResponse;
}

Communication::Message<WatchdogService::Operation> ServiceConnectRequestHandler::createResponse(std::string& receivedRequest) {
    if (!this->connectRequestData.ParseFromString(receivedRequest)) {
        Log::error("Failed to parse received service connect request");
        throw ServiceRequestHandlerException{ServiceRequestHandlerException::ErrorCode::FailedToParse};
    }
    this->processConnectRequest();
    this->connectResponseData.SerializeToString(&responseMessage.body);
    this->responseMessage.header.size = responseMessage.body.size();
    return this->responseMessage;
}

void ServiceConnectRequestHandler::processConnectRequest() {
    if (!Types::isServiceIdentifier(connectRequestData.identifier())) {
        this->connectResponseData.set_responsecode(WatchdogService::NotServiceIdentifier);
    } else {
        const Types::ServiceIdentifier& serviceIdentifier = connectRequestData.identifier();
        auto serviceRecord = servicesCollection.getService(serviceIdentifier);
        if (!serviceRecord.has_value()) {
            this->connectResponseData.set_responsecode(WatchdogService::ServiceNotExists);
        } else if (serviceRecord->connectionState == Mongo::ServiceConnectionState::Connected) {
            this->connectResponseData.set_responsecode(WatchdogService::InvalidConnectionState);
        } else {
            serviceRecord->connectionState = Mongo::ServiceConnectionState::Connected;
            this->authenticationData.identifier = serviceIdentifier;
            if (this->servicesCollection.updateService(std::move(*serviceRecord))) {
                this->authenticationData.sequenceCode = this->generateNewSequenceCode();
                this->connectResponseData.set_responsecode(WatchdogService::Success);
                this->connectResponseData.set_sequencecode(this->authenticationData.sequenceCode);
                this->timerControl();
            }
        }
    }
}

ServicePingRequestHandler::ServicePingRequestHandler(ServiceAuthenticationData& authorizationData, std::function<void()> timerControl)
    : ServiceRequestHandler{authorizationData}, timerControl{std::move(timerControl)} {
    this->responseMessage.header.operationCode = WatchdogService::Operation::PingResponse;
}

Communication::Message<WatchdogService::Operation> ServicePingRequestHandler::createResponse(std::string& receivedRequest) {
    if (!this->pingRequestData.ParseFromString(receivedRequest)) {
        Log::error("Failed to parse received module ping request");
        throw ServiceRequestHandlerException{ServiceRequestHandlerException::ErrorCode::FailedToParse};
    }
    if (pingRequestData.sequencecode() == this->authenticationData.sequenceCode) {
        this->pingResponseData.set_sequencecode(pingRequestData.sequencecode());
        // Start timer to tick
        this->timerControl();
    } else {
        throw ServiceRequestHandlerException(ServiceRequestHandlerException::ErrorCode::Dropped);
    }
    this->pingResponseData.SerializeToString(&responseMessage.body);
    this->responseMessage.header.size = responseMessage.body.size();
    return this->responseMessage;
}

ServiceReconnectRequestHandler::ServiceReconnectRequestHandler(ServiceAuthenticationData& authorizationData,
                                                               Mongo::ServicesCollection& servicesCollection,
                                                               std::function<void()> timerControl)
    : ServiceRequestHandler{authorizationData}, servicesCollection{servicesCollection}, timerControl{timerControl} {
    this->responseMessage.header.operationCode = WatchdogService::Operation::ReconnectResponse;
}

Communication::Message<WatchdogService::Operation> ServiceReconnectRequestHandler::createResponse(std::string& receivedRequest) {
    if (!this->reconnectRequestData.ParseFromString(receivedRequest)) {
        Log::error("Failed to parse received service connect request");
        throw ServiceRequestHandlerException{ServiceRequestHandlerException::ErrorCode::FailedToParse};
    }
    this->processReconnectRequest();
    this->reconnectResponseData.SerializeToString(&responseMessage.body);
    this->responseMessage.header.size = responseMessage.body.size();
    return this->responseMessage;
}

void ServiceReconnectRequestHandler::processReconnectRequest() {
    if (!Types::isServiceIdentifier(reconnectRequestData.identifier())) {
        this->reconnectResponseData.set_responsecode(WatchdogService::NotServiceIdentifier);
    } else {
        const Types::ServiceIdentifier& moduleIdentifier = reconnectRequestData.identifier();
        auto serviceRecord = servicesCollection.getService(moduleIdentifier);
        if (!serviceRecord.has_value()) {
            this->reconnectResponseData.set_responsecode(WatchdogService::ServiceNotExists);
        } else if (serviceRecord->connectionState != Mongo::ServiceConnectionState::Disconnected) {
            this->reconnectResponseData.set_responsecode(WatchdogService::InvalidConnectionState);
        } else {
            serviceRecord->connectionState = Mongo::ServiceConnectionState::Connected;
            if (servicesCollection.updateService(std::move(*serviceRecord))) {
                this->authenticationData.sequenceCode = this->generateNewSequenceCode(this->authenticationData.sequenceCode);
                this->reconnectResponseData.set_sequencecode(this->authenticationData.sequenceCode);
                this->reconnectResponseData.set_responsecode(WatchdogService::Success);
                this->timerControl();
            }
        }
    }
}

ServiceShutdownRequestHandler::ServiceShutdownRequestHandler(ServiceAuthenticationData& authorizationData,
                                                             Mongo::ServicesCollection& servicesCollection)
    : ServiceRequestHandler{authorizationData}, servicesCollection{servicesCollection} {}

Communication::Message<WatchdogService::Operation> ServiceShutdownRequestHandler::createResponse(std::string& receivedRequest) {
    if (!this->shutdownRequestData.ParseFromString(receivedRequest)) {
        Log::error("Failed to parse received module ping request");
        throw ServiceRequestHandlerException{ServiceRequestHandlerException::ErrorCode::FailedToParse};
    } else if (!Types::isServiceIdentifier(shutdownRequestData.identifier())) {
        throw ServiceRequestHandlerException(ServiceRequestHandlerException::ErrorCode::Dropped);
    } else {
        const Types::ModuleIdentifier& serviceIdentifier = shutdownRequestData.identifier();
        auto serviceRecord = servicesCollection.getService(serviceIdentifier);
        if (!serviceRecord.has_value()) {
            throw ServiceRequestHandlerException(ServiceRequestHandlerException::ErrorCode::Dropped);
        } else {
            serviceRecord->connectionState = Mongo::ServiceConnectionState::Registered;
            auto moduleUpdated = this->servicesCollection.updateService(std::move(*serviceRecord));
            if (!moduleUpdated) {
                Log::error("Unable to update module");
            }
        }
    }
    throw ServiceRequestHandlerException(ServiceRequestHandlerException::ErrorCode::NoResponseRequired);
}

} // namespace Watchdog