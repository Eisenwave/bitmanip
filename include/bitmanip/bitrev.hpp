#ifndef BITMANIP_ENDIAN_HPP
#define BITMANIP_ENDIAN_HPP
/*
 * endian.hpp
 * -----------
 * Provides portable conversions between integers and byte arrays.
 * The encode() and decode() methods are the most notable functions in this header.
 * There are also convenience functions encodeBig(), decodeLittle(), ...
 *
 * Whenever possible, the conversion doesn't take place using bitwise operations but by using std::memcpy and reversing
 * the byte order using builtin::byteSwap.
 */

#include "bit.hpp"
#include "build.hpp"
#include "builtin.hpp"

#include <cstring>

namespace bitmanip {

// PORTABLE BIT/BYTE REVERSAL ==========================================================================================

namespace detail {

template <typename Int>
[[nodiscard]] constexpr Int reverseBytes_naive(Int integer) noexcept
{
    std::uint8_t octets[sizeof(Int)]{};
    for (std::size_t i = 0; i < sizeof(Int); ++i) {
        octets[i] = static_cast<std::uint8_t>(integer >> (i * 8));
    }
    Int result = 0;
    for (std::size_t i = 0; i < sizeof(Int); ++i) {
        std::size_t shift = i * 8;
        result |= Int{octets[sizeof(Int) - i - 1]} << shift;
    }
    return result;
}

template <typename Int>
[[nodiscard]] constexpr Int reverseBits_shift(Int integer, const unsigned bitLimit = 0) noexcept
{
    constexpr unsigned start = log2bits_v<Int>;

    for (unsigned i = start; --i != (bitLimit - 1);) {
        Int lo = integer & ALTERNATING_MASKS<Int>[i];
        Int hi = integer & ~ALTERNATING_MASKS<Int>[i];

        lo <<= 1 << i;
        hi >>= 1 << i;

        integer = lo | hi;
    }

    return integer;
}

template <typename Int>
[[nodiscard]] constexpr Int reverseBytes_shift(Int integer) noexcept
{
    return reverseBits_shift<Int>(integer, 3);
}

}  // namespace detail

/**
 * @brief Reverses the bytes of any integer.
 * @param integer the integer
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint reverseBytes(Uint integer) noexcept
{
    if constexpr (sizeof(Uint) == 1) {
        return integer;
    }
    else {
#ifdef BITMANIP_HAS_BUILTIN_BSWAP
        return builtin::isconsteval() ? detail::reverseBytes_shift(integer) : builtin::bswap(integer);
#else
        return detail::reverseBytes_shift(integer);
#endif
    }
}

/**
 * @brief REverses teh bits of any integer.
 * @param integer the integer
 */
template <BITMANIP_UNSIGNED_TYPENAME(Uint)>
[[nodiscard]] constexpr Uint reverseBits(Uint integer) noexcept
{
#ifdef BITMANIP_HAS_BUILTIN_BITREV
    return builtin::isconsteval() ? detail::reverseBits_shift(integer) : builtin::bitrev(integer);
#endif
    return detail::reverseBits_shift(integer);
}

using build::Endian;

// ENDIAN-CORRECT ENCODE/DECODE ========================================================================================

namespace detail {

template <Endian ENDIAN, typename Int>
[[nodiscard]] constexpr std::enable_if_t<std::is_integral_v<Int>, Int> toNativeEndian(Int integer)
{
    if constexpr (ENDIAN != Endian::NATIVE) {
        using Uint = std::make_unsigned_t<Int>;

        return static_cast<Int>(reverseBytes(static_cast<Uint>(integer)));
    }
    else {
        return integer;
    }
}

}  // namespace detail

template <Endian ENDIAN, typename Int>
[[nodiscard]] inline Int decode(const std::uint8_t buffer[sizeof(Int)])
{
    if constexpr (sizeof(Int) == 1) {
        return static_cast<Int>(buffer[0]);
    }
    else {
        Int result = 0;
        std::memcpy(&result, buffer, sizeof(Int));
        return detail::toNativeEndian<ENDIAN>(result);
    }
}

template <Endian ENDIAN, typename Int>
inline void encode(Int integer, std::uint8_t out[sizeof(Int)])
{
    if constexpr (sizeof(Int) == 1) {
        out[0] = static_cast<std::uint8_t>(integer);
    }
    else {
        integer = detail::toNativeEndian<ENDIAN>(integer);
        std::memcpy(out, &integer, sizeof(Int));
    }
}

// CONVENIENCE FUNCTIONS ===============================================================================================

/**
 * @brief Convenience function that forwards to decode<Endian::LITTLE>.
 */
template <typename Int>
[[nodiscard]] constexpr Int decodeLittle(const std::uint8_t buffer[sizeof(Int)])
{
    return decode<Endian::LITTLE, Int>(buffer);
}

/**
 * @brief Convenience function that forwards to decode<Endian::BIG>.
 */
template <typename Int>
[[nodiscard]] constexpr Int decodeBig(const std::uint8_t buffer[sizeof(Int)])
{
    return decode<Endian::BIG, Int>(buffer);
}

/**
 * @brief Convenience function that forwards to decode<Endian::NATIVE>.
 */
template <typename Int>
[[nodiscard]] constexpr Int decodeNative(const std::uint8_t buffer[sizeof(Int)])
{
    return decode<Endian::NATIVE, Int>(buffer);
}

/**
 * @brief Convenience function that forwards to encode<Endian::LITTLE>.
 */
template <typename Int>
constexpr void encodeLittle(Int integer, std::uint8_t out[])
{
    encode<Endian::LITTLE>(integer, out);
}

/**
 * @brief Convenience function that forwards to encode<Endian::BIG>.
 */
template <typename Int>
constexpr void encodeBig(Int integer, std::uint8_t out[])
{
    encode<Endian::BIG>(integer, out);
}

/**
 * @brief Convenience function that forwards to encode<Endian::NATIVE>.
 */
template <typename Int>
constexpr void encodeNative(Int integer, std::uint8_t out[])
{
    encode<Endian::NATIVE>(integer, out);
}

}  // namespace bitmanip

#endif  // ENDIAN_HPP
