#include "test.hpp"

#include "bitmanip/intdiv.hpp"

namespace bitmanip {
namespace {

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

BITMANIP_TEST(traits, commonSignedType)
{
    BITMANIP_STATIC_ASSERT(std::is_same_v<unsigned, commonSignedType<unsigned, unsigned>>);
    BITMANIP_STATIC_ASSERT(std::is_same_v<int, commonSignedType<unsigned, int>>);
    BITMANIP_STATIC_ASSERT(std::is_same_v<int, commonSignedType<int, unsigned>>);
    BITMANIP_STATIC_ASSERT(std::is_same_v<int, commonSignedType<int, int>>);
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

}  // namespace
}  // namespace bitmanip
