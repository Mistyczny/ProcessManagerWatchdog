#include "WatchdogServer.hpp"
#include "Logging.hpp"
#include "MongoDbEnvironment.hpp"
#include <csignal>
#include <exception>
#include <iostream>

namespace Watchdog {

WatchdogServer::WatchdogServer() : modulesAcceptor{ioContext, modulesCollection}, servicesAcceptor{ioContext, servicesCollection} {
    threadsState.start = false;
}

bool WatchdogServer::createWorkingThreads() {
    bool created{true};
    try {
        for (auto threadNr = 0; threadNr < 3; threadNr++) {
            extraWorkingThreads.emplace_back([&]() { ioContext.run(); });
        }
        std::for_each(std::begin(extraWorkingThreads), std::end(extraWorkingThreads), [&](auto& thread) {
            std::thread::id this_id = thread.get_id();
            auto modulesCollectionEntry = Mongo::DbEnvironment::getInstance()->getClient();
            Mongo::ModulesCollection modulesCollection{*modulesCollectionEntry, "Modules"};
            this->modulesCollection.insert({this_id, std::move(modulesCollection)});

            auto servicesCollectionEntry = Mongo::DbEnvironment::getInstance()->getClient();
            Mongo::ServicesCollection services{*servicesCollectionEntry, "Services"};
            this->servicesCollection.insert({this_id, std::move(services)});
        });

    } catch (std::exception& ex) {
        created = false;
    }
    return created;
}

void WatchdogServer::runIoContext() {
    Log::debug("WatchdogServer::runIoContext connection threads joining");
    std::for_each(std::begin(extraWorkingThreads), std::end(extraWorkingThreads), std::mem_fn(&std::thread::join));
}

void WatchdogServer::setupSignalHandlers() {
    signal(SIGINT, WatchdogServer::onSignal);
    signal(SIGABRT, WatchdogServer::onSignal);
    signal(SIGSEGV, WatchdogServer::onSignal);
    signal(SIGTERM, WatchdogServer::onSignal);
}

void WatchdogServer::onSignal(int signalNum) { exit(signalNum); }

bool WatchdogServer::startAcceptingConnections() {
    bool acceptingConnections{true};
    try {
        Log::critical("WatchdogServer::startAcceptingConnections modules acceptor start");
        this->modulesAcceptor.startAcceptingConnections();
        this->servicesAcceptor.startAcceptingServices();
    } catch (std::exception& ex) {
        Log::critical("WatchdogServer::startAcceptingConnections modules acceptor start failure");
        acceptingConnections = false;
    }
    return acceptingConnections;
}

} // namespace Watchdog