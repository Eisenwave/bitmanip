//#define BITMANIP_DISABLE_STATIC_TESTS

#include "test.hpp"

namespace bitmanip {
namespace {

// TEST EXECUTION ======================================================================================================

int testFailureCount = 0;

constexpr const char *TEST_ORDER[]{"traits", "bit", "bitcount", "bitileave", "bitrev", "bitrot", "intdiv", "intlog"};

void runTest(const Test &test) noexcept
{
    static thread_local const char *previousCategory = "";

    if (std::strcmp(previousCategory, test.category) != 0) {
        previousCategory = test.category;
        BITMANIP_LOG(IMPORTANT, "Category: \"" + std::string{test.category} + "\" tests");
    }
    BITMANIP_LOG(INFO, "Running \"" + std::string{test.name} + "\" ...");
    testFailureCount += not test.operator()();
}

int runTests() noexcept
{
    BITMANIP_LOG(INFO, "Running " + stringify(getTestCount()) + " tests ...");

    setTestOrder(TEST_ORDER, std::size(TEST_ORDER));
    forEachTest(&runTest);

    if (testFailureCount == 0) {
        BITMANIP_LOG(IMPORTANT, "All " + stringify(getTestCount()) + " tests passed");
    }
    else {
        BITMANIP_LOG(ERROR,
                     "Some tests failed (" + stringify(testFailureCount) + "/" + stringify(getTestCount()) + ") total");
    }
    return testFailureCount;
}

}  // namespace
}  // namespace bitmanip

int main()
{
    bitmanip::setLogLevel(bitmanip::LogLevel::DEBUG);
    bitmanip::enableLoggingSourceLocation(false);
    bitmanip::enableLoggingTimestamp(false);

    return bitmanip::runTests();
}
