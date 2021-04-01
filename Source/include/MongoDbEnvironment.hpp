#pragma once
#include <memory>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>
#include <mutex>
#include <nlohmann/json.hpp>

namespace Mongo {

struct DbConfiguration {
    std::string ip;
    uint32_t port;

    std::string makeAddressString() {
        std::string addressString{"mongodb://"};
        addressString.append(this->ip);
        addressString.append(":");
        addressString.append(std::to_string(port));
        return addressString;
    }
};

class DbConfigurationReader {
private:
    DbConfiguration& dbConfiguration;
    const std::string configurationPath{"/opt/ProcessManager/MongoDbConfiguration.json"};
    nlohmann::json jsonConfig;

    bool read();

public:
    DbConfigurationReader(DbConfiguration&);
    bool readConfiguration();
};

class DbEnvironment {
private:
    mongocxx::instance mongoInstance{};
    mongocxx::pool clientsPool;

    static inline std::unique_ptr<DbEnvironment> instance = nullptr;
    static inline std::mutex mongoEnvironmentLock;

public:
    DbEnvironment(std::string address);
    ~DbEnvironment() = default;
    static bool initialize();

    static constexpr std::unique_ptr<DbEnvironment>& getInstance() { return instance; };
    mongocxx::pool::entry getClient();

    [[nodiscard]] static bool isConnected();
};

} // namespace Mongo