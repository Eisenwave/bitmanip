#ifndef BITMANIP_ASSERT_HPP
#define BITMANIP_ASSERT_HPP

#include "bitmanip/build.hpp"
#include "bitmanip/builtin.hpp"

#include <string>

namespace bitmanip {

namespace detail {

template <typename From, typename To>
auto isStaticCastable_impl(int) -> decltype(static_cast<To>(std::declval<From>()));

template <typename From, typename To>
auto isStaticCastable_impl(...) -> void;

template <typename From, typename To>
constexpr bool isStaticCastable = not std::is_same_v<decltype(isStaticCastable_impl<From, To>(0)), void>;

template <typename T>
auto stringify_impl(const T &t)
{
    if constexpr (std::is_same_v<bool, T>) {
        return std::string{t ? "true" : "false"};
    }
    else if constexpr (std::is_same_v<T, std::nullptr_t>) {
        return std::string{"nullptr"};
    }
    else if constexpr (std::is_integral_v<T>) {
        return std::to_string(t);
    }
    else if constexpr (std::is_floating_point_v<T>) {
        return std::to_string(t);
    }
    else if constexpr (std::is_enum_v<T>) {
        return std::to_string(static_cast<std::underlying_type_t<T>>(t));
    }
    else if constexpr (std::is_same_v<std::string, T>) {
        return t;
    }
    else if constexpr (std::is_same_v<std::string_view, T>) {
        return std::string{t};
    }
    else if constexpr (std::is_same_v<const char *, const T>) {
        return std::string{t};
    }
    else if constexpr (std::is_pointer_v<T>) {
        return std::to_string(reinterpret_cast<std::uintptr_t>(t));
    }
    else if constexpr (detail::isStaticCastable<T, std::string>) {
        return static_cast<std::string>(t);
    }
}

template <typename T, std::size_t N>
auto stringify_impl(const T (&arr)[N])
{
    std::string result = "{" + stringify_impl(arr[0]);
    result.reserve((result.length() + 1) * N + 1);

    for (std::size_t i = 1; i < N; ++i) {
        result += ", ";
        result += stringify_impl(arr[i]);
    }

    return result + '}';
}

}  // namespace detail

template <typename T>
constexpr bool isStringifieable = not std::is_same_v<void, decltype(detail::stringify_impl(std::declval<T>()))>;

/**
 * @brief Stringifies any type. If the type is not stringifieable, substitution fails.
 * - bools are stringified as "true" or "false"
 * - std::nullptr_t is stringified as "nullptr"
 * - integers are stringified in decimal
 * - floats are stringified in decimal
 * - enums are stringified like their underlying type
 * - const char*, std::string and std::string_view are returned directly
 * - anything explicitly/implicitly convertible to std::string is converted
 * - pointer types are stringified in hexadecimal
 * - everything else is stringified using stream operator
 * @tparam the type
 * @param the value to stringify
 */
template <typename T>
auto stringify(const T &t) noexcept -> std::enable_if_t<isStringifieable<T>, std::string>
{
    return detail::stringify_impl(t);
}

enum class LogLevel : unsigned {
    /** No messages get logged at this level. Use this as a log level to completely disable logging. */
    NONE,
    /** Signals an imminent program failure. (failed assertion, thread crash, hardware errors) */
    FAILURE,
    /** Signals that an error has occured but the program can continue (failed HTTP connection, I/O eror, etc.) */
    ERROR,
    /** Warns the user about unusual behavior. (could not clear some data, had to retry some operation, etc.) */
    WARNING,
    /** Important messages, more relevant than regular info messages. */
    IMPORTANT,
    /** Default level on release builds. Used for general messages. */
    INFO,
    /** Default level on debug builds. Used for messages that are only relevant to the developer. */
    DEBUG
};

constexpr const char *nameOf(LogLevel level) noexcept
{
    switch (level) {
    case LogLevel::NONE: return "NONE";
    case LogLevel::FAILURE: return "FAILURE";
    case LogLevel::ERROR: return "ERROR";
    case LogLevel::WARNING: return "WARNING";
    case LogLevel::IMPORTANT: return "IMPORTANT";
    case LogLevel::INFO: return "INFO";
    case LogLevel::DEBUG: return "DEBUG";
    }
    BITMANIP_UNREACHABLE();
}

constexpr const char *fixedWidthNameOf(LogLevel level) noexcept
{
    switch (level) {
    case LogLevel::NONE: return "NONE";
    case LogLevel::FAILURE: return "FAIL";
    case LogLevel::ERROR: return "EROR";
    case LogLevel::WARNING: return "WARN";
    case LogLevel::IMPORTANT: return "IMPO";
    case LogLevel::INFO: return "INFO";
    case LogLevel::DEBUG: return "DBUG";
    }
    BITMANIP_UNREACHABLE();
}

constexpr bool operator<=(LogLevel x, LogLevel y) noexcept
{
    return static_cast<unsigned>(x) <= static_cast<unsigned>(y);
}

constexpr bool operator<(LogLevel x, LogLevel y) noexcept
{
    return static_cast<unsigned>(x) < static_cast<unsigned>(y);
}

constexpr bool operator>=(LogLevel x, LogLevel y) noexcept
{
    return static_cast<unsigned>(x) >= static_cast<unsigned>(y);
}

constexpr bool operator>(LogLevel x, LogLevel y) noexcept
{
    return static_cast<unsigned>(x) > static_cast<unsigned>(y);
}

// CONFIGURATION =======================================================================================================

/// The function pointer type of the logging callback.
using LogCallback = void (*)(std::string_view);
/// The function pointer type of the logging formatter.
using LogFormatter = void (*)(LogLevel, SourceLocation, const std::string_view[], std::size_t);
/// The function pointer type of the logging flusher.
using LogFlusher = void (*)();

namespace detail {

extern LogLevel logLevel;
extern LogCallback logBackend;
extern LogFormatter logFormatter;
extern LogFlusher logFlusher;

extern bool isTimestampLogging;
extern bool isLevelLogging;
extern bool isSourceLogging;

}  // namespace detail

/**
 * @brief Sets the logging backend callback for voxelio.
 * By default, voxelio logs to std::cout.
 * This behavior can be changed using this function.
 *
 * If the async flag is set, voxelio will automatically wrap the callback in a mutex-guarded logging function.
 * This way the callback will be thread-safe, even if it doesn't contain any locking code itself.
 *
 * @param callback the callback or nullptr if the behavior should be reset to default
 * @param async true if voxelio should automatically ensure thread-safety using a mutex
 */
void setLogBackend(LogCallback callback) noexcept;

/**
 * @brief Sets the logging formatter to the given function or resets it to default if the pointer is nullptr.
 * The formatter should format messages and metadata and then pass it on to the backend using logRaw().
 * @param formatter the formatter
 */
void setLogFormatter(LogFormatter formatter) noexcept;

void setLogFlusher(LogFlusher flusher) noexcept;

inline LogLevel getLogLevel() noexcept
{
    return detail::logLevel;
}

inline void setLogLevel(LogLevel level) noexcept
{
    detail::logLevel = level;
}

inline bool isLoggable(LogLevel level) noexcept
{
    return level <= detail::logLevel;
}

inline void enableLoggingTimestamp(bool enable) noexcept
{
    detail::isTimestampLogging = enable;
}

inline void enableLoggingLevel(bool enable) noexcept
{
    detail::isLevelLogging = enable;
}

inline void enableLoggingSourceLocation(bool enable) noexcept
{
    detail::isSourceLogging = enable;
}

// LOGGING FUNCTIONS ===================================================================================================

/// Directly invokes the logging callback.
inline void logRaw(std::string_view str) noexcept
{
    detail::logBackend(str);
}

inline void log(LogLevel level, SourceLocation location, const std::string_view msg[], std::size_t parts) noexcept
{
    detail::logFormatter(level, location, msg, parts);
}

inline void flushLog() noexcept
{
    detail::logFlusher();
}

/// The default format function. Invoking this bypasses the logFormatter and goes straight to the backend.
void defaultFormat(LogLevel level, SourceLocation location, const std::string_view msg[], std::size_t parts) noexcept;

namespace detail {

template <typename T>
auto stringifyOrView(const T &arg)
{
    if constexpr (isStaticCastable<T, std::string_view>) {
        return static_cast<std::string_view>(arg);
    }
    else {
        return stringify(arg);
    }
}

template <typename... Args, std::enable_if_t<(isStringifieable<Args> && ...), int> = 0>
void vlog(LogLevel level, SourceLocation loc, const Args &... args)
{
    constexpr std::size_t argCount = sizeof...(Args);

    if constexpr (not(isStaticCastable<Args, std::string_view> && ...)) {
        vlog(level, loc, stringifyOrView(args)...);
    }
    else {
        std::string_view msg[argCount]{static_cast<std::string_view>(args)...};
        log(level, loc, msg, argCount);
    }
}

}  // namespace detail

#ifdef BITMANIP_DEBUG

// clang-format off
#define BITMANIP_LOG_SPAM_GUARD_BEGIN(level) do { \
    if constexpr (::bitmanip::LogLevel::level < ::bitmanip::LogLevel::SPAM) {

#define BITMANIP_LOG_SPAM_GUARD_END ; } } while (false)
// clang-format on

#else
#define BITMANIP_LOG_SPAM_GUARD_BEGIN(level) (
#define BITMANIP_LOG_SPAM_GUARD_END )
#endif

#define BITMANIP_LOG_IMPL(level, file, function, line, ...)                                                \
    BITMANIP_LOG_SPAM_GUARD_BEGIN(level)                                                                   \
    ::bitmanip::isLoggable(::bitmanip::LogLevel::level)                                                    \
        ? ::bitmanip::detail::vlog(::bitmanip::LogLevel::level, {(file), (function), (line)}, __VA_ARGS__) \
        : void(0) BITMANIP_LOG_SPAM_GUARD_END

#define BITMANIP_LOG(level, ...) BITMANIP_LOG_IMPL(level, __FILE__, __func__, __LINE__, __VA_ARGS__)

// ASSERTIONS ==========================================================================================================

[[noreturn]] inline void assertFail(const std::string_view msg, SourceLocation loc) noexcept(false)
{
    BITMANIP_LOG_IMPL(FAILURE, loc.file, loc.function, loc.line, "assertion error in ", loc.function, "(): ", msg);
    flushLog();
    throw 0;
}

namespace detail {

enum class Cmp { EQ, NE, GT, LT, GE, LE, _ };

constexpr bool eq(Cmp a, Cmp b)
{
    return static_cast<int>(a) == static_cast<int>(b);
}
constexpr Cmp operator==(Cmp, Cmp)
{
    return Cmp::EQ;
}
constexpr Cmp operator!=(Cmp, Cmp)
{
    return Cmp::NE;
}
constexpr Cmp operator>(Cmp, Cmp)
{
    return Cmp::GT;
}
constexpr Cmp operator<(Cmp, Cmp)
{
    return Cmp::LT;
}
constexpr Cmp operator>=(Cmp, Cmp)
{
    return Cmp::GE;
}
constexpr Cmp operator<=(Cmp, Cmp)
{
    return Cmp::LE;
}

template <Cmp Cmp, typename L, typename R>
constexpr bool cmp(const L &l, const R &r) noexcept
{
    if constexpr (eq(Cmp, Cmp::EQ))
        return l == r;
    else if constexpr (eq(Cmp, Cmp::NE))
        return l != r;
    else if constexpr (eq(Cmp, Cmp::GT))
        return l > r;
    else if constexpr (eq(Cmp, Cmp::LT))
        return l < r;
    else if constexpr (eq(Cmp, Cmp::GE))
        return l >= r;
    else if constexpr (eq(Cmp, Cmp::LE))
        return l <= r;
}

template <Cmp Cmp, typename L, typename R, std::size_t N>
constexpr bool cmp(const L (&l)[N], const R (&r)[N]) noexcept
{
    for (std::size_t i = 0; i < N; ++i) {
        if (not cmp<Cmp>(l[i], r[i])) {
            return false;
        }
    }
    return true;
}

}  // namespace detail

// The following definitions using ternary operators and the do ... while(false) pattern circumvent possible syntax
// breaks in if-else chains.
//
// For example,
//     if (...) ASSERT(...); else ...;
//
// ... would break if ASSERT was using an if-statement or regular code-block, because the following semicolon is then an
// additional statement, disconnecting the else-statement.
#define BITMANIP_ASSERT_IMPL(expr, msg) \
    (static_cast<bool>(expr) ? void(0) : ::bitmanip::assertFail(msg, {__FILE__, __func__, __LINE__}))

#define BITMANIP_ASSERT_CMP(l, r, op)                                                                              \
    do {                                                                                                           \
        auto &&tl_ = (l);                                                                                          \
        auto &&tr_ = (r);                                                                                          \
        constexpr auto cmp_ = ::bitmanip::detail::Cmp::_ op ::bitmanip::detail::Cmp::_;                            \
        BITMANIP_ASSERT_IMPL(::bitmanip::detail::cmp<cmp_>(tl_, tr_),                                              \
                             "Comparison failed: " #l " " #op " " #r " (with \"" #l "\"=" +                        \
                                 ::bitmanip::stringify(tl_) + ", \"" #r "\"=" + ::bitmanip::stringify(tr_) + ")"); \
    } while (false)

#define BITMANIP_ASSERT_CONSEQUENCE(l, r)                                                                            \
    do {                                                                                                             \
        auto &&tl_ = (l);                                                                                            \
        auto &&tr_ = (r);                                                                                            \
        BITMANIP_ASSERT_IMPL(!tl_ || tr_,                                                                            \
                             "Consequence failed: " #l " => " #r " (with \"" #l "\"=" + ::bitmanip::stringify(tl_) + \
                                 ", \"" #r "\"=" + ::bitmanip::stringify(tr_) + ")");                                \
    } while (false)

#define BITMANIP_ASSERT_DIVISIBLE(l, r)                                                                              \
    do {                                                                                                             \
        auto &&tl_ = (l);                                                                                            \
        auto &&tr_ = (r);                                                                                            \
        BITMANIP_ASSERT_IMPL(tl_ % tr_ == 0,                                                                         \
                             "Divisibility failed: " #l " / " #r " (with \"" #l "\"=" + ::bitmanip::stringify(tl_) + \
                                 ", \"" #r "\"=" + ::bitmanip::stringify(tr_) + ")");                                \
    } while (false)

// COMMON ASSERTS (ALWAYS DEFINED THE SAME) ============================================================================

#define BITMANIP_ASSERT(...) BITMANIP_ASSERT_IMPL((__VA_ARGS__), "\"" #__VA_ARGS__ "\" evaluated to false")

#define BITMANIP_ASSERT_MSG(expr, msg) BITMANIP_ASSERT_IMPL(expr, '"' + ::std::string{msg} + '"')
#define BITMANIP_ASSERT_FAIL(...) BITMANIP_ASSERT_MSG(false, __VA_ARGS__)
#define BITMANIP_ASSERT_UNREACHABLE() BITMANIP_ASSERT_FAIL("This execution path must be unreachable")

#define BITMANIP_ASSERT_NOTNULL(...) BITMANIP_ASSERT_IMPL((__VA_ARGS__) != nullptr, #__VA_ARGS__ " must never be null")
#define BITMANIP_ASSERT_NULL(...) BITMANIP_ASSERT_IMPL((__VA_ARGS__) == nullptr, #__VA_ARGS__ " must always be null")
#define BITMANIP_ASSERT_EQ(l, r) BITMANIP_ASSERT_CMP(l, r, ==)
#define BITMANIP_ASSERT_NE(l, r) BITMANIP_ASSERT_CMP(l, r, !=)
#define BITMANIP_ASSERT_LT(l, r) BITMANIP_ASSERT_CMP(l, r, <)
#define BITMANIP_ASSERT_LE(l, r) BITMANIP_ASSERT_CMP(l, r, <=)
#define BITMANIP_ASSERT_GT(l, r) BITMANIP_ASSERT_CMP(l, r, >)
#define BITMANIP_ASSERT_GE(l, r) BITMANIP_ASSERT_CMP(l, r, >=)

// =====================================================================================================================

}  // namespace bitmanip

#endif  // ASSERT_HPP
