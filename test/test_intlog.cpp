#include "bitmanip/intlog.hpp"

#include "test.hpp"

namespace bitmanip {
namespace {

template <std::uint32_t (*log2_floor_32)(std::uint32_t), std::uint64_t (*log2_floor_64)(std::uint64_t) = nullptr>
void test_log2_floor_forPow2()
{
    for (size_t i = 1; i < 32; ++i) {
        BITMANIP_ASSERT_EQ(log2_floor_32(std::uint32_t{1} << i), i);
    }
    if constexpr (log2_floor_64 == nullptr) {
        return;
    }
    for (size_t i = 1; i < 64; ++i) {
        BITMANIP_ASSERT_EQ(log2_floor_64(std::uint64_t{1} << i), i);
    }
}

template <std::uint32_t (*log2_floor_32)(std::uint32_t), std::uint64_t (*log2_floor_64)(std::uint64_t) = nullptr>
void test_log2_floor_general()
{
    for (size_t i = 4; i < 32; ++i) {
        std::uint32_t input = std::uint32_t{1} << i;
        BITMANIP_ASSERT_EQ(log2_floor_32(input), i);
        BITMANIP_ASSERT_EQ(log2_floor_32(input + 1), i);
        BITMANIP_ASSERT_EQ(log2_floor_32(input + 2), i);
        BITMANIP_ASSERT_EQ(log2_floor_32(input + 3), i);
    }
    if constexpr (log2_floor_64 == nullptr) {
        return;
    }
    for (size_t i = 4; i < 64; ++i) {
        std::uint64_t input = std::uint64_t{1} << i;
        BITMANIP_ASSERT_EQ(log2_floor_64(input), i);
        BITMANIP_ASSERT_EQ(log2_floor_64(input + 1), i);
        BITMANIP_ASSERT_EQ(log2_floor_64(input + 2), i);
        BITMANIP_ASSERT_EQ(log2_floor_64(input + 3), i);
    }
}

BITMANIP_TEST(intlog, log10floor_approximate_table)
{
    auto table = detail::makeGuessTable<std::uint64_t, 10>();
    BITMANIP_ASSERT_NE(detail::approximateGuessTable(table), detail::NO_APPROXIMATION);
}

BITMANIP_TEST(intlog, log2floor_naive_forPow2)
{
    test_log2_floor_forPow2<log2floor_naive<std::uint32_t>, log2floor_naive<std::uint64_t>>();
}

BITMANIP_TEST(intlog, log2floor_fast_forPow2)
{
    test_log2_floor_forPow2<log2floor_fast<std::uint32_t>, log2floor_fast<std::uint64_t>>();
}

BITMANIP_TEST(intlog, log2floor_debruijn_forPow2)
{
    test_log2_floor_forPow2<log2floor_debruijn>();
}

BITMANIP_TEST(intlog, log2floor_forPow2)
{
    test_log2_floor_forPow2<log2floor<std::uint32_t>, log2floor<std::uint64_t>>();
}

BITMANIP_TEST(intlog, log2floor_naive_general)
{
    test_log2_floor_general<log2floor_naive<std::uint32_t>, log2floor_naive<std::uint64_t>>();
}

BITMANIP_TEST(intlog, log2floor_fast_general)
{
    test_log2_floor_general<log2floor_fast<std::uint32_t>, log2floor_fast<std::uint64_t>>();
}

BITMANIP_TEST(intlog, log2floor_debruijn_general)
{
    test_log2_floor_general<log2floor_debruijn>();
}

BITMANIP_TEST(intlog, log2floor_general)
{
    test_log2_floor_general<log2floor_debruijn>();
}

BITMANIP_TEST(intlog, log2floor_manual)
{
    BITMANIP_STATIC_ASSERT_EQ(log2floor(0u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log2floor_debruijn(0u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log2floor_fast(0u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log2floor_naive(0u), 0u);
}

BITMANIP_TEST(intlog, log10floor_manual)
{
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint8_t>(0u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint8_t>(9u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint8_t>(10u), 1u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint8_t>(99u), 1u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint8_t>(100u), 2u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint8_t>(255u), 2u);

    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(0u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(1u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(2u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(3u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(4u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(5u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(6u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(7u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(8u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(9u), 0u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(10u), 1u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(11u), 1u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(12u), 1u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(13u), 1u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(14u), 1u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(15u), 1u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(16u), 1u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(99u), 1u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(100u), 2u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(999u), 2u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(1'000u), 3u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(9'999u), 3u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(10000u), 4u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(99'999u), 4u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(100'000u), 5u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(999'999u), 5u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(1'000'000u), 6u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(9'999'999u), 6u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(10'000'000u), 7u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(99'999'999u), 7u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(100'000'000u), 8u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(999'999'999u), 8u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(1'000'000'000u), 9u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(2'000'000'000u), 9u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(4'000'000'000u), 9u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint32_t>(std::uint32_t(~0)), 9u);

    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint64_t>(std::uint64_t{1} << 63), 18u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint64_t>(9'999'999'999'999'999'999ull), 18u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint64_t>(10'000'000'000'000'000'000ull), 19u);
    BITMANIP_STATIC_ASSERT_EQ(log10floor<std::uint64_t>(~std::uint64_t{0}), 19u);
}

}  // namespace
}  // namespace bitmanip
