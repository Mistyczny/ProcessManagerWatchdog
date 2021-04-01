#include "Logging.hpp"
#include "MongoDbEnvironment.hpp"
#include "WatchdogServer.hpp"
#include <iostream>

int main() {
    srand(time(NULL));
    Log::initialize(Log::LogLevel::INFO);
    Mongo::DbEnvironment::initialize();
    if (!Mongo::DbEnvironment::isConnected()) {
        Log::critical("main: Failed connection to mongoDB");
    } else {
        Watchdog::WatchdogServer watchdog{};
        watchdog.setupSignalHandlers();
        watchdog.setAllConnectedToDisconnectedState();
        if (!watchdog.startAcceptingConnections()) {
            Log::critical("main: Failed to start accepting connections");
        } else if (!watchdog.createWorkingThreads()) {
            Log::critical("main: Failed to create working threads");
        } else {
            watchdog.runIoContext();
        }
    }

    return 0;
}