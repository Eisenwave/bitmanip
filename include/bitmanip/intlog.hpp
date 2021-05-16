#ifndef BITMANIP_INTLOG_HPP
#define BITMANIP_INTLOG_HPP
/*
 * intlog.hpp
 * -----------
 * Implements arithmetic related to integer logarithms or exponentiation.
 */

#include "bitcount.hpp"
#include "builtin.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <tuple>
#include <unordered_map>

namespace bitmanip {

#if defined(BITMANIP_HAS_BUILTIN_CLZ) || defined(BITMANIP_HAS_BUILTIN_MSB)
#define BITMANIP_HAS_BUILTIN_LOG2FLOOR
namespace detail {

template <typename Int>
inline Int log2floor_builtin(Int v)
{
#ifdef BITMANIP_HAS_BUILTIN_MSB
    return builtin::msb(v);
#elif defined(BITMANIP_HAS_BUILTIN_CLZ)
    constexpr int maxIndex = bits_v<Int> - 1;
    return (v != 0) * static_cast<Int>(maxIndex - builtin::clz(v));
#endif
}

}  // namespace detail
#endif

// POWER OF 2 TESTING ==================================================================================================

/**
 * @brief Returns whether an unsigned integer is a power of 2 or zero.
 * Note that this test is faster than having to test if val is a power of 2.
 * @param val the parameter to test
 * @return true if val is a power of 2 or if val is zero
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr bool isPow2or0(Uint val)
{
    return (val & (val - 1)) == 0;
}

/**
 * @brief Returns whether an unsigned integer is a power of 2.
 * @param val the parameter to test
 * @return true if val is a power of 2
 * @see is_pow2_or_zero
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr bool isPow2(Uint val)
{
    return val != 0 && isPow2or0(val);
}

// POWER OF 2 ROUNDING =================================================================================================

namespace detail {

#ifdef BITMANIP_HAS_BUILTIN_LOG2FLOOR
#define BITMANIP_HAS_BUILTIN_CEILPOW2
template <typename Uint>
inline Uint ceilPow2m1_builtin(Uint v)
{
    Uint log = log2floor_builtin(v);
    return v | (Uint{1} << log) - Uint{1};
}
#endif

template <typename Uint>
constexpr Uint ceilPow2m1_shift(Uint v)
{
    constexpr std::size_t iterations = log2bits_v<Uint>;
    for (std::size_t i = 0; i < iterations; ++i) {
        // after all iterations, all bits right to the msb will be filled with 1
        v |= v >> (1 << i);
    }
    return v;
}

}  // namespace detail

/**
 * @brief Rounds up an unsigned integer to the next power of 2, minus 1.
 * 0 is not rounded up and stays zero.
 * Examples: 100 -> 127, 1 -> 1, 3 -> 3, 3000 -> 4095, 64 -> 127
 * @param v the value to round up
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint ceilPow2m1(Uint v)
{
// The codegen on other platforms such as ARM is actually better for the repeated shift version.
// Each step on ARM can be performed with a flexible operand which performs a shift.
#if defined(BITMANIP_HAS_BUILTIN_CEILPOW2) && defined(BITMANIP_X86_OR_X64)
    if (not builtin::isconsteval()) {
        return detail::ceilPow2m1_builtin(v);
    }
#endif
    return detail::ceilPow2m1_shift(v);
}

/**
 * @brief Rounds up an unsigned integer to the next power of 2.
 * Powers of two are not affected.
 * 0 is not rounded and stays zero.
 * Examples: 100 -> 128, 1 -> 1, 3 -> 4, 3000 -> 4096
 * @param v the value to round up
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint ceilPow2(Uint v)
{
    return ceilPow2m1(v - 1) + 1;
}

/**
 * @brief Rounds down an unsigned integer to the next power of 2.
 * Powers of 2 are not affected.
 * The result of floorPow2(0) is undefined.
 * Examples: 100 -> 64, 1 -> 1, 3 -> 2, 3000 -> 2048
 * @param v the value to round down
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint floorPow2(Uint v)
{
    return ceilPow2m1(v >> 1) + 1;
}

// BASE 2 LOGARITHMS ===================================================================================================

/**
 * @brief Naive implementation of log2 using repeated single-bit rightshifting.
 */
template <typename Uint>
constexpr Uint log2floor_naive(Uint val) noexcept
{
    Uint result = 0;
    while (val >>= 1) {
        ++result;
    }
    return result;
}

/**
 * @brief Fast implementation of log2.
 * See https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog.
 *
 * Unrolled 32-bit version:
 * unsigned shift = (v > 0xFFFF) << 4;
    v >>= shift;
    r |= shift;

    shift = (v > 0xFF  ) << 3;
    v >>= shift;
    r |= shift;

    shift = (v > 0xF   ) << 2;
    v >>= shift;
    r |= shift;

    shift = (v > 0x3   ) << 1;
    v >>= shift;
    r |= shift;

    shift = (v > 1) << 0;
    r >>= shift;
    r |= shift;
 */
template <typename Uint>
constexpr Uint log2floor_fast(Uint v) noexcept
{
    constexpr std::size_t iterations = log2bits_v<Uint>;
    unsigned result = 0;

    for (std::size_t i = iterations; i != 0; --i) {
        unsigned compBits = 1 << (i - 1);
        Uint compShift = Uint{1} << compBits;
        unsigned shift = unsigned{v >= compShift} << (i - 1);
        v >>= shift;
        result |= shift;
    }

    return result;
}

namespace detail {

constexpr unsigned char MultiplyDeBruijnBitPosition[32] = {0,  9,  1,  10, 13, 21, 2,  29, 11, 14, 16,
                                                           18, 22, 25, 3,  30, 8,  12, 20, 28, 15, 17,
                                                           24, 7,  19, 27, 23, 6,  26, 5,  4,  31};

}

/**
 * @brief log2floor implementation using De Bruijn multiplication.
 * See https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn.
 * @param val the value
 */
constexpr std::uint32_t log2floor_debruijn(std::uint32_t val) noexcept
{
    constexpr std::uint32_t magic = 0x07C4ACDD;

    val = ceilPow2m1(val);
    val *= magic;
    val >>= 27;

    return detail::MultiplyDeBruijnBitPosition[val];
}

/**
 * @brief Computes the floored binary logarithm of a given integer.
 * Example: log2floor(123) = 6
 *
 * This templated function will choose the best available method depending on the type of the integer.
 * It is undefined for negative values.
 *
 * Unlike a traditional log function, it is defined for 0: log2floor(0) = 0
 *
 * @param v the value
 * @return the floored binary logarithm
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint log2floor(Uint v) noexcept
{
#ifdef BITMANIP_HAS_BUILTIN_LOG2FLOOR
    if (not builtin::isconsteval()) {
        return detail::log2floor_builtin(v);
    }
#endif
    if constexpr (std::is_same_v<Uint, std::uint32_t>) {
        return log2floor_debruijn(v);
    }
    else {
        return log2floor_fast<Uint>(v);
    }
}

/**
 * @brief Computes the ceiled binary logarithm of a given integer.
 * Example: log2ceil(123) = 7
 *
 * This templated function will choose the best available method depending on the type of the integer.
 * It is undefined for negative values.
 *
 * Unlike a traditional log function, it is defined for 0: log2ceil(0) = 0
 *
 * @param v the value
 * @return the floored binary logarithm
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint log2ceil(Uint val) noexcept
{
    const Uint result = log2floor(val);
    return result + not isPow2or0(val);
}

/**
 * @brief Computes the number of bits required to represent a given number.
 * Examples: bitLength(0) = 1, bitLength(3) = 2, bitLength(123) = 7, bitLength(4) = 3
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint bitCount(Uint val) noexcept
{
    return log2floor(val) + 1;
}

// ARBITRARY BASE LOGARITHMS ===========================================================================================

namespace detail {

/**
 * @brief Naive implementation of log base N using repeated division.
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint logFloor_naive(Uint val, unsigned base) noexcept
{
    Uint result = 0;
    while (val /= base) {
        ++result;
    }
    return result;
}

}  // namespace detail

/**
 * @brief The maximum possible exponent for a given base that can still be represented by a given integer type.
 * Example: maxExp<uint8_t, 10> = 2, because 10^2 is representable by an 8-bit unsigned integer but 10^3 isn't.
 */
template <unsigned BASE, BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint maxExp = detail::logFloor_naive<Uint>(static_cast<Uint>(~Uint{0u}), BASE);

static_assert(maxExp<10, std::uint8_t> == 2);
static_assert(maxExp<10, std::uint16_t> == 4);
static_assert(maxExp<10, std::uint32_t> == 9);

namespace detail {

/**
 * @brief Simple implementation of log base N using repeated multiplication.
 * This method is slightly more sophisticated than logFloor_naive because it avoids division.
 */
template <unsigned BASE, BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint logFloor_simple(Uint val) noexcept
{
    constexpr Uint limit = maxExp<BASE, Uint>;

    Uint i = 0;
    Uint pow = BASE;
    for (; i <= limit; ++i, pow *= BASE) {
        if (val < pow) {
            return i;
        }
    }
    return i;
}

/**
 * @brief Tiny array implementation to avoid including <array>.
 */
template <typename T, std::size_t N>
struct Table {
    static_assert(N != 0, "Can't create zero-size tables");

    static constexpr std::size_t size = N;
    T data[N];

    constexpr T front() const noexcept
    {
        return data[0];
    }

    constexpr T back() const noexcept
    {
        return data[N - 1];
    }

    constexpr T &operator[](std::size_t i) noexcept
    {
        return data[i];
    }

    constexpr const T &operator[](std::size_t i) const noexcept
    {
        return data[i];
    }
};

template <typename Uint, std::size_t BASE>
constexpr Table<uint8_t, bits_v<Uint>> makeGuessTable()
{
    using resultType = decltype(makeGuessTable<Uint, BASE>());

    resultType result{};
    for (std::size_t i = 0; i < resultType::size; ++i) {
        const Uint pow2 = static_cast<Uint>(Uint{1} << i);
        result[i] = logFloor_naive(pow2, BASE);
    }
    return result;
}

template <typename Uint, std::size_t BASE, typename TableUint = nextLargerUintType<Uint>>
constexpr Table<TableUint, maxExp<BASE, Uint> + 2> makePowerTable()
{
    // the size of the table is maxPow<BASE, Uint> + 2 because we need to store the maximum power
    // +1 because we need to contain it, we are dealing with a size, not an index
    // +1 again because for narrow integers, we access one beyond the "end" of the table
    //
    // as a result, the last multiplication with BASE might overflow, but this is perfectly normal
    using resultType = decltype(makePowerTable<Uint, BASE, TableUint>());

    resultType result{};
    std::uintmax_t x = 1;
    for (std::size_t i = 0; i < resultType::size; ++i, x *= BASE) {
        result[i] = static_cast<TableUint>(x);
    }
    return result;
}

// table that maps from log_2(val) -> approximate log_N(val)
template <typename Uint, std::size_t BASE>
constexpr auto logFloor_guesses = detail::makeGuessTable<Uint, BASE>();

// table that maps from log_N(val) -> pow(N, val + 1)
template <typename Uint, std::size_t BASE>
constexpr auto logFloor_powers = detail::makePowerTable<Uint, BASE>();

template <typename Uint, std::size_t BASE>
constexpr auto powConst_powers = detail::makePowerTable<Uint, BASE, Uint>();

}  // namespace detail

/**
 * @brief Computes pow(BASE, exponent) where BASE is known at compile-time.
 */
template <std::size_t BASE, BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint powConst(Uint exponent)
{
    if constexpr (isPow2(BASE)) {
        return 1 << (exponent * log2floor(BASE));
    }
    else {
        return detail::powConst_powers<Uint, BASE>[exponent];
    }
}

/**
 * @brief Computes the floored logarithm of a number with a given base.
 *
 * Examples:
 *     logFloor<10>(0) = 0
 *     logFloor<10>(5) = 0
 *     logFloor<10>(10) = 1
 *     logFloor<10>(123) = 2
 *
 * Note that unlike a traditional logarithm function, it is defined for 0 and is equal to 0.
 *
 * @see https://stackoverflow.com/q/63411054
 * @tparam BASE the base (e.g. 10)
 * @param val the input value
 * @return floor(log(val, BASE))
 */
template <std::size_t BASE = 10, BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint logFloor(Uint val) noexcept
{
    if constexpr (isPow2(BASE)) {
        return log2floor(val) / log2floor(BASE);
    }
    else {
        constexpr auto &guesses = detail::logFloor_guesses<Uint, BASE>;
        constexpr auto &powers = detail::logFloor_powers<Uint, BASE>;

        const std::uint8_t guess = guesses[log2floor(val)];

        if constexpr (sizeof(Uint) < sizeof(std::uint64_t) || guesses.back() + 2 < powers.size) {
            return guess + (val >= powers[guess + 1]);
        }
        else {
            return guess + (val / BASE >= powers[guess]);
        }
    }
}

/**
 * @brief Convenience function that forwards to logFloor<10>.
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint log10floor(Uint val)
{
    return logFloor<10, Uint>(val);
}

/**
 * @brief Computes the number of digits required to represent a number with a given base.
 */
template <std::size_t BASE = 10, BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint digitCount(Uint val) noexcept
{
    return logFloor<BASE>(val) + 1;
}

}  // namespace bitmanip

#endif
