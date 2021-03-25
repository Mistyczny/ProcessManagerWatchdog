#include "WatchdogAcceptor.hpp"
#include "Connection.hpp"
#include "Logging.hpp"
#include "WatchdogConnection.hpp"
#include <memory>

namespace Watchdog {

ModulesAcceptor::ModulesAcceptor(boost::asio::io_context& ioContext, std::map<std::thread::id, Mongo::ModulesCollection>& modulesCollection,
                                 std::map<std::thread::id, Mongo::ServicesCollection>& servicesCollection)
    : ioContext{ioContext}, modulesCollection{modulesCollection}, servicesCollection{servicesCollection} {
    try {
        boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::tcp::v4(), 1234};
        acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(ioContext, endpoint);
    } catch (boost::system::system_error& err) {
        Log::critical(std::string("Failed during creating acceptor: " + std::string(err.what())));
    }
}

void ModulesAcceptor::startAcceptingConnections() {
    if (acceptor) {
        auto newSession = std::make_shared<ModuleConnection>(ioContext, modulesCollection, servicesCollection);
        acceptor->async_accept(newSession->getSocket(),
                               boost::bind(&ModulesAcceptor::postAccept, this, newSession, boost::asio::placeholders::error));
    } else {
        throw std::runtime_error("Module acceptor was not created");
    }
}

void ModulesAcceptor::postAccept(std::shared_ptr<ModuleConnection> newSession, const boost::system::error_code& error) {
    if (error) {
        Log::critical(std::string("Failure during accepting connection"));
        return;
    }
    Log::info("Module accepted");
    newSession->setTimerWaitForConnection();
    newSession->startReading();
    this->startAcceptingConnections();
}

ServicesAcceptor::ServicesAcceptor(boost::asio::io_context& ioContext,
                                   std::map<std::thread::id, Mongo::ModulesCollection>& modulesCollection,
                                   std::map<std::thread::id, Mongo::ServicesCollection>& servicesCollection)
    : ioContext{ioContext}, servicesCollection{servicesCollection}, modulesCollection{modulesCollection} {
    try {
        boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::tcp::v4(), 1235};
        acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(ioContext, endpoint);
    } catch (boost::system::system_error& err) {
        Log::critical(std::string("Failed during creating acceptor: " + std::string(err.what())));
    }
}

void ServicesAcceptor::startAcceptingServices() {
    if (acceptor) {
        auto newSession = std::make_shared<ServiceConnection>(ioContext, modulesCollection, servicesCollection);
        acceptor->async_accept(newSession->getSocket(),
                               boost::bind(&ServicesAcceptor::serviceAccepted, this, newSession, boost::asio::placeholders::error));
    } else {
        throw std::runtime_error("Module acceptor was not created");
    }
}

void ServicesAcceptor::serviceAccepted(std::shared_ptr<ServiceConnection> newServiceSession, const boost::system::error_code& error) {
    if (error) {
        Log::critical(std::string("Failure during accepting connection"));
        return;
    }
    Log::info("Service accepted");
    this->startAcceptingServices();
    newServiceSession->startReading();
}

} // namespace Watchdog
