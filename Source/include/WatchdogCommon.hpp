#pragma once
#include "Connection.hpp"
#include "WatchdogModule.pb.h"
#include <memory>
#include <sys/types.h>
#include <unistd.h>

namespace Watchdog {

struct ModuleRecord {
    pid_t pid;
    uint32_t pingTimerID;
    Types::ModuleIdentifier identifier;
    std::weak_ptr<Connection::TcpConnection<WatchdogModule::Operation>> owner;
};

enum class TimerOperationCode : uint8_t { SetTimer, TimerExpired };

constexpr size_t PingSendingIntervalInMilliseconds = 7000;
constexpr size_t PingTimerExpirationIntervalInMilliseconds = 8000;

} // namespace Watchdog