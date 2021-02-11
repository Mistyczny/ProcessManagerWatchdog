#pragma once
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include <iostream>
#include <string>
#include <type_traits>

class Log {
private:
    static inline std::unique_ptr<Log> instance = nullptr;
    std::shared_ptr<spdlog::logger> logger = nullptr;

public:
    enum class LogLevel : uint8_t { TRACE, DEBUG, INFO, WARNING, ERROR, CRITICAL };

    template <typename T> Log(T moduleIdentifier, LogLevel logLevel) {
        logger = spdlog::basic_logger_mt("file_logger", std::string{"/home/kacper/Logs/" + std::string(moduleIdentifier) + ".log"});
        logger->set_pattern("[%H:%M:%S %z] [thread %t] %v");
        logger->set_level(spdlog::level::err);
        spdlog::flush_every(std::chrono::seconds(3));
    }

    virtual ~Log() = default;

    template <typename T> static void initialize(T moduleIdentifier, LogLevel logLevel) {
        if (!instance) {
            instance = std::make_unique<Log>(moduleIdentifier, logLevel);
        }
    }

    template <typename T> static void trace(T message) {
        static_assert(std::is_convertible<T, std::string>::value, "Message is not trivially convertible to std::string");
        if (instance) {
            instance->logger->trace(message);
        }
    }

    template <typename T> static void trace(T message, T functionName) {
        static_assert(std::is_convertible<T, std::string>::value, "Message is not trivially convertible to std::string");
        if (instance) {
            instance->logger->trace(message);
        }
    }

    template <typename T> static void debug(T message) {
        static_assert(std::is_convertible<T, std::string>::value, "Message is not trivially convertible to std::string");
        if (instance) {
            instance->logger->debug(message);
        }
    }

    template <typename T> static void info(T message) {
        static_assert(std::is_convertible<T, std::string>::value, "Message is not trivially convertible to std::string");
        if (instance) {
            instance->logger->info(message);
        }
    }

    template <typename T> static void error(T message) {
        static_assert(std::is_convertible<T, std::string>::value, "Message is not trivially convertible to std::string");
        if (instance) {
            instance->logger->error(message);
        }
    }

    template <typename T> static void critical(T message) {
        static_assert(std::is_convertible<T, std::string>::value, "Message is not trivially convertible to std::string");
        if (instance) {
            instance->logger->critical(message);
        }
    }
};
