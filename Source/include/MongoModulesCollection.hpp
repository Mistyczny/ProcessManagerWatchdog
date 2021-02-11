#pragma once
#include "Types.hpp"
#include <boost/asio.hpp>
#include <mongocxx/client.hpp>
#include <optional>

namespace Mongo {

enum class ConnectionState : int32_t { Registered, Connected, Disconnected };

struct ModuleRecord {
    Types::ModuleIdentifier identifier;
    ConnectionState connectionState;
    std::string ipAddress;

    [[nodiscard]] std::string connectionStateToString() const {
        std::string stateAsString{};
        switch (connectionState) {
        case ConnectionState::Registered:
            stateAsString = "Registered";
            break;
        case ConnectionState::Connected:
            stateAsString = "Connected";
            break;
        case ConnectionState::Disconnected:
            stateAsString = "Disconnected";
            break;
        default:
            stateAsString = "Error";
            break;
        }
        return stateAsString;
    }
};

class ModulesCollection {
private:
    mongocxx::collection modulesCollection;
    std::optional<ModuleRecord> viewToModuleRecord(bsoncxx::document::view&);

public:
    ModulesCollection(mongocxx::client& client, std::string collectionName);
    virtual ~ModulesCollection() = default;

    bool insertOne(ModuleRecord&& record);
    bool findOne(Types::ModuleIdentifier& moduleIdentifier);
    void deleteOne(Types::ModuleIdentifier& moduleIdentifier);
    bool setDisconnected(Types::ModuleIdentifier& moduleIdentifier);
    [[nodiscard]] bool setAllAsRegistered();
    std::optional<ModuleRecord> getModule(Types::ModuleIdentifier& moduleIdentifier);
    std::optional<ModuleRecord> getModule(const Types::ModuleIdentifier& moduleIdentifier);
    void drop();
    std::vector<ModuleRecord> getAllModules();
    bool updateModule(ModuleRecord&& record);
};

} // namespace Mongo