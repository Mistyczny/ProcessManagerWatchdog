#include "MongoDbEnvironment.hpp"
#include <bsoncxx/json.hpp>
#include <iostream>

namespace Mongo {

using bsoncxx::builder::basic::array;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

bool DbEnvironment::initialize(std::string address) {
    bool isInitialized{true};
    if (!instance) {
        try {
            instance = std::make_unique<DbEnvironment>(address);
        } catch (std::exception& ex) {
            isInitialized = false;
        }
    }
    return isInitialized;
}

DbEnvironment::DbEnvironment(std::string address) : mongoInstance{}, clientsPool{mongocxx::uri{address}} {}

mongocxx::pool::entry DbEnvironment::getClient() {
    auto entry = clientsPool.acquire();
    return entry;
}

bool DbEnvironment::isConnected() {
    bool mongoDbConnected{true};
    auto modulesCollectionEntry = DbEnvironment::getInstance()->getClient();
    mongocxx::client& client = *modulesCollectionEntry;
    bsoncxx::document::value command = make_document(kvp("ping", 1));
    bsoncxx::document::view command_view = command.view();
    auto db = client["ProcessManager"];
    try {
        auto res = db.run_command(command_view);
        if (res.view()["ok"].get_double() != double(1)) {
            mongoDbConnected = false;
        }
    } catch (std::exception&) {
        mongoDbConnected = false;
        std::cout << "TIMED OUT" << std::endl;
    }
    return mongoDbConnected;
}

} // namespace Mongo