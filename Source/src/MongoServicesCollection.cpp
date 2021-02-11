#include "MongoServicesCollection.hpp"
#include "Logging.hpp"
#include "MongoDbEnvironment.hpp"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

namespace Mongo {

ServicesCollection::ServicesCollection(mongocxx::client& client, std::string collectionName)
    : servicesCollection{client["ProcessManager"][collectionName]} {}

void ServicesCollection::drop() { servicesCollection.drop(); }

std::optional<ServiceRecord> ServicesCollection::viewToServiceRecord(bsoncxx::document::view& view) {
    std::optional<ServiceRecord> serviceRecord{std::nullopt};
    auto serviceIdentifier = view["ServiceIdentifier"];
    auto connectionState = view["ConnectionState"];
    auto ipAddress = view["IpAddress"];

    if (serviceIdentifier.type() != bsoncxx::type::k_int32) {
        Log::error("Failed to get module identifier");
    } else if (connectionState.type() != bsoncxx::type::k_int32) {
        Log::error("Failed to get connection state");
    } else if (ipAddress.type() != bsoncxx::type::k_utf8) {
        Log::error("Failed to get ip address");
    } else {
        serviceRecord = std::make_optional<ServiceRecord>();
        serviceRecord->identifier = serviceIdentifier.get_int32();
        int32_t connState = connectionState.get_int32();
        serviceRecord->connectionState = static_cast<Mongo::ServiceConnectionState>(connState);
        auto adressView = ipAddress.get_utf8().value;
        serviceRecord->ipAddress = adressView.to_string();
    }
    return serviceRecord;
}

bool ServicesCollection::insertOne(ServiceRecord&& record) {
    bool serviceInserted{true};
    auto builder = document{};
    bsoncxx::document::value newService = builder << "ServiceIdentifier" << record.identifier // To prevent line move by clang
                                                  << "IpAddress" << record.ipAddress          // To prevent line move by clang
                                                  << "ConnectionState" << static_cast<int32_t>(record.connectionState) // Prevent move
                                                  << finalize;
    auto result = servicesCollection.insert_one(std::move(newService));
    if (!result) {
        serviceInserted = false;
    }
    return serviceInserted;
}

std::optional<ServiceRecord> ServicesCollection::getService(const Types::ServiceIdentifier& serviceIdentifier) {
    std::optional<ServiceRecord> serviceRecord{std::nullopt};
    auto builder = document{};
    bsoncxx::document::value entry = builder << "ServiceIdentifier" << serviceIdentifier << finalize;

    auto result = servicesCollection.find_one(std::move(entry));
    if (result) {
        bsoncxx::document::view view{result->view()};
        serviceRecord = this->viewToServiceRecord(view);
    }
    return serviceRecord;
}

bool ServicesCollection::updateService(ServiceRecord&& record) {
    bool recordUpdated{false};
    auto result = servicesCollection.update_one(document{}                                      // To prevent line move by clang
                                                    << "ServiceIdentifier" << record.identifier // To prevent line move by clang
                                                    << finalize,                                // To prevent line move by clang
                                                document{}                                      // To prevent line move by clang
                                                    << "$set" << open_document                  // To prevent line move by clang
                                                    << "ConnectionState" << static_cast<int32_t>(record.connectionState) // Prevent
                                                    << "IpAddress" << record.ipAddress << close_document << finalize);
    if (result) {
        recordUpdated = true;
    }
    return recordUpdated;
}

} // namespace Mongo