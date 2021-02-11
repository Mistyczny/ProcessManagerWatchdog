#pragma once
#include "Types.hpp"
#include "WatchdogModule.pb.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

namespace Communication {

enum class RegistrationResponseCode : uint16_t { Success = 0, NotModuleIdentifier };
enum class ConnectResponseCode : uint16_t { Success = 0, NotModuleIdentifier, ModuleNotExists, InvalidConnectionState };
enum class ReconnectResponseCode : uint16_t { Success = 0, NotModuleIdentifier, ModuleNotExists, InvalidConnectionState };

template <typename T> struct MessageHeader {
    T operationCode;
    uint32_t size;
};

template <typename T> struct Message {
    MessageHeader<T> header{};
    std::string body{};
};

} // namespace Communication