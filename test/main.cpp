#include "bitmanip/all.hpp"

//#define BITMANIP_DISABLE_STATIC_TESTS

#include "test.hpp"

#include <random>

namespace bitmanip {
namespace {

// UTILITY =============================================================================================================

using fast_rng32 = std::linear_congruential_engine<uint32_t, 1664525, 1013904223, 0>;
using fast_rng64 = std::linear_congruential_engine<uint64_t, 6364136223846793005, 1442695040888963407, 0>;
using default_rng = std::mt19937;

constexpr std::uint32_t DEFAULT_SEED = 12345;

inline std::uint32_t hardwareSeed()
{
    return std::random_device{}();
}

// STATIC ==============================================================================================================

template <std::uint32_t (*log2_floor_32)(std::uint32_t), std::uint64_t (*log2_floor_64)(std::uint64_t) = nullptr>
void test_log2_floor_forPow2()
{
    for (size_t i = 1; i < 32; ++i) {
        BITMANIP_ASSERT_EQ(log2_floor_32(std::uint32_t{1} << i), i);
    }
    if (log2_floor_64 == nullptr) {
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
    if (log2_floor_64 == nullptr) {
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

// TESTS ===============================================================================================================

BITMANIP_TEST(intlog, log10floor_approximate_table)
{
    auto table = detail::makeGuessTable<std::uint64_t, 10>();
    detail::approximateGuessTable(table);
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

BITMANIP_TEST(bitileave, ileaveZeros_naive_manual)
{
    BITMANIP_STATIC_ASSERT_EQ(detail::ileaveZeros_naive(0xff, 0), 0xffu);
    BITMANIP_STATIC_ASSERT_EQ(detail::ileaveZeros_naive(0xff, 1), 0b0101'0101'0101'0101u);
    BITMANIP_STATIC_ASSERT_EQ(detail::ileaveZeros_naive(0xff, 2), 0b001001001001001001001001u);
    BITMANIP_STATIC_ASSERT_EQ(detail::ileaveZeros_naive(0xffff'ffff, 1), 0x5555'5555'5555'5555u);
    BITMANIP_STATIC_ASSERT_EQ(detail::ileaveZeros_naive(0xffff'ffff, 2), 0x9249'2492'4924'9249u);
    BITMANIP_STATIC_ASSERT_EQ(detail::ileaveZeros_naive(0xffff'ffff, 3), 0x1111'1111'1111'1111u);
    BITMANIP_STATIC_ASSERT_EQ(detail::ileaveZeros_naive(0xffff'ffff, 7), 0x0101'0101'0101'0101u);
    BITMANIP_STATIC_ASSERT_EQ(detail::ileaveZeros_naive(0xffff'ffff, 15), 0x0001'0001'0001'0001u);
    BITMANIP_STATIC_ASSERT_EQ(detail::ileaveZeros_naive(0xffff'ffff, 31), 0x0000'0001'0000'0001u);

    BITMANIP_STATIC_ASSERT_EQ(ileaveZeros_const<4>(12345678), detail::ileaveZeros_naive(12345678, 4));
}

BITMANIP_TEST(bitileave, duplBits_naive_manual)
{
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(0xf, 0), 0u);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(0xf, 2), 0xffu);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(0x55, 2), 0x3333u);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(0xff, 2), 0xffffu);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(1, 1), 1u);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(1, 2), 3u);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(1, 4), 0xfu);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(1, 8), 0xffu);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(1, 16), 0xffffu);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(1, 32), 0xffffffffu);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(1, 64), 0xffffffffffffffffu);
}

BITMANIP_TEST(bitileave, ileaveBits_and_duplBits_manual)
{
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 1), 1),
                              0x5555'5555'5555'5555u);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 1), 2),
                              0x3333'3333'3333'3333u);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 1), 4),
                              0x0f0f'0f0f'0f0f'0f0fu);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 1), 8),
                              0x00ff'00ff'00ff'00ffu);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 1), 16),
                              0x0000'ffff'0000'ffffu);

    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 2), 1),
                              0x9249'2492'4924'9249u);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 2), 2),
                              0x30C3'0C30'C30C'30C3u);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 2), 4),
                              0xF00F'00F0'0F00'F00Fu);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 2), 8),
                              0x00FF'0000'FF00'00FFu);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 2), 16),
                              0xFFFF'0000'0000'FFFFu);

    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 3), 1),
                              0x1111'1111'1111'1111u);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 3), 2),
                              0x0303'0303'0303'0303u);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 3), 4),
                              0x000f'000f'000f'000fu);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 3), 8),
                              0x0000'00ff'0000'00ffu);
    BITMANIP_STATIC_ASSERT_EQ(detail::duplBits_naive(detail::ileaveZeros_naive(std::uint32_t(-1), 3), 16), 0xffffu);
}

BITMANIP_TEST(bitileave, remIleavedBits_naive_manual)
{
    BITMANIP_STATIC_ASSERT_EQ(detail::remIleavedBits_naive(0xff, 0), 0xffu);
    BITMANIP_STATIC_ASSERT_EQ(detail::remIleavedBits_naive(0xff, 1), 0xfu);
    BITMANIP_STATIC_ASSERT_EQ(detail::remIleavedBits_naive(0b01010101, 1), 0b1111u);
    BITMANIP_STATIC_ASSERT_EQ(detail::remIleavedBits_naive(0x5555'5555'5555'5555, 1), 0xffff'ffffu);
    BITMANIP_STATIC_ASSERT_EQ(detail::remIleavedBits_naive(0x1111'1111'1111'1111, 3), 0b1111'1111'1111'1111u);

    BITMANIP_STATIC_ASSERT_EQ(detail::remIleavedBits_naive(0x5555'5555'5555'5555, 0), 0x5555'5555'5555'5555u);
    BITMANIP_STATIC_ASSERT_EQ(detail::remIleavedBits_naive(0x5, 1), 3u);
    BITMANIP_STATIC_ASSERT_EQ(detail::remIleavedBits_naive(0x5555'5555'5555'5555, 1), 0xffff'ffffu);
    BITMANIP_STATIC_ASSERT_EQ(detail::remIleavedBits_naive(0xffff'ffff'ffff'ffff, 1), 0xffff'ffffu);
    BITMANIP_STATIC_ASSERT_EQ(detail::remIleavedBits_naive(0xffff'ffff'ffff'ffff, 1), 0xffff'ffffu);
    BITMANIP_STATIC_ASSERT_EQ(detail::remIleavedBits_naive(0x9249'2492'4924'9249, 2), 0x3fffffu);
}

BITMANIP_TEST(bitileave, remIleavedBits_const_manual)
{
    BITMANIP_STATIC_ASSERT_EQ(remIleavedBits_const<0>(0xff), 0xffu);
    BITMANIP_STATIC_ASSERT_EQ(remIleavedBits_const<1>(0xff), 0xfu);
    BITMANIP_STATIC_ASSERT_EQ(remIleavedBits_const<1>(0b01010101), 0b1111u);
    BITMANIP_STATIC_ASSERT_EQ(remIleavedBits_const<1>(0x5555'5555'5555'5555), 0xffff'ffffu);
    BITMANIP_STATIC_ASSERT_EQ(remIleavedBits_const<3>(0x1111'1111'1111'1111), 0b1111'1111'1111'1111u);

    BITMANIP_STATIC_ASSERT_EQ(remIleavedBits_const<4>(12345678), detail::remIleavedBits_naive(12345678, 4));
    BITMANIP_STATIC_ASSERT_EQ(remIleavedBits_const<8>(12345678), detail::remIleavedBits_naive(12345678, 8));
    BITMANIP_STATIC_ASSERT_EQ(remIleavedBits_const<16>(12345678), detail::remIleavedBits_naive(12345678, 16));
    BITMANIP_STATIC_ASSERT_EQ(remIleavedBits_const<32>(12345678), detail::remIleavedBits_naive(12345678, 32));
    BITMANIP_STATIC_ASSERT_EQ(remIleavedBits_const<63>(12345678), detail::remIleavedBits_naive(12345678, 63));
    BITMANIP_STATIC_ASSERT_EQ(remIleavedBits_const<64>(12345678), detail::remIleavedBits_naive(12345678, 64));
}

BITMANIP_TEST(bitileave, remIleavedBits_naive_matches_templated)
{
    constexpr size_t iterations = 1024 * 8;

    std::mt19937 rng{12345};
    std::uniform_int_distribution<std::uint64_t> distr{0, std::numeric_limits<std::uint64_t>::max()};

    for (size_t i = 0; i < iterations; ++i) {
        const auto input = distr(rng);
        BITMANIP_ASSERT_EQ(remIleavedBits_const<1>(input), detail::remIleavedBits_naive(input, 1));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<2>(input), detail::remIleavedBits_naive(input, 2));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<3>(input), detail::remIleavedBits_naive(input, 3));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<4>(input), detail::remIleavedBits_naive(input, 4));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<5>(input), detail::remIleavedBits_naive(input, 5));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<6>(input), detail::remIleavedBits_naive(input, 6));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<7>(input), detail::remIleavedBits_naive(input, 7));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<8>(input), detail::remIleavedBits_naive(input, 8));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<9>(input), detail::remIleavedBits_naive(input, 9));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<10>(input), detail::remIleavedBits_naive(input, 10));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<11>(input), detail::remIleavedBits_naive(input, 11));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<12>(input), detail::remIleavedBits_naive(input, 12));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<13>(input), detail::remIleavedBits_naive(input, 13));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<14>(input), detail::remIleavedBits_naive(input, 14));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<15>(input), detail::remIleavedBits_naive(input, 15));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<16>(input), detail::remIleavedBits_naive(input, 16));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<32>(input), detail::remIleavedBits_naive(input, 32));
        BITMANIP_ASSERT_EQ(remIleavedBits_const<63>(input), detail::remIleavedBits_naive(input, 63));
    }
}

BITMANIP_TEST(bitileave, ileaveZeros_naive_matches_template)
{
    constexpr size_t iterations = 1024 * 8;

    std::mt19937 rng{12345};
    std::uniform_int_distribution<std::uint32_t> distr{0, std::numeric_limits<std::uint32_t>::max()};

    for (size_t i = 0; i < iterations; ++i) {
        const auto input = distr(rng);
        BITMANIP_ASSERT_EQ(ileaveZeros_const<1>(input), detail::ileaveZeros_naive(input, 1));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<2>(input), detail::ileaveZeros_naive(input, 2));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<3>(input), detail::ileaveZeros_naive(input, 3));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<4>(input), detail::ileaveZeros_naive(input, 4));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<5>(input), detail::ileaveZeros_naive(input, 5));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<6>(input), detail::ileaveZeros_naive(input, 6));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<7>(input), detail::ileaveZeros_naive(input, 7));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<8>(input), detail::ileaveZeros_naive(input, 8));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<9>(input), detail::ileaveZeros_naive(input, 9));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<10>(input), detail::ileaveZeros_naive(input, 10));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<11>(input), detail::ileaveZeros_naive(input, 11));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<12>(input), detail::ileaveZeros_naive(input, 12));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<13>(input), detail::ileaveZeros_naive(input, 13));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<14>(input), detail::ileaveZeros_naive(input, 14));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<15>(input), detail::ileaveZeros_naive(input, 15));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<16>(input), detail::ileaveZeros_naive(input, 16));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<16>(input), detail::ileaveZeros_naive(input, 16));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<32>(input), detail::ileaveZeros_naive(input, 32));
        BITMANIP_ASSERT_EQ(ileaveZeros_const<63>(input), detail::ileaveZeros_naive(input, 63));
    }
}

BITMANIP_TEST(bitileave, ileave_manual)
{
    BITMANIP_STATIC_ASSERT_EQ(ileave(0b1111'1111u, 0u), 0b1010'1010'1010'1010u);
    BITMANIP_STATIC_ASSERT_EQ(ileave(0u, 0b1'1111'1111u), 0b01'0101'0101'0101'0101u);
    BITMANIP_STATIC_ASSERT_EQ(ileave(0u, 0xffff'ffffu), 0x5555'5555'5555'5555u);
    BITMANIP_STATIC_ASSERT_EQ(ileave(0u, static_cast<std::uint32_t>(ileave(0u, 0b11u))), 0b10001u);

    BITMANIP_STATIC_ASSERT_EQ(ileave(0u, 0u, 0b1111u), 0b001001001001u);
    BITMANIP_STATIC_ASSERT_EQ(ileave(0b1111u, 0u, 0u), 0b100100100100u);
}

BITMANIP_TEST(bitileave, ileaveBytes_manual)
{
    for (size_t i = 0; i <= 8; ++i) {
        BITMANIP_ASSERT_EQ(ileaveBytes(0, i), 0u);
    }

    BITMANIP_ASSERT_EQ(ileaveBytes(0xcc, 1), 0xccu);

    BITMANIP_ASSERT_EQ(ileaveBytes(0xff, 2), 0x5555u);
    BITMANIP_ASSERT_EQ(ileaveBytes(0xff00, 2), 0xaaaau);

    BITMANIP_ASSERT_EQ(ileaveBytes(0x0000ff, 3), 0b001'001'001'001'001'001'001'001u << 0);
    BITMANIP_ASSERT_EQ(ileaveBytes(0x00ff00, 3), 0b001'001'001'001'001'001'001'001u << 1);
    BITMANIP_ASSERT_EQ(ileaveBytes(0xff0000, 3), 0b001'001'001'001'001'001'001'001u << 2);

    BITMANIP_ASSERT_EQ(ileaveBytes(0x000000ff, 8), 0x0101'0101'0101'0101u);
    BITMANIP_ASSERT_EQ(ileaveBytes(0x0000ff00, 8), 0x0202'0202'0202'0202u);
    BITMANIP_ASSERT_EQ(ileaveBytes(0x00ff0000, 8), 0x0404'0404'0404'0404u);
    BITMANIP_ASSERT_EQ(ileaveBytes(0xff000000, 8), 0x0808'0808'0808'0808u);

    BITMANIP_ASSERT_EQ(ileaveBytes(0xff000000ff, 8), 0x1111'1111'1111'1111u);
}

BITMANIP_TEST(bitileave, ileaveBytes_naive_matches_jmp)
{
    constexpr size_t iterations = 1024 * 16;

    fast_rng64 rng{12345};
    std::uniform_int_distribution<std::uint64_t> distr;

    for (size_t i = 0; i < iterations; ++i) {
        std::uint64_t bytesAsInt = distr(rng);

        std::uint64_t jmp = detail::ileaveBytes_jmp(bytesAsInt, i % 9);
        std::uint64_t naive = detail::ileaveBytes_naive(bytesAsInt, i % 9);
        BITMANIP_ASSERT_EQ(jmp, naive);
    }
}

BITMANIP_TEST(bitileave, ileaveBytes_bitCountPreserved)
{
    constexpr size_t iterations = 1024 * 16;

    fast_rng64 rng{12345};
    std::uniform_int_distribution<std::uint64_t> distr;

    for (size_t i = 0; i < iterations; ++i) {
        std::uint8_t count = i % 9;
        std::uint64_t next = distr(rng);
        std::uint64_t bytesAsInt = count == 0 ? 0 : next >> ((8 - count) * 8);

        std::uint64_t ileaved = ileaveBytes(bytesAsInt, count);
        BITMANIP_ASSERT_EQ(popCount(ileaved), popCount(bytesAsInt));
    }
}

BITMANIP_TEST(bitileave, ileaveBytes_dileaveBytes_random)
{
    constexpr size_t iterations = 1024 * 16;

    fast_rng64 rng{12345};
    std::uniform_int_distribution<std::uint64_t> distr;

    for (size_t i = 0; i < iterations; ++i) {
        std::uint8_t count = i % 9;
        std::uint64_t next = distr(rng);
        std::uint64_t bytesAsInt = count == 0 ? 0 : next >> ((8 - count) * 8);

        std::uint64_t ileaved = ileaveBytes(bytesAsInt, count);
        std::uint64_t dileaved = dileave_bytes(ileaved, count);

        BITMANIP_ASSERT_EQ(dileaved, bytesAsInt);
    }
}

BITMANIP_TEST(bitileave, ileave3_naive_matches_regular)
{
    std::mt19937 rng{12345};
    std::uniform_int_distribution<std::uint32_t> distr{0, std::numeric_limits<std::uint32_t>::max()};

    for (size_t i = 0; i < 1024 * 1; ++i) {
        std::uint32_t v[] = {distr(rng), distr(rng), distr(rng)};
        BITMANIP_ASSERT_EQ(ileave(v), detail::ileave_naive(v[0], v[1], v[2]));
    }
}

BITMANIP_TEST(bitileave, dileave3_reverses_ileave3)
{
    std::mt19937 rng{12345};
    std::uniform_int_distribution<std::uint32_t> distr{0, 1u << 21};

    for (size_t i = 0; i < 1024 * 1; ++i) {
        const auto x = distr(rng), y = distr(rng), z = distr(rng);
        std::uint32_t expected[] = {x, y, z};
        std::uint32_t actual[3];
        detail::dileave_naive(ileave(x, y, z), actual[0], actual[1], actual[2]);
        BITMANIP_ASSERT_EQ(actual, expected);
    }
}

template <typename T, Rounding Round, Rounding TieBreak>
struct test_intdiv_tester {
    static constexpr T (*divFunc)(T, T) = div<Round, TieBreak>;

    template <auto num, auto den, auto quot>
    static constexpr void run()
    {
        if constexpr (den != 0) {
            // INT_MIN leads to unavoidable underflow for some divisions:
            // INT_MIN / -1 => signed integer overflow, UB
            constexpr bool isMinEdgeCase = std::min(T(num), T(den)) == std::numeric_limits<T>::min();

            BITMANIP_STATIC_ASSERT_EQ(divFunc(T(num), T(den)), T(quot));

            if constexpr (std::is_signed_v<T> && not isMinEdgeCase) {
                BITMANIP_STATIC_ASSERT_EQ(divFunc(T(num), T(-den)), T(-quot));
                BITMANIP_STATIC_ASSERT_EQ(divFunc(T(-num), T(den)), T(-quot));
                BITMANIP_STATIC_ASSERT_EQ(divFunc(T(-num), T(-den)), T(quot));
            }
        }
    }
};

template <typename T, Rounding Round, Rounding TieBreak>
void test_intdiv_manual_norem_impl()
{
    using tester = test_intdiv_tester<T, Round, TieBreak>;

    // trivial cases
    tester::template run<0, 1, 0>();
    tester::template run<0, 1, 0>();
    tester::template run<1, 1, 1>();

    // division by one
    tester::template run<0, 1, 0>();
    tester::template run<1, 1, 1>();
    tester::template run<2, 1, 2>();
    tester::template run<3, 1, 3>();

    // division by 2
    tester::template run<0, 2, 0>();
    tester::template run<2, 2, 1>();
    tester::template run<4, 2, 2>();
    tester::template run<8, 2, 4>();
    tester::template run<16, 2, 8>();
    tester::template run<32, 2, 16>();
    tester::template run<64, 2, 32>();

    // division by seven
    tester::template run<0, 7, 0>();
    tester::template run<7, 7, 1>();
    tester::template run<14, 7, 2>();
    tester::template run<21, 7, 3>();
    tester::template run<28, 7, 4>();
    tester::template run<35, 7, 5>();

    constexpr T min = std::numeric_limits<T>::min();
    constexpr T max = std::numeric_limits<T>::max();

    tester::template run<0, max, 0>();
    tester::template run<0, min, 0>();

    tester::template run<max, 1, max>();
    tester::template run<min, 1, min>();

    tester::template run<max, max, 1>();
    tester::template run<min, min, 1>();
}

template <Rounding Round, Rounding TieBreak = Rounding::MAGNIFY>
void test_intdiv_manual_norem()
{
    test_intdiv_manual_norem_impl<signed char, Round, TieBreak>();
    test_intdiv_manual_norem_impl<unsigned char, Round, TieBreak>();
    test_intdiv_manual_norem_impl<int, Round, TieBreak>();
    test_intdiv_manual_norem_impl<unsigned, Round, TieBreak>();
    test_intdiv_manual_norem_impl<long long, Round, TieBreak>();
    test_intdiv_manual_norem_impl<unsigned long long, Round, TieBreak>();
}

BITMANIP_TEST(intdiv, manual_norem_trunc)
{
    test_intdiv_manual_norem<Rounding::TRUNC>();
}

BITMANIP_TEST(intdiv, manual_norem_floor)
{
    test_intdiv_manual_norem<Rounding::FLOOR>();
}

BITMANIP_TEST(intdiv, manual_norem_ceil)
{
    test_intdiv_manual_norem<Rounding::CEIL>();
}

BITMANIP_TEST(intdiv, manual_norem_magnify)
{
    test_intdiv_manual_norem<Rounding::MAGNIFY>();
}

BITMANIP_TEST(intdiv, manual_norem_round_trunc)
{
    test_intdiv_manual_norem<Rounding::ROUND, Rounding::TRUNC>();
}

BITMANIP_TEST(intdiv, manual_norem_round_magnify)
{
    test_intdiv_manual_norem<Rounding::ROUND, Rounding::MAGNIFY>();
}

// TEST EXECUTION ======================================================================================================

int testFailureCount = 0;

constexpr const char *TEST_ORDER[]{"bit", "bitcount", "bitileave", "bitrev", "bitrot", "intdiv", "intlog"};

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
