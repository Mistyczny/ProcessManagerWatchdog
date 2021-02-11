#include "Logging.hpp"
#include "MongoDbEnvironment.hpp"
#include "WatchdogServer.hpp"
#include <iostream>

int main() {
    srand(time(NULL));
    Log::initialize("WatchdogServer", Log::LogLevel::TRACE);
    Mongo::DbEnvironment::initialize("127.0.0.1");
    if (!Mongo::DbEnvironment::isConnected()) {
        Log::critical("main: Failed connection to mongoDB");
    } else {
        Watchdog::WatchdogServer watchdog{};
        watchdog.setupSignalHandlers();
        if (!watchdog.startAcceptingConnections()) {
            Log::critical("main: Failed to start accepting connections");
        } else if (!watchdog.createWorkingThreads()) {
            Log::critical("main: Failed to create working threads");
        } else {
            watchdog.runIoContext();
        }
        Log::critical("Leaving");
    }

    return 0;
}