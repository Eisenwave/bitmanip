#ifndef BITMANIP_BIT_HPP
#define BITMANIP_BIT_HPP

#include <cstdint>
#include <type_traits>

#define BITMANIP_INTEGRAL_TYPENAME(T) typename T, ::std::enable_if_t<::std::is_integral_v<T>, int> = 0
#define BITMANIP_UNSIGNED_TYPENAME(T) typename T, ::std::enable_if_t<::std::is_unsigned_v<T>, int> = 0

namespace bitmanip {

// TYPE BIT COUNTS =====================================================================================================

/**
 * @brief templated variable which contains the number of bits for any given integer type.
 * Unlike std::numeric_limits<T>::digits, this simply considers the bit count, which is different from the digit count
 * for signed types.
 * Example: bits_v<std::uint32_t> = 32
 */
template <BITMANIP_INTEGRAL_TYPENAME(Int)>
constexpr unsigned bits_v = sizeof(Int) * 8;

/**
 * @brief templated variable which contains the log2 of the number of bits of a type.
 * Example: log2bits_v<std::uint32_t> = 5
 */
template <BITMANIP_INTEGRAL_TYPENAME(Int)>
constexpr unsigned log2bits_v = "0112222333333334"[sizeof(Int) - 1] + 3 - '0';

// TRAITS ==============================================================================================================

namespace detail {

template <typename Int>
auto nextLargerUint_impl()
{
    if constexpr (bits_v<Int> >= 8) {
        return std::uint_least16_t{0};
    }
    else if constexpr (bits_v<Int> >= 16) {
        return std::uint_least32_t{0};
    }
    else if constexpr (bits_v<Int> >= 32) {
        return std::uint_least64_t{0};
    }
    else {
        return std::uintmax_t{0};
    }
}

}  // namespace detail

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
using nextLargerUintType = decltype(detail::nextLargerUint_impl<Uint>());

// ALTERNATING BIT SEQUENCE ============================================================================================

/**
 * @brief Creates an alternating sequence of 1s and 0s, starting with 1.
 *
 * Examples: alternate(1, 2) = 0b...0101010101
 *           alternate(1, 3) = 0b...1001001001
 *           alternate(2, 2) = 0b...1100110011
 *           alternate(2, 4) = 0b...1100000011
 *
 * @param period describes how many 1 and 0 bits per period should be stored, must not be zero
 * @param mod the total number of bits per period, before scaling by the period parameter. Only the first bit of each
 * unscaled period is a 1-bit. If mod == 1, then all bits are 1-bits. Must not be zero.
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint alternate(unsigned period = 1, unsigned mod = 2) noexcept
{
    Uint result = 0;
    for (std::size_t i = 0; i < bits_v<Uint>; ++i) {
        Uint bit = ((i / period) % mod) == 0;
        result |= bit << i;
    }
    return result;
}

namespace detail {

template <typename Uint, unsigned MOD = 2>
inline constexpr Uint ALTERNATING_MASKS[7] = {alternate<Uint>(1, MOD),
                                              alternate<Uint>(2, MOD),
                                              alternate<Uint>(4, MOD),
                                              alternate<Uint>(8, MOD),
                                              alternate<Uint>(16, MOD),
                                              alternate<Uint>(32, MOD),
                                              alternate<Uint>(64, MOD)};

}  // namespace detail

// BIT GETTING / SETTING ===============================================================================================

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr bool getBit(Uint input, unsigned index) noexcept
{
    return (input >> index) & Uint{1};
}

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint clearBit(Uint input, unsigned index) noexcept
{
    return input & ~(Uint{1} << index);
}

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint flipBit(Uint input, unsigned index) noexcept
{
    return input ^ (Uint{1} << index);
}

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint setBit(Uint input, unsigned index) noexcept
{
    return input | (Uint{1} << index);
}

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint setBit(Uint input, unsigned index, bool value) noexcept
{
    return clearBit(input, index) | (Uint{value} << index);
}

// ADVANCES SINGLE-BIT OPERATIONS ======================================================================================

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint makeMask(Uint length) noexcept
{
    return (Uint{1} << length) - Uint{1};
}

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint isolateLsb(Uint input) noexcept
{
    return input & -input;
}

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint resetLsb(Uint input) noexcept
{
    return input & (input - 1);
}

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint hiMaskUntilLsb(Uint input) noexcept
{
    return input ^ -input;
}

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint loMaskUntilLsb(Uint input) noexcept
{
    return ~(input ^ -input);
}

}  // namespace bitmanip

#endif
