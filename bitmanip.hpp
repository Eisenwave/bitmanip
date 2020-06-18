#pragma once
#ifndef BITMANIP_HPP
#define BITMANIP_HPP

#include <cstddef>
#include <cstdint>
#include <type_traits>

// ===== USER CONFIGURATION ============================================================================================

// Uncomment the following #define if you prefer std::includes over the custom solutions provided in this section.
// There are few functions that use <array> and <algorithm> (of which only std::min is used), so you can save on
// compilation time by using the definitions in this section.
// #define BITMANIP_PREFER_STD_INCLUDES

// Uncomment the following #define if you want a bitmanip:: namespace.
// #define BITMANIP_PREFER_NAMESPACE

// ====== CONDITIONALLY CONFIGURED CODE ================================================================================
#ifdef PREFER_STD_INCLUDES
#include <algorithm>
#include <array>

#ifdef BITMANIP_PREFER_NAMESPACE
namespace bitmanip {
#endif

template <typename T, size_t N>
using arr = std::array<T, N>;

using std::min;

#else

#ifdef BITMANIP_PREFER_NAMESPACE
namespace bitmanip {
#endif
/**
 * @brief Simple implementation of min to avoid including the entirety of <algorithm>
 * @param x the first value
 * @param y the second value
 * @param the minimum of the values
 */
template <typename T>
constexpr T min(T x, T y)
{
    return x < y ? x : y;
}

template <typename T, size_t N>
struct arr {
    T data[N];
};
#endif
// ==== UTILITY ========================================================================================================

/**
 * @brief templated variable which contains the number of bits for any given integer type.
 * Example: bits_v<uint32_t> = 32
 */
template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr size_t bits_v = 0;

template <>
constexpr size_t bits_v<char> = 8;

template <>
constexpr size_t bits_v<bool> = bits_v<char> * sizeof(bool);

template <>
constexpr size_t bits_v<uint8_t> = 8;

template <>
constexpr size_t bits_v<uint16_t> = 16;

template <>
constexpr size_t bits_v<uint32_t> = 32;

template <>
constexpr size_t bits_v<uint64_t> = 64;

template <>
constexpr size_t bits_v<int8_t> = 8;

template <>
constexpr size_t bits_v<int16_t> = 16;

template <>
constexpr size_t bits_v<int32_t> = 32;

template <>
constexpr size_t bits_v<int64_t> = 64;

/**
 * @brief Integer division but with ceiling (rounding up to the next integer) instead of rounding down as usual.
 * @param the dividend
 * @param the divisor
 * @param the quotient, rounded up to the nearest integer
 */
template <typename Int, std::enable_if_t<(std::is_integral_v<Int>), int> = 0>
constexpr Int div_ceil(Int x, Int y)
{
    return 1 + ((x - 1) / y);
}

// ===== BIT MANIPULATION ==============================================================================================

/**
 * @brief Returns whether an unsigned integer is a power of 2 or zero.
 * Note that this test is faster than having to test if val is a power of 2.
 * @param val the parameter to test
 * @return true if val is a power of 2 or if val is zero
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr bool is_pow2_or_zero(Uint val)
{
    return (val & (val - 1)) == 0;
}

/**
 * @brief Returns whether an unsigned integer is a power of 2.
 * @param val the parameter to test
 * @return true if val is a power of 2
 * @see is_pow2_or_zero
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr bool is_pow2(Uint val)
{
    return val != 0 && is_pow2_or_zero(val);
}

/**
 * @brief Naive implementation of log2 using repeated single-bit rightshifting.
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint log2_floor_naive(Uint val) noexcept
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
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint log2_floor_fast(Uint v) noexcept
{
    constexpr size_t iterations = log2_floor_naive(bits_v<Uint>);
    unsigned result = 0;

    for (size_t i = iterations; i != 0; --i) {
        unsigned compBits = 1 << (i - 1);
        Uint compShift = Uint{1} << compBits;
        unsigned shift = unsigned{v >= compShift} << (i - 1);
        v >>= shift;
        result |= shift;
    }

    return result;
}

/**
 * @brief log2_floor implementation using De Bruijn multiplication.
 * See https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn.
 * @param val the value
 */
constexpr uint32_t log2_floor_debruijn(uint32_t val) noexcept
{
    constexpr uint32_t MultiplyDeBruijnBitPosition[32] = {0, 9,  1,  10, 13, 21, 2,  29, 11, 14, 16, 18, 22, 25, 3, 30,
                                                          8, 12, 20, 28, 15, 17, 24, 7,  19, 27, 23, 6,  26, 5,  4, 31};

    // first round up to one less than a power of 2
    // this step is not necessary if val is a power of 2
    // Note: the link in @see says that this is rounding down, but it is actually rounding up
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;

    return MultiplyDeBruijnBitPosition[(val * uint32_t{0x07C4ACDD}) >> 27];
}

/**
 * @brief Computes the floored binary logarithm of a given integer.
 * Example: log2_floor(100) = 6
 *
 * This templated function will choose the best available method depending on the type of the integer.
 * It may fail if a negative signed integer is given.
 *
 * @param v the value
 * @return the floored binary logarithm
 */
template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr Int log2_floor(Int v) noexcept
{
    if constexpr (std::is_signed_v<Int>) {
        using Uint = std::make_unsigned_t<Int>;
        DEBUG_ASSERT_GE(v, 0);
        return log2_floor_fast<Uint>(static_cast<Uint>(v));
    }
    else {
        return log2_floor_fast<Int>(v);
    }
}

template <>
constexpr uint32_t log2_floor<uint32_t>(uint32_t v) noexcept
{
    return log2_floor_debruijn(v);
}

/**
 * @brief Computes the ceiled binary logarithm of a given integer.
 * Example: log2_ceil(100) = 7
 *
 * This templated function will choose the best available method depending on the type of the integer.
 * It may fail if a negative signed integer is given.
 *
 * @param v the value
 * @return the floored binary logarithm
 */
template <typename Int, std::enable_if_t<std::is_integral_v<Int>, int> = 0>
constexpr Int log2_ceil(Int val) noexcept
{
    const Int result = log2_floor(val);
    const bool inputIsntPow2 = val != (static_cast<Int>(1) << result);
    return result + inputIsntPow2;
}

/**
 * @brief Rounds up an unsigned integer to the next power of 2.
 * Powers of two are not affected.
 * Examples: 100 -> 128, 1 -> 1, 3 -> 4, 3000 -> 4096
 * @param v the value to round up
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint ceil_pow2(Uint v)
{
    DEBUG_ASSERT_NE(v, 0);
    constexpr size_t iterations = log2_floor_naive(bits_v<Uint>);
    // decrement is necessary so that powers of 2 don't get increased
    v--;
    for (size_t i = 0; i < iterations; ++i) {
        // after all iterations, all bits right to the msb will be filled with 1
        v |= v >> (1 << i);
    }
    // this step turns the number back to a power of two
    return v + 1;
}

/**
 * @brief Rounds down an unsigned integer to the next power of 2.
 * Powers of 2 are not affected.
 * Examples: 100 -> 64, 1 -> 1, 3 -> 2, 3000 -> 2048
 * @param v the value to round down
 */
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint>, int> = 0>
constexpr Uint floor_pow2(Uint v)
{
    DEBUG_ASSERT_NE(v, 0);
    constexpr size_t iterations = log2_floor_naive(bits_v<Uint>);
    // leftshift is necessary to floor instead of ceil
    v >>= 1;
    for (size_t i = 0; i < iterations; ++i) {
        // after all iterations, all bits right to the msb will be filled with 1
        v |= v >> (1 << i);
    }
    // this step turns the number back to a power of two
    return v + 1;
}

/**
 * @brief Interleaves an input number with <bits> zero-bits per input bit. Example: 0b11 -> 0b0101
 * @param input the input number
 * @param bits the amount of zero bits per input bit to interleave
 * @return the input number interleaved with input-bits
 */
constexpr uint64_t ileave_zeros_naive(uint32_t input, size_t bits)
{
    const auto lim = min<size_t>(32, div_ceil<size_t>(64, bits + 1));

    uint64_t result = 0;
    for (size_t i = 0, b_out = 0; i < lim; ++i) {
        result |= static_cast<uint64_t>(input & 1) << b_out;
        input >>= 1;
        b_out += bits + 1;
    }

    return result;
}

/**
 * @brief Removes each interleaved <bits> bits. Example: 0b010101 --rem 1--> 0b111
 * @param input the input number
 * @param bits input bits per output bit
 * @return the the output with removed bits
 */
constexpr uint64_t rem_ileaved_bits_naive(uint64_t input, size_t bits) noexcept
{
    // increment once to avoid modulo divisions by 0
    // this way our function is noexcept and safe for all inputs
    ++bits;
    uint64_t result = 0;
    for (size_t i = 0, b_out = 0; i < 64; ++i) {
        if (i % bits == 0) {
            result |= (input & 1) << b_out++;
        }
        input >>= 1;
    }

    return result;
}

/**
 * @brief Duplicates each input bit <bits> times. Example: 0b101 -> 0b110011
 * @param input the input number
 * @param out_bits_per_in_bits the output bits per input bits
 * @return the output number or 0 if the bits parameter was zero
 */
constexpr uint64_t dupl_bits_naive(uint64_t input, size_t out_bits_per_in_bits)
{
    if (out_bits_per_in_bits == 0) {
        return 0;
    }
    const auto lim = div_ceil<size_t>(64, out_bits_per_in_bits);

    uint64_t result = 0;
    for (size_t i = 0, b_out = 0; i < lim; ++i) {
        for (size_t j = 0; j < out_bits_per_in_bits; ++j, ++b_out) {
            result |= static_cast<uint64_t>((input >> i) & 1) << b_out;
        }
    }

    return result;
}

/**
 * @brief Interleaves <BITS> zero-bits inbetween each input bit.
 * @param input the input number
 * @tparam BITS the number of bits to be interleaved, must be > 0
 * @return the input interleaved with <BITS> bits
 */
template <size_t BITS>
constexpr uint64_t ileave_zeros(uint32_t input)
{
    if constexpr (BITS == 0) {
        return input;
    }
    else {
        constexpr uint64_t MASKS[] = {
            dupl_bits_naive(ileave_zeros_naive(~uint32_t(0), BITS), 1),
            dupl_bits_naive(ileave_zeros_naive(~uint32_t(0), BITS), 2),
            dupl_bits_naive(ileave_zeros_naive(~uint32_t(0), BITS), 4),
            dupl_bits_naive(ileave_zeros_naive(~uint32_t(0), BITS), 8),
            dupl_bits_naive(ileave_zeros_naive(~uint32_t(0), BITS), 16),
        };

        uint64_t n = input;
        for (int i = 4; i != -1; --i) {
            const auto shift = BITS * (1 << i);
            n |= n << shift;
            n &= MASKS[i];
        }

        return n;
    }
}

/**
 * @brief Removes each interleaved <BITS> bits. Example: 0b010101 --rem 1--> 0b111
 * If BITS is zero, no bits are removed and the input is returned.
 * @param input the input number
 * @param bits input bits per output bit
 * @return the the output with removed bits
 */
template <size_t BITS>
constexpr uint64_t rem_ileaved_bits(uint64_t input) noexcept
{
    if constexpr (BITS == 0) {
        return input;
    }
    else {
        constexpr uint64_t MASKS[] = {
            dupl_bits_naive(ileave_zeros_naive(~uint32_t(0), BITS), 1),
            dupl_bits_naive(ileave_zeros_naive(~uint32_t(0), BITS), 2),
            dupl_bits_naive(ileave_zeros_naive(~uint32_t(0), BITS), 4),
            dupl_bits_naive(ileave_zeros_naive(~uint32_t(0), BITS), 8),
            dupl_bits_naive(ileave_zeros_naive(~uint32_t(0), BITS), 16),
            dupl_bits_naive(ileave_zeros_naive(~uint32_t(0), BITS), 32),
        };

        input &= MASKS[0];
        for (size_t i = 0; i < 5; ++i) {
            unsigned rshift = (1 << i) * BITS;
            input |= input >> rshift;
            input &= MASKS[i + 1];
        }

        return input;
    }
}

/**
 * @brief Interleaves 2 integers, where hi comprises the upper bits of each bit pair and lo the lower bits.
 * Examle: ileave2(0b111, 0b000) = 0b101010
 *
 * This is also referred to as a Morton Code in scientific literature.
 *
 * @param hi the high bits
 * @param lo the low bits
 * @return the interleaved bits
 */
constexpr uint64_t ileave2(uint32_t hi, uint32_t lo)
{  // TODO implement like ileav3
    constexpr uint64_t MASKS[] = {0x5555'5555'5555'5555,
                                  0x3333'3333'3333'3333,
                                  0x0F0F'0F0F'0F0F'0F0F,
                                  0x00FF'00FF'00FF'00FF,
                                  0x0000'FFFF'0000'FFFF};

    uint64_t result = 0;
    uint32_t *nums[] = {&hi, &lo};

    for (size_t i = 0; i < 2; ++i) {
        uint64_t n = *nums[i];
        for (int i = 4; i != -1; --i) {
            n |= n << (1 << i);
            n &= MASKS[i];
        }
        result |= n << (1 - i);
    }

    return result;
}

/**
 * @brief Interleaves 3 integers, where x comprises the uppermost bits of each bit triple and z the lowermost bits.
 *
 * This is also referred to as a Morton Code in scientific literature.
 *
 * @param x the highest bits
 * @param y the middle bits
 * @param z the lowest bits
 * @return the interleaved bits
 */
constexpr uint64_t ileave3(uint32_t x, uint32_t y, uint32_t z)
{
    return (ileave_zeros<2>(x) << 2) | (ileave_zeros<2>(y) << 1) | ileave_zeros<2>(z);
}

/**
 * @brief Deinterleaves 3 integers which are interleaved in a single number.
 * Visualization: abcdefghi -> (adg, beh, cfi)
 *
 * This is also referred to as a Morton Code in scientific literature.
 *
 * @param n the number
 * @return the interleaved bits
 */
constexpr arr<uint32_t, 3> dileave3(uint64_t n)
{
    uint32_t x = static_cast<uint32_t>(rem_ileaved_bits<2>(n >> 2));
    uint32_t y = static_cast<uint32_t>(rem_ileaved_bits<2>(n >> 1));
    uint32_t z = static_cast<uint32_t>(rem_ileaved_bits<2>(n >> 0));
    return {x, y, z};
}

#ifdef BITMANIP_PREFER_NAMESPACE
}
#endif

#endif  // BITMANIP_HPP
