#include "MongoDbEnvironment.hpp"
#include "Logging.hpp"
#include <fstream>
#include <iostream>

namespace Mongo {

using bsoncxx::builder::basic::array;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

DbConfigurationReader::DbConfigurationReader(DbConfiguration& dbConfiguration) : dbConfiguration{dbConfiguration} {}

bool DbConfigurationReader::readConfiguration() {
    bool readConfiguration{false};
    std::string configFile{configurationPath};
    std::ifstream ifs(configFile);
    if (ifs.is_open()) {
        try {
            jsonConfig = nlohmann::json::parse(ifs);
            readConfiguration = this->read();
        } catch (nlohmann::json::exception& ex) {
            readConfiguration = false;
        }
    }
    return readConfiguration;
}

bool DbConfigurationReader::read() {
    bool read{false};
    if (!jsonConfig.contains("Ip")) {
        Log::critical("Read mongodb configuration file does not contain ip");
    } else if (!jsonConfig.contains("Port")) {
        Log::critical("Read mongodb configuration file does not contain port");
    } else {
        dbConfiguration.ip = jsonConfig["Ip"].get<std::string>();
        dbConfiguration.port = jsonConfig["Port"].get<uint32_t>();
        read = true;
    }
    return read;
}

bool DbEnvironment::initialize() {
    bool isInitialized{true};
    if (!instance) {
        DbConfiguration dbConfiguration{};
        DbConfigurationReader dbConfigurationReader{dbConfiguration};
        if (dbConfigurationReader.readConfiguration()) {
            std::string address = dbConfiguration.makeAddressString();
            try {
                instance = std::make_unique<DbEnvironment>(address);
            } catch (std::exception& ex) {
                isInitialized = false;
            }
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
    }
    return mongoDbConnected;
}

} // namespace Mongo