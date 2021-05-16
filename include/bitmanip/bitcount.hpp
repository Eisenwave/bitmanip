#ifndef BITMANIP_BITCOUNT_HPP
#define BITMANIP_BITCOUNT_HPP

#include "bit.hpp"
#include "builtin.hpp"

#include <type_traits>

#ifndef BITMANIP_HAS_BUILTIN_POPCOUNT
#include <bitset>
#endif

namespace bitmanip {

// CLZ AND CTZ =========================================================================================================

namespace detail {

template <typename Uint>
[[nodiscard]] constexpr unsigned char countLeadingZeros_naive(Uint input) noexcept
{
    constexpr Uint highestBit = Uint{1} << (bits_v<Uint> - 1);

    if (input == 0) {
        return bits_v<Uint>;
    }
    unsigned char result = 0;
    for (; (input & highestBit) == 0; input <<= 1) {
        ++result;
    }
    return result;
}

template <typename Uint>
[[nodiscard]] constexpr unsigned char countTrailingZeros_naive(Uint input) noexcept
{
    if (input == 0) {
        return bits_v<Uint>;
    }
    unsigned char result = 0;
    for (; (input & 1) == 0; input >>= 1) {
        ++result;
    }
    return result;
}

template <typename Uint>
[[nodiscard]] constexpr unsigned char findFirstSet_parallel(Uint input) noexcept
{
    constexpr std::size_t iterations = log2bits_v<Uint>;
    constexpr std::size_t resultMask = bits_v<Uint> - 1;

    std::size_t result = bits_v<Uint>;
    for (std::size_t i = 0, add = 1; i < iterations; ++i, add <<= 1) {
        bool hasBits = input & ALTERNATING_MASKS<Uint>[i];
        result -= hasBits * add;
    }

    return static_cast<unsigned char>(result & resultMask);
}

template <typename Uint>
[[nodiscard]] constexpr unsigned char countTrailingZeros_ffs(Uint input) noexcept
{
    Uint ffs = findFirstSet_parallel(input);
    return ffs == 0 ? bits_v<Uint> : ffs - 1;
}

}  // namespace detail

/**
 * @brief Counts the number of leading zeros in a number.
 * The number of leading zeros in 0 is equal to the number of bits of the input type.
 * Example: countLeadingZeros(u8{7}) = 5
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr unsigned char countLeadingZeros(Uint input) noexcept
{
#ifdef BITMANIP_HAS_BUILTIN_CLZ
    if (builtin::isconsteval()) {
        return detail::countLeadingZeros_naive(input);
    }
    return input == 0 ? bits_v<Uint> : static_cast<unsigned char>(builtin::clz(input));
#else
    return detail::countLeadingZeros_naive(input);
#endif
}

/**
 * @brief Counts the number of trailing zeros in a number.
 * The number of trailing zeros in 0 is equal to the number of bits of the input type.
 * Example: countTrailingZeros(u8{8}) = 3
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr unsigned char countTrailingZeros(Uint input) noexcept
{
#ifdef BITMANIP_HAS_BUILTIN_CTZ
    if (builtin::isconsteval()) {
        return detail::countTrailingZeros_naive(input);
    }
    return input == 0 ? bits_v<Uint> : static_cast<unsigned char>(builtin::ctz(input));
#else
    return detail::countTrailingZeros_naive(input);
#endif
}

// BIT COUNTING ========================================================================================================

namespace detail {

// bitset operations are not constexpr so we need a naive implementations for constexpr contexts

template <typename Int>
[[nodiscard]] constexpr unsigned char popCount_naive(Int input) noexcept
{
    unsigned char result = 0;
    for (; input != 0; input >>= 1) {
        result += input & 1;
    }
    return result;
}

// This implementation is taken from https://stackoverflow.com/a/21618038 .
// The core idea is to XOR the high half bits with the low half bits recursively.
// In the end, the lowest bit will have been XORed with every other bit.
template <typename Int>
[[nodiscard]] constexpr bool parity_xor(Int input) noexcept
{
    constexpr unsigned iterations = log2bits_v<Int> - 1;

    for (unsigned shift = 1 << iterations; shift != 0; shift >>= 1) {
        input ^= input >> shift;
    }
    return input & 1;
}

}  // namespace detail

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr unsigned char popCount(Uint input) noexcept
{
#ifdef BITMANIP_HAS_BUILTIN_POPCOUNT
    return builtin::isconsteval() ? detail::popCount_naive(input)
                                  : static_cast<unsigned char>(builtin::popcount(input));
#else
    return detail::popCount_stdBitset(input);
#endif
}

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr bool parity(Uint input) noexcept
{
#ifdef BITMANIP_HAS_BUILTIN_PARITY
    return builtin::isconsteval() ? detail::parity_xor<Uint>(input) : builtin::parity(input);
#else
    return static_cast<unsigned char>(std::bitset<bits_v<Uint>>{input}.count());
#endif
}

}  // namespace bitmanip

#endif
