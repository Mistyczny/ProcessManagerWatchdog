#pragma once
#include "MongoModulesCollection.hpp"
#include "MongoServicesCollection.hpp"
#include "WatchdogAcceptor.hpp"
#include <boost/asio.hpp>
#include <thread>
#include <vector>

namespace Watchdog {

struct StartingState {
    mutable std::mutex stateLock;
    std::condition_variable stateCondition;
    bool state;
};

struct AsioThreadsState {
    mutable std::mutex lock;
    std::condition_variable condition;
    bool start;
};

class WatchdogServer {
private:
    boost::asio::io_context ioContext;
    std::vector<std::thread> extraWorkingThreads;
    std::map<std::thread::id, Mongo::ModulesCollection> modulesCollection;
    std::map<std::thread::id, Mongo::ServicesCollection> servicesCollection;
    ModulesAcceptor modulesAcceptor;
    ServicesAcceptor servicesAcceptor;
    StartingState state;
    AsioThreadsState threadsState;

    static void onSignal(int signalNum);

public:
    WatchdogServer();
    virtual ~WatchdogServer() = default;

    bool createWorkingThreads();
    void runIoContext();
    void setupSignalHandlers();
    bool startAcceptingConnections();
    void setAllConnectedToDisconnectedState();
};

} // namespace Watchdog
