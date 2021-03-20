#pragma once
#include "Communication.hpp"
#include "MongoModulesCollection.hpp"
#include "MongoServicesCollection.hpp"
#include "WatchdogConnection.hpp"
#include <boost/asio.hpp>
#include <memory>

namespace Watchdog {

class ModulesAcceptor {
private:
    boost::asio::io_context& ioContext;
    std::map<std::thread::id, Mongo::ModulesCollection>& modulesCollection;
    std::map<std::thread::id, Mongo::ServicesCollection>& servicesCollection;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor = nullptr;

public:
    ModulesAcceptor(boost::asio::io_context& ioContext, std::map<std::thread::id, Mongo::ModulesCollection>&,
                    std::map<std::thread::id, Mongo::ServicesCollection>& servicesCollection);
    virtual ~ModulesAcceptor() = default;

    void startAcceptingConnections();
    void postAccept(std::shared_ptr<ModuleConnection> newSession, const boost::system::error_code& error);
};

class ServicesAcceptor {
private:
    boost::asio::io_context& ioContext;
    std::map<std::thread::id, Mongo::ModulesCollection>& modulesCollection;
    std::map<std::thread::id, Mongo::ServicesCollection>& servicesCollection;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor{nullptr};

public:
    explicit ServicesAcceptor(boost::asio::io_context&, std::map<std::thread::id, Mongo::ModulesCollection>& modulesCollection,
                              std::map<std::thread::id, Mongo::ServicesCollection>& servicesCollection);
    virtual ~ServicesAcceptor() = default;

    void startAcceptingServices();
    void serviceAccepted(std::shared_ptr<ServiceConnection> newServiceSession, const boost::system::error_code& error);
};

} // namespace Watchdog