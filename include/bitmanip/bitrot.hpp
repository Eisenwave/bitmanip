#ifndef BITMANIP_BITROT_HPP
#define BITMANIP_BITROT_HPP

#include "bit.hpp"

namespace bitmanip {

// BIT ROTATION ========================================================================================================

namespace detail {

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint rotateLeft_shift(Uint n, unsigned rot) noexcept
{
    constexpr unsigned char mask = 8 * sizeof(Uint) - 1;

    rot &= mask;
    Uint hi = n << rot;
    Uint lo = n >> (-rot & mask);
    return hi | lo;
}

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint rotateRight_shift(Uint n, unsigned rot) noexcept
{
    constexpr unsigned char mask = 8 * sizeof(Uint) - 1;

    rot &= mask;
    Uint lo = n >> rot;
    Uint hi = n << (-rot & mask);
    return hi | lo;
}

}  // namespace detail

/**
 * @brief Rotates an integer to the left by a given amount of bits.
 * This is similar to a leftshift, but the bits shifted out of the integer are inserted on the low side of the
 * integer.
 * @param n the number to shift
 * @param rot bit count
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint rotateLeft(Uint n, unsigned rot = 1) noexcept
{
#ifdef BITMANIP_HAS_BUILTIN_ROTL
    return builtin::isconsteval() ? detail::rotateLeft_shift(n, rot) : builtin::rotl(n, rot);
#else
    return detail::rotateLeft_shift(n, rot);
#endif
}

/**
 * @brief Rotates an integer to the right by a given amount of bits.
 * This is similar to a leftshift, but the bits shifted out of the integer are inserted on the high side of the
 * integer.
 * @param n the number to shift
 * @param rot bit count
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint rotateRight(Uint n, unsigned rot = 1) noexcept
{
#ifdef BITMANIP_HAS_BUILTIN_ROTL
    return builtin::isconsteval() ? detail::rotateRight_shift(n, rot) : builtin::rotr(n, rot);
#else
    return detail::rotateRight_shift(n, rot);
#endif
}

}  // namespace bitmanip

#endif  // BITROT_HPP
