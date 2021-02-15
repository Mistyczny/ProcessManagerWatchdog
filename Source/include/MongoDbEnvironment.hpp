#pragma once
#include <memory>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>
#include <mutex>

namespace Mongo {

class DbEnvironment {
private:
    mongocxx::instance mongoInstance{};
    mongocxx::pool clientsPool;

    static inline std::unique_ptr<DbEnvironment> instance = nullptr;
    static inline std::mutex mongoEnvironmentLock;

public:
    DbEnvironment(std::string address);
    ~DbEnvironment() = default;
    static bool initialize(std::string address = "mongodb://localhost:27017");

    static constexpr std::unique_ptr<DbEnvironment>& getInstance() { return instance; };
    mongocxx::pool::entry getClient();

    [[nodiscard]] static bool isConnected();
};

} // namespace Mongo