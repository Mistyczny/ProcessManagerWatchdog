#pragma once
#include "Types.hpp"
#include <boost/asio.hpp>
#include <mongocxx/client.hpp>
#include <optional>

namespace Mongo {

enum class ServiceConnectionState : int32_t { Registered, Connected, Disconnected };

struct ServiceRecord {
    uint16_t port;
    Types::ServiceIdentifier identifier;
    ServiceConnectionState connectionState;
    std::string ipAddress;

    [[nodiscard]] std::string connectionStateToString() const {
        std::string stateAsString{};
        switch (connectionState) {
        case ServiceConnectionState::Registered:
            stateAsString = "Registered";
            break;
        case ServiceConnectionState::Connected:
            stateAsString = "Connected";
            break;
        case ServiceConnectionState::Disconnected:
            stateAsString = "Disconnected";
            break;
        default:
            stateAsString = "Error";
            break;
        }
        return stateAsString;
    }
};

class ServicesCollection {
private:
    mongocxx::collection servicesCollection;

    std::optional<ServiceRecord> viewToServiceRecord(bsoncxx::document::view&);

public:
    ServicesCollection(mongocxx::client& client, std::string collectionName);
    virtual ~ServicesCollection() = default;

    bool insertOne(ServiceRecord&& record);
    std::optional<ServiceRecord> getService(const Types::ServiceIdentifier& moduleIdentifier);
    bool updateService(ServiceRecord&& record);
    void drop();

    bool markAllConnectedAsDisconnected();
};

} // namespace Mongo