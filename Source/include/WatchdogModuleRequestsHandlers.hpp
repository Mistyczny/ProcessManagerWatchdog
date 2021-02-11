#pragma once
#include "Communication.hpp"
#include "MongoModulesCollection.hpp"
#include "Types.hpp"
#include "WatchdogModule.pb.h"

namespace Watchdog {

struct ModuleAuthenticationData {
    Types::ModuleIdentifier identifier;
    uint32_t sequenceCode;
};

class ModuleRequestHandlerException : std::exception {
public:
    enum class ErrorCode { Dropped, NoResponseRequired, FailedToParse, Unknown };

private:
    ErrorCode errorCode;

public:
    explicit ModuleRequestHandlerException(ErrorCode errorCode) { this->errorCode = errorCode; }
    ~ModuleRequestHandlerException() override = default;
};

class ModuleRequestHandler {
protected:
    ModuleAuthenticationData& authenticationData;
    Communication::Message<WatchdogModule::Operation> responseMessage{};

    uint32_t generateNewSequenceCode(uint32_t oldSequenceCode = 0);

public:
    explicit ModuleRequestHandler(ModuleAuthenticationData& authenticationData);
    virtual ~ModuleRequestHandler() = default;

    [[nodiscard]] virtual Communication::Message<WatchdogModule::Operation> createResponse(std::string& receivedRequest) = 0;
};

class ModuleConnectRequestHandler : public ModuleRequestHandler {
protected:
    Mongo::ModulesCollection& modulesCollection;
    std::function<void()> timerControl;
    WatchdogModule::ConnectRequestData connectRequest;
    WatchdogModule::ConnectResponseData connectResponse;

    void processConnectRequest();

public:
    ModuleConnectRequestHandler(ModuleAuthenticationData&, Mongo::ModulesCollection&, std::function<void()> timerControl);
    ~ModuleConnectRequestHandler() override = default;

    [[nodiscard]] Communication::Message<WatchdogModule::Operation> createResponse(std::string& receivedRequest) override;
};

class ModulePingRequestHandler : public ModuleRequestHandler {
protected:
    std::function<void()> timerControl;
    WatchdogModule::PingRequestData pingRequest;
    WatchdogModule::PingResponseData pingResponse;

public:
    ModulePingRequestHandler(ModuleAuthenticationData&, std::function<void()>);
    ~ModulePingRequestHandler() override = default;

    [[nodiscard]] Communication::Message<WatchdogModule::Operation> createResponse(std::string& receivedRequest) override;
};

class ModuleReconnectRequestHandler : public ModuleRequestHandler {
protected:
    Mongo::ModulesCollection& modulesCollection;
    std::function<void()> timerControl;
    WatchdogModule::ReconnectRequestData reconnectRequest;
    WatchdogModule::ReconnectResponseData reconnectResponse;

    void processReconnectRequest();

public:
    ModuleReconnectRequestHandler(ModuleAuthenticationData&, Mongo::ModulesCollection&, std::function<void()>);
    ~ModuleReconnectRequestHandler() override = default;

    [[nodiscard]] Communication::Message<WatchdogModule::Operation> createResponse(std::string& receivedRequest) override;
};

class ModuleShutdownRequestHandler : public ModuleRequestHandler {
protected:
    Mongo::ModulesCollection& modulesCollection;
    WatchdogModule::ShutdownRequestData shutdownRequest;

    void processShutdownRequest();

public:
    ModuleShutdownRequestHandler(ModuleAuthenticationData&, Mongo::ModulesCollection&);
    ~ModuleShutdownRequestHandler() override = default;

    [[nodiscard]] Communication::Message<WatchdogModule::Operation> createResponse(std::string& receivedRequest) override;
};

} // namespace Watchdog