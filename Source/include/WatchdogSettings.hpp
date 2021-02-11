#pragma once
#include "Logging.hpp"
#include <string>
#include <vector>

namespace Watchdog {

struct EnvironmentConfiguration {
    std::vector<std::string> pathsToSearchModules;
};

class WatchdogSettings {
private:
    uint32_t maxNumberOfModules;
    // Logging::LogLevel loggingLevel;
public:
    WatchdogSettings();
    ~WatchdogSettings();
};

class WatchdogSettingsReader {
private:
public:
    WatchdogSettingsReader(WatchdogSettings&);
    WatchdogSettingsReader(const WatchdogSettingsReader&) = delete;
    WatchdogSettingsReader(WatchdogSettingsReader&&) = delete;
    virtual ~WatchdogSettingsReader() = default;
};

class WatchdogSettingsWriter {
private:
public:
    WatchdogSettingsWriter(WatchdogSettings&);
    WatchdogSettingsWriter(const WatchdogSettingsWriter&) = delete;
    WatchdogSettingsWriter(WatchdogSettingsWriter&&) = delete;
    virtual ~WatchdogSettingsWriter() = default;
};

} // namespace Watchdog