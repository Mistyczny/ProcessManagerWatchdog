#include "MongoModulesCollection.hpp"
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

ModulesCollection::ModulesCollection(mongocxx::client& client, std::string collectionName)
    : modulesCollection{client["ProcessManager"][collectionName]} {}

bool ModulesCollection::insertOne(ModuleRecord&& record) {
    bool moduleInserted{true};
    auto builder = document{};
    bsoncxx::document::value newModule = builder << "ModuleIdentifier" << record.identifier // To prevent line move by clang
                                                 << "IpAddress" << record.ipAddress         // To prevent line move by clang
                                                 << "ConnectionState" << static_cast<int32_t>(record.connectionState) // Prevent move
                                                 << finalize;
    auto result = modulesCollection.insert_one(std::move(newModule));
    if (!result) {
        moduleInserted = false;
    }
    return moduleInserted;
}

bool ModulesCollection::findOne(Types::ModuleIdentifier& moduleIdentifier) {
    bool found{true};
    auto builder = document{};
    bsoncxx::document::value entry = builder << "ModuleIdentifier" << moduleIdentifier << finalize;

    auto result = modulesCollection.find_one(std::move(entry));
    if (!result) {
        found = false;
    }
    return found;
}

void ModulesCollection::deleteOne(Types::ModuleIdentifier& moduleIdentifier) {
    auto builder = document{};
    bsoncxx::document::value entry = builder << "ModuleIdentifier" << moduleIdentifier << finalize;
    modulesCollection.delete_one(std::move(entry));
}

bool ModulesCollection::setAllAsRegistered() {
    bool allSetAsRegistered{false};
    auto result = modulesCollection.update_many(document{}                                          // To prevent line move by clang
                                                    << "ModuleIdentifier" << open_document          // To prevent line move by clang
                                                    << "$gt" << Types::getMinimalModuleIdentifier() // To prevent line move by clang
                                                    << close_document << finalize,                  // To prevent line move by clang
                                                document{}                                          // To prevent line move by clang
                                                    << "$set" << open_document                      // To prevent line move by clang
                                                    << "ConnectionState" << static_cast<int32_t>(Mongo::ConnectionState::Registered)
                                                    << close_document << finalize);
    if (result) {
        allSetAsRegistered = true;
    }
    return allSetAsRegistered;
}

bool ModulesCollection::setDisconnected(Types::ModuleIdentifier& moduleIdentifier) {
    bool recordUpdated{false};
    auto result =
        modulesCollection.update_one(document{}                                    // To prevent line move by clang
                                         << "ModuleIdentifier" << moduleIdentifier // To prevent line move by clang
                                         << finalize,                              // To prevent line move by clang
                                     document{}                                    // To prevent line move by clang
                                         << "$set" << open_document                // To prevent line move by clang
                                         << "ConnectionState" << static_cast<int32_t>(Mongo::ConnectionState::Disconnected) // Prevent
                                         << close_document << finalize);

    if (result) {
        recordUpdated = true;
    }
    return recordUpdated;
}

std::optional<ModuleRecord> ModulesCollection::viewToModuleRecord(bsoncxx::document::view& view) {
    std::optional<ModuleRecord> moduleRecord{std::nullopt};
    auto modIdentifier = view["ModuleIdentifier"];
    auto connectionState = view["ConnectionState"];
    auto ipAddress = view["IpAddress"];

    if (modIdentifier.type() != bsoncxx::type::k_int32) {
        Log::error("Failed to get module identifier");
    } else if (connectionState.type() != bsoncxx::type::k_int32) {
        Log::error("Failed to get connection state");
    } else if (ipAddress.type() != bsoncxx::type::k_utf8) {
        Log::error("Failed to get ip address");
    } else {
        moduleRecord = std::make_optional<ModuleRecord>();
        moduleRecord->identifier = modIdentifier.get_int32();
        int32_t connState = connectionState.get_int32();
        moduleRecord->connectionState = static_cast<Mongo::ConnectionState>(connState);
        auto adressView = ipAddress.get_utf8().value;
        moduleRecord->ipAddress = adressView.to_string();
    }
    return moduleRecord;
}

std::optional<ModuleRecord> ModulesCollection::getModule(Types::ModuleIdentifier& moduleIdentifier) {
    std::optional<ModuleRecord> moduleRecord{std::nullopt};
    auto builder = document{};
    bsoncxx::document::value entry = builder << "ModuleIdentifier" << moduleIdentifier << finalize;

    auto result = modulesCollection.find_one(std::move(entry));
    if (result) {
        bsoncxx::document::view view{result->view()};
        moduleRecord = this->viewToModuleRecord(view);
    }
    return moduleRecord;
}

void ModulesCollection::drop() { modulesCollection.drop(); }

std::optional<ModuleRecord> ModulesCollection::getModule(const Types::ModuleIdentifier& moduleIdentifier) {
    std::optional<ModuleRecord> moduleRecord{std::nullopt};
    auto builder = document{};
    bsoncxx::document::value entry = builder << "ModuleIdentifier" << moduleIdentifier << finalize;

    auto result = modulesCollection.find_one(std::move(entry));
    if (result) {
        bsoncxx::document::view view{result->view()};
        moduleRecord = this->viewToModuleRecord(view);
    }
    return moduleRecord;
}

std::vector<ModuleRecord> ModulesCollection::getAllModules() {
    std::vector<ModuleRecord> records{};
    auto cursor = modulesCollection.find({});
    for (auto document : cursor) {
        bsoncxx::document::view view{document};
        if (auto moduleRecord = this->viewToModuleRecord(view); moduleRecord.has_value()) {
            records.push_back(std::move(*moduleRecord));
        }
    }
    return records;
}

bool ModulesCollection::updateModule(ModuleRecord&& record) {
    bool recordUpdated{false};
    auto result = modulesCollection.update_one(document{}                                     // To prevent line move by clang
                                                   << "ModuleIdentifier" << record.identifier // To prevent line move by clang
                                                   << finalize,                               // To prevent line move by clang
                                               document{}                                     // To prevent line move by clang
                                                   << "$set" << open_document                 // To prevent line move by clang
                                                   << "ConnectionState" << static_cast<int32_t>(record.connectionState) // Prevent
                                                   << "IpAddress" << record.ipAddress << close_document << finalize);
    if (result) {
        recordUpdated = true;
    }
    return recordUpdated;
}

} // namespace Mongo