#pragma once
#include "Types.hpp"
#include <boost/asio.hpp>
#include <mongocxx/client.hpp>
#include <optional>

namespace Mongo {

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