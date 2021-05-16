#ifndef ILEAVE_HPP
#define ILEAVE_HPP
/*
 * ileave.hpp
 * -----------
 * Provides bit-interleaving and bit-de-interleaving functionality.
 * Each function typically has multiple implementations in detail:: namespaces, which are then chosen in a single
 * function in bitmanip::
 */

#include "bit.hpp"
#include "intdiv.hpp"
#include "intlog.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace bitmanip {
namespace detail {

// BIT DUPLICATION =====================================================================================================

/**
 * @brief Duplicates each input bit <bits> times. Example: 0b101 -> 0b110011
 * @param input the input number
 * @param out_bits_per_in_bits the output bits per input bits
 * @return the output number or 0 if the bits parameter was zero
 */
[[nodiscard]] constexpr std::uint64_t duplBits_naive(std::uint64_t input, std::size_t out_bits_per_in_bits) noexcept
{
    if (out_bits_per_in_bits == 0) {
        return 0;
    }
    const auto lim = bitmanip::divCeil<std::size_t>(64, out_bits_per_in_bits);

    std::uint64_t result = 0;
    for (std::size_t i = 0, b_out = 0; i < lim; ++i) {
        for (std::size_t j = 0; j < out_bits_per_in_bits; ++j, ++b_out) {
            result |= static_cast<std::uint64_t>((input >> i) & 1) << b_out;
        }
    }

    return result;
}

// ZERO-BIT INTERLEAVING ===============================================================================================

/**
 * @brief Interleaves an input number with <bits> zero-bits per input bit. Example: 0b11 -> 0b0101
 * @param input the input number
 * @param bits the amount of zero bits per input bit to interleave
 * @return the input number interleaved with input-bits
 */
[[nodiscard]] constexpr std::uint64_t ileaveZeros_naive(std::uint32_t input, std::size_t bits) noexcept
{
    const auto lim = std::min<std::size_t>(32, bitmanip::divCeil<std::size_t>(64, bits + 1));

    std::uint64_t result = 0;
    for (std::size_t i = 0, b_out = 0; i < lim; ++i) {
        result |= static_cast<std::uint64_t>(input & 1) << b_out;
        input >>= 1;
        b_out += bits + 1;
    }

    return result;
}

template <unsigned BITS>
[[nodiscard]] constexpr std::uint64_t ileaveZeros_shift(std::uint32_t input) noexcept
{
    if constexpr (BITS == 0) {
        return input;
    }
    else {
        constexpr std::uint64_t MASKS[] = {
            detail::duplBits_naive(detail::ileaveZeros_naive(~std::uint32_t(0), BITS), 1),
            detail::duplBits_naive(detail::ileaveZeros_naive(~std::uint32_t(0), BITS), 2),
            detail::duplBits_naive(detail::ileaveZeros_naive(~std::uint32_t(0), BITS), 4),
            detail::duplBits_naive(detail::ileaveZeros_naive(~std::uint32_t(0), BITS), 8),
            detail::duplBits_naive(detail::ileaveZeros_naive(~std::uint32_t(0), BITS), 16),
        };
        // log2_floor(0) == 0 so this is always safe, even for 1 bit
        constexpr int start = 4 - static_cast<int>(bitmanip::log2floor(BITS >> 1));

        std::uint64_t n = input;
        for (int i = start; i != -1; --i) {
            unsigned shift = BITS * (1u << i);
            n |= n << shift;
            n &= MASKS[i];
        }

        return n;
    }
}

#ifdef BITMANIP_HAS_BUILTIN_PDEP
#define BITMANIP_HAS_BUILTIN_ILEAVE_ZEROS
template <unsigned BITS, unsigned SHIFT = 0>
[[nodiscard]] inline std::uint64_t ileaveZeros_builtin(std::uint32_t input) noexcept
{
    constexpr std::uint64_t mask = detail::ileaveZeros_naive(~std::uint32_t(0), BITS) << SHIFT;
    return builtin::depositBits(std::uint64_t{input}, mask);
}
#endif

}  // namespace detail

/**
 * @brief Interleaves BITS zero-bits inbetween each input bit and optionally leftshifts the result by SHIFT bits.
 *
 * SHIFT is an additional parameter because left-shifting is a no-op when the builtin implementation is available.
 * @param input the input number
 * @tparam BITS the number of bits to be interleaved or zero for an identity mapping
 * @tparam SHIFT the number of bits to left-shift the input by after interleaving zeros
 * @return the input interleaved with <BITS> bits
 */
template <unsigned BITS, unsigned SHIFT = 0>
[[nodiscard]] constexpr std::uint64_t ileaveZeros_const(std::uint32_t input) noexcept
{
#ifdef BITMANIP_HAS_BUILTIN_ILEAVE_ZEROS
    if (not isConstantEvaluated()) {
        return detail::ileaveZeros_builtin<BITS, SHIFT>(input);
    }
#endif
    return detail::ileaveZeros_shift<BITS>(input) << SHIFT;
}

// BITWISE DE-INTERLEAVING =============================================================================================

namespace detail {

/**
 * @brief Removes each interleaved <bits> bits. Example: 0b010101 --rem 1--> 0b111
 * @param input the input number
 * @param bits input bits per output bit
 * @return the the output with removed bits
 */
[[nodiscard]] constexpr std::uint64_t remIleavedBits_naive(std::uint64_t input, std::size_t bits) noexcept
{
    // increment once to avoid modulo divisions by 0
    // this way our function is noexcept and safe for all inputs
    ++bits;
    std::uint64_t result = 0;
    for (std::size_t i = 0, b_out = 0; i < 64; ++i) {
        if (i % bits == 0) {
            result |= (input & 1) << b_out++;
        }
        input >>= 1;
    }

    return result;
}

template <unsigned BITS>
[[nodiscard]] constexpr std::uint64_t remIleavedBits_shift(std::uint64_t input) noexcept
{
    if constexpr (BITS == 0) {
        return input;
    }
    else {
        constexpr std::uint64_t MASKS[] = {
            detail::duplBits_naive(detail::ileaveZeros_naive(~std::uint32_t(0), BITS), 1),
            detail::duplBits_naive(detail::ileaveZeros_naive(~std::uint32_t(0), BITS), 2),
            detail::duplBits_naive(detail::ileaveZeros_naive(~std::uint32_t(0), BITS), 4),
            detail::duplBits_naive(detail::ileaveZeros_naive(~std::uint32_t(0), BITS), 8),
            detail::duplBits_naive(detail::ileaveZeros_naive(~std::uint32_t(0), BITS), 16),
            detail::duplBits_naive(detail::ileaveZeros_naive(~std::uint32_t(0), BITS), 32),
        };
        // log2_floor(0) == 0 so this is always safe, even for 1 bit
        constexpr std::size_t iterations = 5 - bitmanip::log2floor(BITS >> 1);

        input &= MASKS[0];

        for (std::size_t i = 0; i < iterations; ++i) {
            unsigned rshift = (1u << i) * BITS;
            input |= input >> rshift;
            input &= MASKS[i + 1];
        }

        return input;
    }
}

#ifdef BITMANIP_HAS_BUILTIN_PEXT
#define BITMANIP_HAS_BUILTIN_REM_ILEAVED_BITS
template <unsigned BITS, unsigned SHIFT>
inline std::uint64_t remIleavedBits_builtin(std::uint64_t input) noexcept
{
    constexpr std::uint64_t mask = detail::ileaveZeros_naive(~std::uint32_t(0), BITS) << SHIFT;
    return builtin::extractBits(std::uint64_t{input}, mask);
}
#endif

}  // namespace detail

/**
 * @brief Removes each interleaved <BITS> bits. Example: 0b010101 --rem 1--> 0b111
 * If BITS is zero, no bits are removed and the input is returned.
 * @param input the input number
 * @param bits input bits per output bit
 * @return the the output with removed bits
 */
template <unsigned BITS, unsigned SHIFT = 0>
[[nodiscard]] constexpr std::uint64_t remIleavedBits_const(std::uint64_t input) noexcept
{
#ifdef BITMANIP_HAS_BUILTIN_REM_ILEAVED_BITS
    if (not isConstantEvaluated()) {
        return detail::remIleavedBits_builtin<BITS, SHIFT>(input);
    }
#endif
    return detail::remIleavedBits_shift<BITS>(input >> SHIFT);
}

// NUMBER INTERLEAVING =================================================================================================

namespace detail {

[[nodiscard]] constexpr std::uint64_t ileave3_naive(std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept
{
    return (detail::ileaveZeros_naive(x, 2) << 2) | (detail::ileaveZeros_naive(y, 2) << 1) | ileaveZeros_naive(z, 2);
}

}  // namespace detail

[[nodiscard]] constexpr std::uint64_t ileave2(std::uint32_t hi, std::uint32_t lo) noexcept
{
    return ileaveZeros_const<1, 1>(hi) | ileaveZeros_const<1, 0>(lo);
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
[[nodiscard]] constexpr std::uint64_t ileave3(std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept
{
    return ileaveZeros_const<2, 2>(x) | ileaveZeros_const<2, 1>(y) | ileaveZeros_const<2, 0>(z);
}

// NUMBER DE-INTERLEAVING ==============================================================================================

namespace detail {

constexpr void dileave3_naive(std::uint64_t n, std::uint32_t out[3]) noexcept
{
    out[0] = static_cast<std::uint32_t>(detail::remIleavedBits_naive(n >> 2, 2));
    out[1] = static_cast<std::uint32_t>(detail::remIleavedBits_naive(n >> 1, 2));
    out[2] = static_cast<std::uint32_t>(detail::remIleavedBits_naive(n >> 0, 2));
}

}  // namespace detail

/**
 * @brief Deinterleaves 3 integers which are interleaved in a single number.
 * Visualization: abcdefghi -> (adg, beh, cfi)
 *
 * This is also referred to as a Morton Code in scientific literature.
 *
 * @param n the number
 * @return the interleaved bits
 */
constexpr void dileave3(std::uint64_t n, std::uint32_t out[3]) noexcept
{
    out[0] = static_cast<std::uint32_t>(remIleavedBits_const<2, 2>(n));
    out[1] = static_cast<std::uint32_t>(remIleavedBits_const<2, 1>(n));
    out[2] = static_cast<std::uint32_t>(remIleavedBits_const<2, 0>(n));
}

// BYTE INTERLEAVING ===================================================================================================

/**
 * @brief Interleaves up to 8 bytes into a 64-bit integer.
 * @param bytes the bytes in little-endian order
 * @tparam COUNT the number of bytes to interleave
 * @return the interleaved bytes stored in a 64-bit integer
 */
template <std::size_t COUNT, std::enable_if_t<COUNT <= 8, int> = 0>
[[nodiscard]] constexpr std::uint64_t ileaveBytes_const(std::uint64_t bytes) noexcept
{
    std::uint64_t result = 0;
    // use if-constexpr to avoid instantiation of ileave_zeros
    if constexpr (COUNT != 0) {
        for (std::size_t i = 0; i < COUNT; ++i) {
            result |= ileaveZeros_const<COUNT - 1>(bytes & 0xff) << i;
            bytes >>= 8;
        }
    }
    return result;
}

namespace detail {

// alternative implementation using a naive algorithm
[[nodiscard]] constexpr std::uint64_t ileaveBytes_naive(std::uint64_t bytes, std::size_t count) noexcept
{
    BITMANIP_ASSUME(count <= 8);

    std::uint64_t result = 0;
    for (std::size_t i = 0; i < count; ++i) {
        result |= ileaveZeros_naive(bytes & 0xff, count - 1) << i;
        bytes >>= 8;
    }
    return result;
}

// alternative implementation adapting ileave_bytes_const to work with a runtime parameter
[[nodiscard]] constexpr std::uint64_t ileaveBytes_jmp(std::uint64_t bytes, std::size_t count) noexcept
{
    BITMANIP_ASSUME(count <= 8);

    switch (count) {
    case 0: return ileaveBytes_const<0>(bytes);
    case 1: return ileaveBytes_const<1>(bytes);
    case 2: return ileaveBytes_const<2>(bytes);
    case 3: return ileaveBytes_const<3>(bytes);
    case 4: return ileaveBytes_const<4>(bytes);
    case 5: return ileaveBytes_const<5>(bytes);
    case 6: return ileaveBytes_const<6>(bytes);
    case 7: return ileaveBytes_const<7>(bytes);
    case 8: return ileaveBytes_const<8>(bytes);
    }
    BITMANIP_UNREACHABLE();
}

}  // namespace detail

/**
 * @brief Interleaves up to 8 bytes into a 64-bit integer.
 * @param bytes the bytes in little-endian order
 * @param count number of bytes to interleave
 * @return the interleaved bytes stored in a 64-bit integer
 */
[[nodiscard]] constexpr std::uint64_t ileaveBytes(std::uint64_t bytes, std::size_t count) noexcept
{
    return detail::ileaveBytes_jmp(bytes, count);
}

// BYTE DE-INTERLEAVING ================================================================================================

template <std::size_t COUNT, std::enable_if_t<COUNT <= 8, int> = 0>
[[nodiscard]] constexpr std::uint64_t dileaveBytes_const(std::uint64_t ileaved) noexcept
{
    std::uint64_t result = 0;
    // use if-constexpr to avoid instantiation of rem_ileaved_bits
    if constexpr (COUNT != 0) {
        for (std::size_t i = COUNT; i != 0; --i) {
            // if we also masked the result with 0xff, then this would be safe for a hi-polluted ileaved number
            result <<= 8;
            result |= remIleavedBits_const<COUNT - 1>(ileaved >> (i - 1));
        }
    }
    return result;
}

namespace detail {

// alternative implementation adapting ileave_bytes_const to work with a runtime parameter
[[nodiscard]] constexpr std::uint64_t dileaveBytes_jmp(std::uint64_t bytes, std::size_t count) noexcept
{
    BITMANIP_ASSUME(count <= 8u);

    switch (count) {
    case 0: return dileaveBytes_const<0>(bytes);
    case 1: return dileaveBytes_const<1>(bytes);
    case 2: return dileaveBytes_const<2>(bytes);
    case 3: return dileaveBytes_const<3>(bytes);
    case 4: return dileaveBytes_const<4>(bytes);
    case 5: return dileaveBytes_const<5>(bytes);
    case 6: return dileaveBytes_const<6>(bytes);
    case 7: return dileaveBytes_const<7>(bytes);
    case 8: return dileaveBytes_const<8>(bytes);
    }
    BITMANIP_UNREACHABLE();
}

}  // namespace detail

[[nodiscard]] constexpr std::uint64_t dileave_bytes(std::uint64_t bytes, std::size_t count) noexcept
{
    return detail::dileaveBytes_jmp(bytes, count);
}

}  // namespace bitmanip

#endif  // ILEAVE_HPP
