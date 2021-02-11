#pragma once
#include <mutex>
#include <queue>

template <typename T> class MessageQueue {
private:
    mutable std::mutex queueLock;
    std::queue<T> messagesQueue;

public:
    MessageQueue() = default;
    MessageQueue(const MessageQueue<T>&) = delete;
    virtual ~MessageQueue() = default;

    [[nodiscard]] size_t push(const T& message) {
        std::scoped_lock lock(this->queueLock);
        this->messagesQueue.push(message);
        return this->messagesQueue.size();
    }

    [[nodiscard]] size_t push(T&& message) {
        std::scoped_lock lock(this->queueLock);
        this->messagesQueue.push(std::move(message));
        return this->messagesQueue.size();
    }
    [[nodiscard]] const T& front() {
        std::scoped_lock lock(this->queueLock);
        return this->messagesQueue.front();
    }
    void pop() {
        std::scoped_lock lock(this->queueLock);
        this->messagesQueue.pop();
    }
    [[nodiscard]] size_t size() const {
        std::scoped_lock lock(this->queueLock);
        return this->messagesQueue.size();
    }
    [[nodiscard]] bool empty() const {
        std::scoped_lock lock(this->queueLock);
        return this->messagesQueue.empty();
    }
};