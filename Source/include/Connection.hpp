#pragma once
#include "Communication.hpp"
#include "Logging.hpp"
#include "MessageQueue.hpp"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
#include <memory>

namespace Connection {

template <typename T> class TcpConnection : public std::enable_shared_from_this<TcpConnection<T>> {
protected:
    // Input/Output object, shared
    boost::asio::io_context& ioContext;
    // Connection socket
    std::unique_ptr<boost::asio::ip::tcp::socket> socket = nullptr;
    // Queue of messages to send
    MessageQueue<Communication::Message<T>> messagesQueue;
    // Buffer message into which incoming messages will be written
    std::unique_ptr<Communication::Message<T>> incomingMessage = nullptr;
    // Wait for pings from client
    boost::asio::deadline_timer timer;
    // Last ping timestamp
    boost::posix_time::ptime last_ping;
    // Verify if there is already thread sending message's of this client
    std::atomic<bool> sendingInProgress = false;

    void readMessageHeader() {
        Log::trace("TcpConnection::readMessageHeader start");
        boost::asio::async_read(*this->socket, boost::asio::buffer(&this->incomingMessage->header, sizeof(Communication::MessageHeader<T>)),
                                boost::bind(&TcpConnection::postReadMessageHeader, this->shared_from_this(), boost::placeholders::_1,
                                            boost::asio::placeholders::bytes_transferred));
    }

    void postReadMessageHeader(const boost::system::error_code& error, size_t bytes_transferred) {
        if (error) {
            Log::debug("TcpConnection::postReadMessageHeader: " + error.message());
            this->disconnect();
        } else {
            if (bytes_transferred != sizeof(Communication::MessageHeader<T>)) {
                Log::error("TcpConnection::postReadMessageHeader bytes read: " + std::to_string(bytes_transferred));
            } else if (this->incomingMessage->header.size <= 0) {
                Log::error("TcpConnection::postReadMessageHeader required empty body: " +
                           std::to_string(this->incomingMessage->header.size));
            } else {
                Log::info("Resize to: " + std::to_string(this->incomingMessage->header.size));
                this->incomingMessage->body.resize(this->incomingMessage->header.size);
                this->readMessageBody();
            }
        }
    }

    void readMessageBody() {
        Log::trace("TcpConnection::readMessageBody start");
        // Start writing message from sending header asynchronously
        // Once message writing is done, completion handler will be called;
        boost::asio::async_read(*this->socket, boost::asio::buffer(this->incomingMessage->body.data(), this->incomingMessage->body.size()),
                                boost::bind(&TcpConnection::postReadMessageBody, this->shared_from_this(), boost::placeholders::_1,
                                            boost::asio::placeholders::bytes_transferred));
    }

    void postReadMessageBody(const boost::system::error_code& error, size_t bytes_transferred) {
        if (error) {
            Log::debug("TcpConnection::postReadMessageHeader: " + error.message());
            this->disconnect();
        } else {
            // Move message to current space to handle it
            auto localMessage{std::move(this->incomingMessage)};

            // Start handling received message here
            this->handleReceivedMessage(std::move(localMessage));

            // Start reading next message
            this->startReadingSequence();
        }
    }

    void writeMessageHeader() {
        Log::trace("TcpConnection::writeMessageHeader start");
        if (socket) {
            boost::asio::async_write(*this->socket,
                                     boost::asio::buffer(&this->messagesQueue.front().header, sizeof(Communication::MessageHeader<T>)),
                                     boost::bind(&TcpConnection::postWriteMessageHeader, this->shared_from_this(),
                                                 boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        }
    }

    void blockingWriteMessageHeader() {
        Log::trace("TcpConnection::writeMessageHeader start");
        if (socket) {
            Log::trace("TcpConnection::blockingWriteMessageHeader size: " + std::to_string(this->messagesQueue.front().header.size));
            boost::asio::write(*this->socket,
                               boost::asio::buffer(&this->messagesQueue.front().header, sizeof(Communication::MessageHeader<T>)));
        }
    }

    void postWriteMessageHeader(const boost::system::error_code& error, size_t bytes_transferred) {
        if (error) {
            Log::debug("TcpConnection::postWriteMessageHeader" + error.message());
            this->disconnect();
            this->sendingInProgress = false;
        } else if (bytes_transferred != sizeof(Communication::MessageHeader<T>)) {
            Log::error("TcpConnection::postWriteMessageHeader bytes write: " + std::to_string(bytes_transferred));
            // try to send header once again because not whole header has been sent
            this->writeMessageHeader();
        } else {
            Log::debug("TcpConnection::postWriteMessageHeader try to send message body");
            // Check if there is something in message body to send
            if (this->messagesQueue.front().body.size() > 0) {
                // Start sending message body
                this->writeMessageBody();
            } else {
                Log::debug("TcpConnection::postReadMessageHeader message body is empty");
                // If message body was empty just pop it out from queue
                this->messagesQueue.pop();
                // Check if there are more messages to send, if there are keep sending
                if (!this->messagesQueue.empty()) {
                    this->writeMessageHeader();
                }
            }
        }
    }

    void writeMessageBody() {
        Log::trace("TcpConnection::writeMessageBody start");
        boost::asio::async_write(*this->socket,
                                 boost::asio::buffer(this->messagesQueue.front().body.data(), this->messagesQueue.front().body.size()),
                                 boost::bind(&TcpConnection::postWriteMessageBody, this->shared_from_this(),
                                             boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void postWriteMessageBody(const boost::system::error_code& error, size_t bytes_transferred) {
        if (error) {
            Log::debug("TcpConnection::postWriteMessageBody: " + error.message());
            this->disconnect();
            this->sendingInProgress = false;
        } else {
            // Remove from queue message which we just sent
            this->messagesQueue.pop();

            // Check if there are more messages to send, if there are keep sending
            if (!this->messagesQueue.empty()) {
                this->writeMessageHeader();
            } else {
                this->sendingInProgress = false;
            }
        }
    }

    void timerExpired() { this->onTimerExpiration(); }
    virtual void handleReceivedMessage(std::unique_ptr<Communication::Message<T>> receivedMessage) = 0;
    virtual void onTimerExpiration() = 0;

public:
    explicit TcpConnection(boost::asio::io_context& ioContext) : ioContext{ioContext}, timer{ioContext} {
        socket = std::make_unique<boost::asio::ip::tcp::socket>(ioContext);
        Log::debug("TcpConnection::TcpConnection created");
    }
    virtual ~TcpConnection() { Log::debug("TcpConnection::TcpConnection dead"); }
    [[nodiscard]] constexpr boost::asio::ip::tcp::socket& getSocket() { return *this->socket; }

    bool isConnected() { return this->socket->is_open(); }

    void startReading() {
        last_ping = boost::posix_time::microsec_clock::local_time();
        this->startReadingSequence();
    }

    void startReadingSequence() {
        // Make sure of that there are no existing message buffer
        if (this->incomingMessage) {
            Log::debug("TcpConnection::startReadingSequence message buffer was not released before retrying");
            incomingMessage.reset();
        }
        Log::trace("Allocating new message");
        try {
            incomingMessage = std::make_unique<Communication::Message<T>>();
        } catch (std::exception& ex) {
            Log::trace("Allocating new message ex: " + std::string(ex.what()));
        }

        this->readMessageHeader();
    }

    virtual void disconnect() {
        if (this->socket->is_open()) {
            this->socket->close();
        }
    }

    virtual void closeSocket() {
        if (this->socket->is_open()) {
            this->socket->close();
        }
    }

    void sendMessage(Communication::Message<T>& message) {
        auto messagesInQueue = this->messagesQueue.push(message);
        if (!this->socket) {
            Log::error("TcpConnection::sendMessage socket was nullptr");
        } else if (!this->socket->is_open()) {
            Log::debug("TcpConnection::sendMessage connection is not open");
        } else if (sendingInProgress) {
            Log::debug("TcpConnection::sendMessage there is already sending thread running");
        } else {
            this->sendingInProgress = true;
            this->writeMessageHeader();
        }
    }

    bool connect(boost::asio::ip::tcp::endpoint& endpoint) {
        bool connected{true};
        if (this->socket) {
            try {
                this->socket->connect(endpoint);
            } catch (std::exception& ex) {
                Log::critical("TcpConnection::connect failed to connect with err: " + std::string(ex.what()));
                connected = false;
            }
        } else {
            connected = false;
        }
        return connected;
    }

    void setTimerExpiration(size_t microsec) {
        this->timer.expires_from_now(boost::posix_time::millisec(microsec));
        this->timer.async_wait(boost::bind(&TcpConnection::timerExpired, this->shared_from_this()));
    }

    void makeNewSocket() {
        socket.reset();
        socket = std::make_unique<boost::asio::ip::tcp::socket>(ioContext);
    }
};

} // namespace Connection