#include "assert.hpp"

#include <ctime>
#include <iostream>
#include <mutex>

#include <tuple>

namespace bitmanip {

namespace {
namespace ansi {

#define ASCII_ESC "\x1b"

constexpr const char *RESET = ASCII_ESC "[0m";

// constexpr const char *FG_16C_BLK = ASCII_ESC "[38;5;0m";
// constexpr const char *FG_16C_RED = ASCII_ESC "[38;5;1m";
constexpr const char *FG_16C_GRN = ASCII_ESC "[38;5;2m";
constexpr const char *FG_16C_ORG = ASCII_ESC "[38;5;3m";
// constexpr const char *FG_16C_BLU = ASCII_ESC "[38;5;4m";
// constexpr const char *FG_16C_MAG = ASCII_ESC "[38;5;5m";
// constexpr const char *FG_16C_CYA = ASCII_ESC "[38;5;6m";
constexpr const char *FG_16C_BRI_GRA = ASCII_ESC "[38;5;7m";
// constexpr const char *FG_16C_GRA = ASCII_ESC "[38;5;8m";
constexpr const char *FG_16C_BRI_RED = ASCII_ESC "[38;5;9m";
// constexpr const char *FG_16C_BRI_GRN = ASCII_ESC "[38;5;10m";
constexpr const char *FG_16C_YLW = ASCII_ESC "[38;5;11m";
constexpr const char *FG_16C_BRI_BLU = ASCII_ESC "[38;5;12m";
constexpr const char *FG_16C_BRI_MAG = ASCII_ESC "[38;5;13m";
// constexpr const char *FG_16C_BRI_CYA = ASCII_ESC "[38;5;14m";
// constexpr const char *FG_16C_WHT = ASCII_ESC "[38;5;15m";

}  // namespace ansi

// constexpr const char* ISO8601_DATETIME = "%Y-%m-%d %H:%M:%S";
constexpr const char *ISO8601_TIME = "%H:%M:%S";

std::time_t time() noexcept
{
    std::time_t rawTime;
    BITMANIP_ASSERT_MSG(std::time(&rawTime) != -1, "Failed to get system time");
    return rawTime;
}

std::tm *localtime(std::time_t time) noexcept
{
    std::tm *timeInfo = std::localtime(&time);
    BITMANIP_ASSERT_NOTNULL(timeInfo);
    return timeInfo;
}

std::string currentIso8601Time() noexcept
{
    constexpr size_t resultLength = 8;

    std::tm *timeInfo = localtime(time());
    char buffer[resultLength + 1];
    std::strftime(buffer, sizeof(buffer), ISO8601_TIME, timeInfo);

    return {buffer, resultLength};
}

constexpr const char *prefixOf(LogLevel level) noexcept
{
    switch (level) {
    case LogLevel::NONE: return ansi::FG_16C_ORG;
    case LogLevel::FAILURE: return ansi::FG_16C_BRI_RED;
    case LogLevel::ERROR: return ansi::FG_16C_BRI_RED;
    case LogLevel::WARNING: return ansi::FG_16C_YLW;
    case LogLevel::IMPORTANT: return ansi::FG_16C_GRN;
    case LogLevel::INFO: return ansi::FG_16C_BRI_BLU;
    case LogLevel::DEBUG: return ansi::FG_16C_BRI_MAG;
    }
    BITMANIP_ASSERT_UNREACHABLE();
}

void logToCout(std::string_view msg) noexcept
{
    std::cout << msg;
}

void flushCout() noexcept
{
    std::cout.flush();
}

void doNothing() {}

[[maybe_unused]] std::string substrBeforeFirst(const std::string &str, char delimiter)
{
    const auto pos = str.find_first_of(delimiter);
    return pos == std::string::npos ? str : str.substr(0, pos);
}

[[maybe_unused]] std::string substrBeforeLast(const std::string &str, char delimiter)
{
    const auto pos = str.find_last_of(delimiter);
    return pos == std::string::npos ? str : str.substr(0, pos);
}

[[maybe_unused]] std::string substrAfterFirst(const std::string &str, char delimiter)
{
    const auto pos = str.find_first_of(delimiter);
    return pos == std::string::npos ? str : str.substr(pos + 1, str.size());
}

[[maybe_unused]] std::string substrAfterLast(const std::string &str, char delimiter)
{
    const auto pos = str.find_last_of(delimiter);
    return pos == std::string::npos ? str : str.substr(pos + 1, str.size());
}

}  // namespace

LogLevel detail::logLevel = LogLevel::INFO;
LogCallback detail::logBackend = &logToCout;
LogFormatter detail::logFormatter = &defaultFormat;
LogFlusher detail::logFlusher = &flushCout;

bool detail::isTimestampLogging = true;
bool detail::isLevelLogging = true;
bool detail::isSourceLogging = true;

void defaultFormat(LogLevel level, SourceLocation location, const std::string_view msg[], std::size_t parts) noexcept
{
#ifdef BITMANIP_UNIX
#define BITMANIP_IF_UNIX(code) code
#define BITMANIP_IF_WINDOWS(code)
#else
#define BITMANIP_IF_UNIX(code)
#define BITMANIP_IF_WINDOWS(code) code
#endif

    std::string prefix;

    if (detail::isTimestampLogging) {
        prefix += "[";
        prefix += currentIso8601Time();
        prefix += "] [";
    }
    else {
        prefix += '[';
    }
    BITMANIP_IF_UNIX(prefix += prefixOf(level));
    prefix += fixedWidthNameOf(level);
    BITMANIP_IF_UNIX(prefix += ansi::RESET);
    prefix += "] ";

    if (detail::isSourceLogging) {
        BITMANIP_IF_UNIX(prefix += ansi::FG_16C_BRI_GRA);
        prefix += substrAfterLast(location.file, BITMANIP_IF_WINDOWS('\\') BITMANIP_IF_UNIX('/'));
        prefix += '@';
        prefix += bitmanip::stringify(location.line);
        prefix += ": ";
        BITMANIP_IF_UNIX(prefix += ansi::RESET);
    }

    logRaw(prefix);
    for (std::size_t i = 0; i < parts; ++i) {
        logRaw(msg[i]);
    }
    logRaw("\n");
}

void setLogBackend(LogCallback callback) noexcept
{
    if (callback == nullptr) {
        callback = &logToCout;
    }
    detail::logBackend = callback;
    detail::logFlusher = &doNothing;
}

void setLogFormatter(LogFormatter callback) noexcept
{
    detail::logFormatter = callback == nullptr ? &defaultFormat : callback;
}

void setLogFlusher(LogFlusher flusher) noexcept
{
    detail::logFlusher = flusher;
}

}  // namespace bitmanip
