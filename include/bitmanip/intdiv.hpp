#ifndef BITMANIP_INTDIV_HPP
#define BITMANIP_INTDIV_HPP
/*
 * intdiv.hpp
 * -----------
 * Implements ceil, floor, outwards rounding modes for integer division.
 * - cloor is often necessary to get a consistent space downscaling.
 * - ceil is often necessary to get the size of a containing array of data which is not aligned to the container size
 */

#include "bit.hpp"

#include <type_traits>

namespace bitmanip {

namespace detail {

template <typename T>
constexpr signed char divSgn(T n)
{
    if constexpr (std::is_unsigned_v<T>) {
        return 1;
    }
    else {
        return (n > T{0}) - (n < T{0});
    }
};

}  // namespace detail

enum class Rounding {
    /// Towards zero
    TRUNC,
    /// Away from zero
    MAGNIFY,
    /// Towards positive infinity
    CEIL,
    /// Towards negative infinity
    FLOOR,
    /// Towards the closest integer
    ROUND
};

/**
 * @brief Similar to std::common_type_t<A, B>, but if A or B are signed, the result will also be signed.
 *
 * This differs from the regular type promotion rules, where signed types are promoted to unsigned types.
 */
template <typename A, typename B, std::enable_if_t<std::is_integral_v<A> && std::is_integral_v<B>, int> = 0>
using commonSignedType = std::conditional_t<std::is_unsigned_v<A> && std::is_unsigned_v<B>,
                                            std::common_type_t<A, B>,
                                            std::common_type_t<std::make_signed_t<A>, std::make_signed_t<B>>>;

/**
 * Performs a division with the usual truncating behavior of C/C++.
 * However, the result is the common signed type of the dividend and divisor, contrary to the usual unsigned conversion.
 *
 * @param x the dividend
 * @param y the divisor
 * @return trunc(x / y)
 */
template <typename Dividend, typename Divisor>
constexpr commonSignedType<Dividend, Divisor> divTrunc(Dividend x, Divisor y) noexcept
{
    auto cx = static_cast<commonSignedType<Dividend, Divisor>>(x);
    auto cy = static_cast<commonSignedType<Dividend, Divisor>>(y);

    return cx / cy;
}

/**
 * Performs a division but rounds towards positive infinity.
 *
 * @param x the dividend
 * @param y the divisor
 * @return ceil(x / y)
 */
template <typename Dividend, typename Divisor>
constexpr commonSignedType<Dividend, Divisor> divCeil(Dividend x, Divisor y) noexcept
{
    const bool quotientPositive = (x >= 0) == (y >= 0);

    auto cx = static_cast<commonSignedType<Dividend, Divisor>>(x);
    auto cy = static_cast<commonSignedType<Dividend, Divisor>>(y);

    return cx / cy + (cx % cy != 0 && quotientPositive);
}

/**
 * Performs a division but rounds towards negative infinity.
 * For positive numbers, this is equivalent to regular division.
 * For all numbers, this is equivalent to a floating point division and then a floor().
 * Negative numbers will be decremented before division, leading to this type of rounding.
 *
 * Examples:
 * floor(-1/2) = floor(-0.5) = -1
 * floor(-2/2) = floor(-1) = -1
 *
 * This function imitates such behavior but without the use of any floating point arithmetic.
 *
 * @param x the dividend
 * @param y the divisor
 * @return floor(x / y)
 */
template <typename Dividend, typename Divisor>
constexpr commonSignedType<Dividend, Divisor> divFloor(Dividend x, Divisor y) noexcept
{
    const bool quotientNegative = (x >= 0) != (y >= 0);

    auto cx = static_cast<commonSignedType<Dividend, Divisor>>(x);
    auto cy = static_cast<commonSignedType<Dividend, Divisor>>(y);

    return cx / cy - (cx % cy != 0 && quotientNegative);
}

/**
 * Performs a division but rounds away from zero.
 * For positive numbers, this is equivalent to ceiling division and for negative numbers, it is equivalent to flooring
 * division.
 *
 * Examples:
 * divMagnify(1,2)   =  1
 * divMagnify(-5,10) = -1
 *
 * @param x the dividend
 * @param y the divisor
 * @param up(x / y)
 */
template <typename Dividend, typename Divisor>
constexpr commonSignedType<Dividend, Divisor> divMagnify(Dividend x, Divisor y) noexcept
{
    signed char quotientSgn = detail::divSgn(x) * detail::divSgn(y);

    auto cx = static_cast<commonSignedType<Dividend, Divisor>>(x);
    auto cy = static_cast<commonSignedType<Dividend, Divisor>>(y);

    return cx / cy + (cx % cy != 0) * quotientSgn;
}

constexpr Rounding DEFAULT_ROUND_TIE_BREAK = Rounding::MAGNIFY;

/**
 * Performs a division but rounds to the closest number. Ties (halfway cases) are broken with a rounding mode of choice.
 * UP and TRUNC are supported, where UP is the default, which matches the semantics of std::round.
 *
 * This means that by default, haflway cases like 0.5, -0.5 would be rounded to 1, -1 respectively.
 *
 * Examples:
 * divRound(1,2)   =  1
 * divRound(-5,10) = -1
 *
 * @tparam TIE_BREAK the rounding mode for ties (halfway cases); limited to TRUNC and UP
 * @param x the dividend
 * @param y the divisor
 * @param round(x / y)
 */
template <Rounding TIE_BREAK = DEFAULT_ROUND_TIE_BREAK, typename Dividend, typename Divisor>
constexpr commonSignedType<Dividend, Divisor> divRound(Dividend x, Divisor y) noexcept
{
    signed char sgnX = x < 0 ? -1 : 1;
    signed char sgnY = y < 0 ? -1 : 1;
    signed char sgnQ = sgnX * sgnY;

    auto cx = static_cast<commonSignedType<Dividend, Divisor>>(x);
    auto cy = static_cast<commonSignedType<Dividend, Divisor>>(y);

    auto absRemainder = cx % cy * sgnX;
    auto absHalfDvsor = (cy >> 1) * sgnY;

    bool increment = false;
    if constexpr (TIE_BREAK == Rounding::TRUNC) {
        increment = absRemainder > absHalfDvsor;
    }
    else if (TIE_BREAK == Rounding::MAGNIFY) {
        increment = absRemainder >= absHalfDvsor + (cy & 1);
    }
    static_assert(TIE_BREAK == Rounding::TRUNC || TIE_BREAK == Rounding::MAGNIFY,
                  "Only TRUNC and UP rounding modes are allowed as tie breakers");

    return cx / cy + increment * sgnQ;
}

/**
 * Performs a division with a rounding mode of choice.
 *
 * @tparam ROUND the rounding mode
 * @tparam TIE_BREAK the tie break for ROUND rounding, see divRound for limitations
 * @param x the dividend
 * @param y the divisor
 * @return (x / y), rounded with the chosen mode and tie break
 */
template <Rounding ROUND, Rounding TIE_BREAK = DEFAULT_ROUND_TIE_BREAK, typename Dividend, typename Divisor>
constexpr commonSignedType<Dividend, Divisor> div(Dividend x, Divisor y) noexcept
{
    if constexpr (ROUND == Rounding::TRUNC) {
        return divTrunc(x, y);
    }
    else if constexpr (ROUND == Rounding::CEIL) {
        return divCeil(x, y);
    }
    else if constexpr (ROUND == Rounding::FLOOR) {
        return divFloor(x, y);
    }
    else if constexpr (ROUND == Rounding::MAGNIFY) {
        return divMagnify(x, y);
    }
    else if constexpr (ROUND == Rounding::ROUND) {
        return divRound<TIE_BREAK>(x, y);
    }
}

// UNSIGNED REMAINDER (MODULUS) ========================================================================================

template <typename Int, typename Uint, std::enable_if_t<(std::is_integral_v<Int> && std::is_unsigned_v<Uint>), int> = 0>
constexpr Uint umod(Int n, Uint mod) noexcept
{
    if constexpr (std::is_unsigned_v<Int>) {
        return n % mod;
    }
    else {
        Uint rem = n % static_cast<Int>(mod);
        return static_cast<Uint>(rem) + (mod & signFill(rem));
    }
}

// MIDPOINT ============================================================================================================

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint midpointFloor(Uint x, Uint y)
{
    Uint sum = x + y;
    return sum >> Uint{1} | makeHighest<Uint>(sum < x);
}

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint midpointCeil(Uint x, Uint y)
{
    return midpointFloor(x, y) + ((x + y) & Uint{1});
}

template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
constexpr Uint midpointToLeft(Uint x, Uint y)
{
    return midpointFloor(x, y) + ((x + y) & Uint{1} & Uint{x > y});
}

}  // namespace bitmanip

#endif  // INTDIV_HPP
