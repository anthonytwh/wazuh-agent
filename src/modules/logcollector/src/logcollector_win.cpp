#include "event_reader_win.hpp"

#include <config.h>
#include <timeHelper.hpp>

#include <chrono>
#include <map>

#include "logcollector.hpp"

namespace logcollector
{

    void Logcollector::AddPlatformSpecificReader(
        std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
    {
        const auto refreshInterval = configurationParser->GetTimeConfigOrDefault(
            config::logcollector::DEFAULT_RELOAD_INTERVAL, "logcollector", "reload_interval");

        const std::vector<std::map<std::string, std::string>> defaultWinOsConfig {};

        auto windowsConfig = configurationParser->GetConfigOrDefault(defaultWinOsConfig, "logcollector", "windows");

        for (auto& entry : windowsConfig)
        {
            const auto channel = entry["channel"];
            const auto query = entry["query"];
            AddReader(std::make_shared<winevt::WindowsEventTracerReader>(
                [this](const std::string& location, const std::string& log, const std::string& collectorType)
                { PushMessage(location, log, collectorType); },
                // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
                [this](std::chrono::milliseconds duration) -> Awaitable { co_await Wait(duration); },
                channel,
                query,
                refreshInterval));
        }
    }

} // namespace logcollector
