#ifndef BITMANIP_TEST_HPP
#define BITMANIP_TEST_HPP

#include "assert.hpp"

#include <cstring>
#include <random>

namespace bitmanip {

struct Test {
public:
    const char *category;
    const char *name;

    explicit Test(const char *category, const char *name) : category{category}, name{name} {}
    virtual ~Test();

    bool operator()() const noexcept
    {
        try {
            run();
            return true;
        }
        catch (...) {
            return false;
        }
    }

protected:
    virtual void run() const = 0;
};

using TestConsumer = void(const Test &test);

namespace detail {

void registerTest(Test *test);

}  // namespace detail

void setTestOrder(const char *const prefixes[], std::size_t count);

void forEachTest(TestConsumer *action);

std::size_t getTestCount();

#define BITMANIP_TEST_CLASS_NAME(category, name) Test_##category##_##name##_

#define BITMANIP_TEST(category, name)                                                                       \
    struct BITMANIP_TEST_CLASS_NAME(category, name) : ::bitmanip::Test {                                    \
        [[maybe_unused]] BITMANIP_TEST_CLASS_NAME(category, name)() : ::bitmanip::Test{#category, #name} {} \
        void run() const final;                                                                             \
    };                                                                                                      \
    static const int category##_##name##_ =                                                                 \
        (::bitmanip::detail::registerTest(new BITMANIP_TEST_CLASS_NAME(category, name)), 0);                \
                                                                                                            \
    void BITMANIP_TEST_CLASS_NAME(category, name)::run() const

#ifndef BITMANIP_DISABLE_STATIC_TESTS
#define BITMANIP_STATIC_ASSERT_EQ(x, y) \
    static_assert(x == y);              \
    BITMANIP_ASSERT_EQ(x, y)
#define BITMANIP_STATIC_ASSERT(...) \
    static_assert(__VA_ARGS__);     \
    BITMANIP_ASSERT(__VA_ARGS__)
#else
#define BITMANIP_STATIC_ASSERT_EQ(x, y) BITMANIP_ASSERT_EQ(x, y)
#define BITMANIP_STATIC_ASSERT(...) BITMANIP_ASSERT(__VA_ARGS__)
#endif

// UTILITY =============================================================================================================

using fast_rng32 = std::linear_congruential_engine<uint32_t, 1664525, 1013904223, 0>;
using fast_rng64 = std::linear_congruential_engine<uint64_t, 6364136223846793005, 1442695040888963407, 0>;
using default_rng = std::mt19937;

constexpr std::uint32_t DEFAULT_SEED = 12345;

inline std::uint32_t hardwareSeed()
{
    return std::random_device{}();
}

}  // namespace bitmanip

#endif
