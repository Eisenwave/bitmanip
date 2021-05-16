#include "test.hpp"

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

namespace bitmanip {

Test::~Test() = default;

// The reason why we have such strange code design with a pointer to a local static is that registerTests(Test*) gets
// invoked during static initialization.
// To make sure that testsPtr is the first thing to be initialized, we have initialize it locally during the first
// invocation of registerTest().
static std::vector<std::unique_ptr<Test>> *testsPtr;
static bool anyTestsRegistered = false;

static std::unordered_map<std::string, unsigned> testPriorityMap;

unsigned getPriority(const std::string &prefix)
{
    auto location = testPriorityMap.find(prefix);
    BITMANIP_ASSERT_MSG(location != testPriorityMap.end(), "Priority for category " + prefix + " is unspecified");
    return location->second;
}

void detail::registerTest(Test *test)
{
    static std::vector<std::unique_ptr<Test>> tests{};
    [[maybe_unused]] static const int reg_ = (testsPtr = &tests, anyTestsRegistered = true, 0);

    tests.push_back(std::unique_ptr<Test>{test});
}

void forEachTest(TestConsumer *action)
{
    if (not anyTestsRegistered) {
        return;
    }

    std::sort(testsPtr->begin(), testsPtr->end(), [](const auto &l, const auto &r) -> bool {
        unsigned lp = getPriority(l->category);
        unsigned rp = getPriority(r->category);
        return lp < rp;
    });

    for (const auto &testPtr : *testsPtr) {
        action(*testPtr);
    }
}

void setTestOrder(const char *const prefixes[], std::size_t count)
{
    BITMANIP_ASSERT_MSG(testPriorityMap.empty(), "Test order can only be specified once");
    for (unsigned i = 0; i < count; ++i) {
        testPriorityMap.emplace(prefixes[i], i);
    }
}

std::size_t getTestCount()
{
    return anyTestsRegistered ? testsPtr->size() : 0;
}

}  // namespace bitmanip
