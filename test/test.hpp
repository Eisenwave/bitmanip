#ifndef BITMANIP_TEST_HPP
#define BITMANIP_TEST_HPP

#include "assert.hpp"

#include <cstring>

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

#ifndef DISABLE_STATIC_TESTS
#define BITMANIP_STATIC_ASSERT_EQ(x, y) \
    static_assert(x == y);              \
    BITMANIP_ASSERT_EQ(x, y)
#else
#define STATIC_ASSERT_EQ(x, y) TEST_ASSERT_EQ(x, y)
#endif

}  // namespace bitmanip

#endif
