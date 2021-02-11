#pragma once
#include "Communication.hpp"
#include "MongoServicesCollection.hpp"
#include "Types.hpp"
#include "WatchdogService.pb.h"
#include <boost/asio.hpp>

namespace Watchdog {

struct ServiceAuthenticationData {
    Types::ServiceIdentifier identifier;
    uint32_t sequenceCode;
    boost::asio::ip::tcp::endpoint remoteEndpoint;
};

class ServiceRequestHandlerException : std::exception {
public:
    enum class ErrorCode { Dropped, NoResponseRequired, FailedToParse, Unknown };

    explicit ServiceRequestHandlerException(ErrorCode errorCode) { this->errorCode = errorCode; }
    ~ServiceRequestHandlerException() override = default;

private:
    ErrorCode errorCode;
};

class ServiceRequestHandler {
protected:
    ServiceAuthenticationData& authenticationData;
    Communication::Message<WatchdogService::Operation> responseMessage{};

    uint32_t generateNewSequenceCode(uint32_t oldSequenceCode = 0);

public:
    explicit ServiceRequestHandler(ServiceAuthenticationData&);
    virtual ~ServiceRequestHandler() = default;

    [[nodiscard]] virtual Communication::Message<WatchdogService::Operation> createResponse(std::string& receivedRequest) = 0;
};

class ServiceConnectRequestHandler : public ServiceRequestHandler {
protected:
    Mongo::ServicesCollection& servicesCollection;
    std::function<void()> timerControl;
    WatchdogService::ConnectRequestData connectRequestData{};
    WatchdogService::ConnectResponseData connectResponseData{};

    void processConnectRequest();

public:
    explicit ServiceConnectRequestHandler(ServiceAuthenticationData&, Mongo::ServicesCollection&, std::function<void()>);
    ~ServiceConnectRequestHandler() override = default;

    [[nodiscard]] Communication::Message<WatchdogService::Operation> createResponse(std::string& receivedRequest);
};

class ServicePingRequestHandler : public ServiceRequestHandler {
protected:
    std::function<void()> timerControl;
    WatchdogService::PingRequestData pingRequestData{};
    WatchdogService::PingResponseData pingResponseData{};

public:
    explicit ServicePingRequestHandler(ServiceAuthenticationData&, std::function<void()>);
    ~ServicePingRequestHandler() override = default;

    [[nodiscard]] Communication::Message<WatchdogService::Operation> createResponse(std::string& receivedRequest);
};

class ServiceReconnectRequestHandler : public ServiceRequestHandler {
protected:
    Mongo::ServicesCollection& servicesCollection;
    std::function<void()> timerControl;
    WatchdogService::ReconnectRequestData reconnectRequestData{};
    WatchdogService::ReconnectResponseData reconnectResponseData{};

    void processReconnectRequest();

public:
    explicit ServiceReconnectRequestHandler(ServiceAuthenticationData&, Mongo::ServicesCollection&, std::function<void()>);
    ~ServiceReconnectRequestHandler() override = default;

    [[nodiscard]] Communication::Message<WatchdogService::Operation> createResponse(std::string& receivedRequest);
};

class ServiceShutdownRequestHandler : public ServiceRequestHandler {
protected:
    Mongo::ServicesCollection& servicesCollection;
    WatchdogService::ShutdownRequestData shutdownRequestData{};

public:
    explicit ServiceShutdownRequestHandler(ServiceAuthenticationData&, Mongo::ServicesCollection&);
    ~ServiceShutdownRequestHandler() override = default;

    [[nodiscard]] Communication::Message<WatchdogService::Operation> createResponse(std::string& receivedRequest);
};

} // namespace Watchdog