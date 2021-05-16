#ifndef BITMANIP_BUILTIN_HPP
#define BITMANIP_BUILTIN_HPP
/*
 * builtin.hpp
 * -----------
 * Forwards to builtin functions in a compiler-independent way.
 */

#include "build.hpp"

#ifdef BITMANIP_MSVC
#include <intrin.h>
#endif

#if defined(BITMANIP_X86_OR_X64) && defined(BITMANIP_GNU_OR_CLANG)
#include <immintrin.h>
#endif

#include <cstdint>
#include <type_traits>

// BITMANIP_HAS_BUILTIN(builtin):
//     Checks whether a builtin exists.
//     This only works on recent versions of clang, as this macro does not exist in gcc.
//     For gcc, the result is always 1 (true), optimistically assuming that the builtin exists.
#ifdef __has_builtin
#define BITMANIP_HAS_BUILTIN_HAS_BUILTIN
#define BITMANIP_HAS_BUILTIN(builtin) __has_builtin(builtin)
#else
#define BITMANIP_HAS_BUILTIN(builtin) 1
#endif

// METAPROGRAMMING =====================================================================================================

// BITMANIP_UNREACHABLE():
//     Signals to the compiler that a given code point is unreachable.
//     This macro is always defined, but does nothing if no builtin is available.
#if defined(BITMANIP_GNU_OR_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_unreachable)
#define BITMANIP_HAS_BUILTIN_UNREACHABLE
#define BITMANIP_UNREACHABLE() __builtin_unreachable()

#elif defined(BITMANIP_MSVC)
#define BITMANIP_HAS_BUILTIN_UNREACHABLE
#define BITMANIP_UNREACHABLE() __assume(false)

#else
#define BITMANIP_UNREACHABLE()
#endif

// BITMANIP_ASSUME():
//     Signals to the compiler that a given expression always evaluates to true.
//     This macro is always defined, but does nothing if no builtin is available.
//     Note that the expression inside of BITMANIP_ASSUME() is evaluated, although the result is unused.
//     This makes it unsafe to use when the expression has side effects.
#if defined(BITMANIP_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_assume)
#define BITMANIP_HAS_BUILTIN_ASSUME
#define BITMANIP_ASSUME(condition) __builtin_assume(condition)

#elif defined(BITMANIP_GNU)
#define BITMANIP_HAS_BUILTIN_ASSUME
#define BITMANIP_ASSUME(condition) (static_cast<bool>(condition) ? void(0) : __builtin_unreachable())

#elif defined(BITMANIP_MSVC)
#define BITMANIP_HAS_BUILTIN_ASSUME
#define BITMANIP_ASSUME(condition) __assume(condition)

#else
#define BITMANIP_ASSUME(condition)
#endif

namespace bitmanip::builtin {

// void trap():
//     Executes an abnormal instruction or otherwise exits the program immediately.
//     This bypasses even std::terminate() and is completely unhandled.
//     This function always exists and will call std::terminate() by default.
#if defined(BITMANIP_GNU_OR_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_trap)
#define BITMANIP_HAS_BUILTIN_TRAP
[[noreturn]] inline void trap()
{
    __builtin_trap();
}
#else
[[noreturn]] void trap();
#endif

// bool isconsteval():
//     To be used in an if-statement to verify whether the current context is constant-evaluated.
//     This builtin can potentially not exist and it has no sane default.
//     clang 9+ and gcc 9+ support this builtin, even for C++17, allowing for implementation of
//     std::is_constant_evaluated() before C++20.
// clang-format off
#if defined(BITMANIP_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_is_constant_evaluated) || \
    defined(BITMANIP_GNU) && BITMANIP_GNU >= 9 || \
    defined(BITMANIP_MSVC) && BITMANIP_MSVC >= 1925
// clang-format on
#define BITMANIP_HAS_BUILTIN_ISCONSTEVAL
constexpr bool isconsteval() noexcept
{
    return __builtin_is_constant_evaluated();
}
#else
constexpr bool isconsteval() noexcept
{
    return true;
}
#endif

// To bitcast<To, From>(const From &from):
//     Wrapper for __builtin_bit_cast.
//     This builtin should behave identically to std::bit_cast from the <bit> header.
// clang-format off
#if defined(BITMANIP_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_bit_cast) || \
    defined(BITMANIP_GNU) && BITMANIP_GNU >= 11 || \
    defined(BITMANIP_MSVC) && BITMANIP_MSVC >= 1926
// clang-format on
template <typename To, typename From>
constexpr bool isbitcastable_ =
    sizeof(To) == sizeof(From) && std::is_trivially_copyable_v<From> &&std::is_trivially_copyable_v<To>;

template <typename To, typename From>
constexpr std::enable_if_t<isbitcastable_<To, From>, To> bitcast(const From &from)
{
    return __builtin_bit_cast(To, from);
}
#endif

// BIT COUNTING ========================================================================================================

// int clrsb(unsigned ...):
//     Counts the number of redundant sign bits, i.o.w. the number of bits following the sign bit which are equal to it.
//     This is the number of leading zeros minus one for positive numbers.
//     For negative numbers, it is the number of leading ones - 1.
#if defined(BITMANIP_GNU_OR_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_clrsb) && \
    BITMANIP_HAS_BUILTIN(__builtin_clrsbl) && BITMANIP_HAS_BUILTIN(__builtin_clrsbll)
#define BITMANIP_HAS_BUILTIN_CLRSB
inline int clrsb(char x) noexcept
{
    return __builtin_clrsb(x);
}

inline int clrsb(short x) noexcept
{
    return __builtin_clrsb(x);
}

inline int clrsb(int x) noexcept
{
    return __builtin_clrsb(x);
}

inline int clrsb(long x) noexcept
{
    return __builtin_clrsbl(x);
}

inline int clrsb(long long x) noexcept
{
    return __builtin_clrsbll(x);
}
#endif

// int clz(unsigned ...):
//     Counts the number of leading zeros in an unsigned integer type.
//     The result of countLeadingZeros(0) is undefined for all types.
#if defined(BITMANIP_GNU_OR_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_clz) && BITMANIP_HAS_BUILTIN(__builtin_clzl) && \
    BITMANIP_HAS_BUILTIN(__builtin_clzll)
#define BITMANIP_HAS_BUILTIN_CLZ
inline int clz(unsigned char x) noexcept
{
    return __builtin_clz(x) - int{(sizeof(int) - sizeof(unsigned char)) * 8};
}

inline int clz(unsigned short x) noexcept
{
    return __builtin_clz(x) - int{(sizeof(int) - sizeof(unsigned short)) * 8};
}

inline int clz(unsigned int x) noexcept
{
    return __builtin_clz(x);
}

inline int clz(unsigned long x) noexcept
{
    return __builtin_clzl(x);
}

inline int clz(unsigned long long x) noexcept
{
    return __builtin_clzll(x);
}

#elif defined(BITMANIP_MSVC) && defined(BITMANIP_X86_OR_X64)
#define BITMANIP_HAS_BUILTIN_CLZ
__forceinline int clz(unsigned char x) noexcept
{
    return __lzcnt16(x) - 8;
}

__forceinline int clz(unsigned short x) noexcept
{
    return __lzcnt16(x);
}

__forceinline int clz(unsigned int x) noexcept
{
    return __lzcnt(x);
}

__forceinline int clz(unsigned long x) noexcept
{
    static_assert(sizeof(unsigned long) == sizeof(unsigned) || sizeof(unsigned long) == sizeof(uint64_t));

    if constexpr (sizeof(unsigned long) == sizeof(unsigned)) {
        return __lzcnt(static_cast<unsigned>(x));
    }
    else {
        return __lzcnt64(static_cast<uint64_t>(x));
    }
}

#ifdef BITMANIP_64_BIT
__forceinline int clz(uint64_t x) noexcept
{
    return __lzcnt64(x);
}
#else
[[noreturn]] __forceinline int clz(uint64_t) noexcept
{
    trap();
}
#endif
#endif

// int ctz(unsigned ...):
//     Counts the number of trailing zeros in an unsigned integer type.
//     The result of countTrailingZeros(0) is undefined for all types.
#if defined(BITMANIP_GNU_OR_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_ctz) && BITMANIP_HAS_BUILTIN(__builtin_ctzl) && \
    BITMANIP_HAS_BUILTIN(__builtin_ctzll)
#define BITMANIP_HAS_BUILTIN_CTZ
inline int ctz(unsigned char x) noexcept
{
    return __builtin_ctz(x);
}

inline int ctz(unsigned short x) noexcept
{
    return __builtin_ctz(x);
}

inline int ctz(unsigned int x) noexcept
{
    return __builtin_ctz(x);
}

inline int ctz(unsigned long x) noexcept
{
    return __builtin_ctzl(x);
}

inline int ctz(unsigned long long x) noexcept
{
    return __builtin_ctzll(x);
}
#endif

// int lsb(unsigned ...):
//     Returns the index of the least significant bit.
//     For example, leastSignificantBit(0b110) -> 1.
//     The result of leastSignificantBit(0) is 0 for all types.
#ifdef BITMANIP_MSVC
#define BITMANIP_HAS_BUILTIN_LSB
__forceinline int lsb(unsigned char x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanForward(&result, x);
    return nonzero * static_cast<int>(result);
}

__forceinline int lsb(unsigned short x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanForward(&result, x);
    return nonzero * static_cast<int>(result);
}

__forceinline int lsb(unsigned int x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanForward(&result, x);
    return nonzero * static_cast<int>(result);
}

__forceinline int lsb(unsigned long x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanForward(&result, x);
    return nonzero * static_cast<int>(result);
}

#ifdef BITMANIP_64_BIT
__forceinline int lsb(uint64_t x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanForward64(&result, x);
    return nonzero * static_cast<int>(result);
}
#else
[[noreturn]] __forceinline int lsb(uint64_t) noexcept
{
    trap();
}
#endif
#endif

// int msb(unsigned ...):
//     Returns the index of the most significant bit.
//     For example, mostSignificantBit(0b110) -> 2.
//     The result of mostSignificantBit(0) is 0 for all types.
#ifdef BITMANIP_MSVC
#define BITMANIP_HAS_BUILTIN_MSB
__forceinline int msb(unsigned char x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanReverse(&result, x);
    return nonzero * static_cast<int>(result);
}

__forceinline int msb(unsigned short x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanReverse(&result, x);
    return nonzero * static_cast<int>(result);
}

__forceinline int msb(unsigned int x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanReverse(&result, x);
    return nonzero * static_cast<int>(result);
}

__forceinline int msb(unsigned long x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanReverse(&result, x);
    return nonzero * static_cast<int>(result);
}

#ifdef BITMANIP_64_BIT
__forceinline int msb(uint64_t x) noexcept
{
    unsigned long result;
    bool nonzero = _BitScanReverse64(&result, x);
    return nonzero * static_cast<int>(result);
}
#else
[[noreturn]] __forceinline int msb(uint64_t) noexcept
{
    trap();
}
#endif
#endif

// int ffs(unsigned ...):
//     Returns one plus the number of trailing zeros.
//     findFirstSet(0) evaluates to zero.
#if defined(BITMANIP_GNU_OR_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_ffs) && BITMANIP_HAS_BUILTIN(__builtin_ffsl) && \
    BITMANIP_HAS_BUILTIN(__builtin_ffsll)
#define BITMANIP_HAS_BUILTIN_FFS
inline int ffs(unsigned char x) noexcept
{
    return __builtin_ffs(x);
}

inline int ffs(unsigned short x) noexcept
{
    return __builtin_ffs(x);
}

inline int ffs(unsigned int x) noexcept
{
    return __builtin_ffs(static_cast<int>(x));
}

inline int ffs(unsigned long x) noexcept
{
    return __builtin_ffsl(static_cast<long>(x));
}

inline int ffs(unsigned long long x) noexcept
{
    return __builtin_ffsll(static_cast<long long>(x));
}
#endif

// int popcount(unsigned ...):
//     Counts the number of one-bits in an unsigned integer type.
#if defined(BITMANIP_GNU_OR_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_popcount) && \
    BITMANIP_HAS_BUILTIN(__builtin_popcountl) && BITMANIP_HAS_BUILTIN(__builtin_popcountll)
#define BITMANIP_HAS_BUILTIN_POPCOUNT
inline int popcount(unsigned char x) noexcept
{
    return __builtin_popcount(x);
}

inline int popcount(unsigned short x) noexcept
{
    return __builtin_popcount(x);
}

inline int popcount(unsigned int x) noexcept
{
    return __builtin_popcount(x);
}

inline int popcount(unsigned long x) noexcept
{
    return __builtin_popcountl(x);
}

inline int popcount(unsigned long long x) noexcept
{
    return __builtin_popcountll(x);
}
#elif defined(BITMANIP_MSVC)
#define BITMANIP_HAS_BUILTIN_POPCOUNT
__forceinline int popcount(uint8_t x) noexcept
{
    return static_cast<int>(__popcnt16(x));
}

__forceinline int popcount(uint16_t x) noexcept
{
    return static_cast<int>(__popcnt16(x));
}

__forceinline int popcount(uint32_t x) noexcept
{
    return static_cast<int>(__popcnt(x));
}

#ifdef BITMANIP_64_BIT
__forceinline int popcount(uint64_t x) noexcept
{
    return static_cast<int>(__popcnt64(x));
}
#else
[[noreturn]] __forceinline int popcount(uint64_t) noexcept
{
    trap();
}
#endif
#endif

// int parity(unsigned ...):
//     Returns the parity of a number.
//     This is a bool which indicates whether the number of set bits in x is odd.
//     The parity of 0 is 0, the parity of 1 is 1.
#if defined(BITMANIP_GNU_OR_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_parity) && \
    BITMANIP_HAS_BUILTIN(__builtin_parityl) && BITMANIP_HAS_BUILTIN(__builtin_parityll)
#define BITMANIP_HAS_BUILTIN_PARITY
inline bool parity(unsigned char x) noexcept
{
    return __builtin_parity(x);
}

inline bool parity(unsigned short x) noexcept
{
    return __builtin_parity(x);
}

inline bool parity(unsigned int x) noexcept
{
    return __builtin_parity(x);
}

inline bool parity(unsigned long x) noexcept
{
    return __builtin_parityl(x);
}

inline bool parity(unsigned long long x) noexcept
{
    return __builtin_parityll(x);
}
#endif

// ADVANCED BITWISE OPS ================================================================================================

// unsigned rotr(unsigned ...):
//     Right-rotates the bits of a number.
#if defined(BITMANIP_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_rotateright8) && \
    BITMANIP_HAS_BUILTIN(__builtin_rotateright64)
#define BITMANIP_HAS_BUILTIN_ROTR
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint> && (sizeof(Uint) <= 8), int> = 0>
Uint rotr(Uint x, unsigned char rot) noexcept
{
    if constexpr (sizeof(x) == 1) return __builtin_rotateright8(static_cast<uint8_t>(x), rot);
    if constexpr (sizeof(x) == 2) return __builtin_rotateright16(static_cast<uint16_t>(x), rot);
    if constexpr (sizeof(x) == 4) return __builtin_rotateright32(static_cast<uint32_t>(x), rot);
    if constexpr (sizeof(x) == 8) return __builtin_rotateright64(static_cast<uint64_t>(x), rot);
}
#endif

// unsigned rotl(unsigned ...):
//     Left-rotates the bits of a number.
#if defined(BITMANIP_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_rotateleft8) && \
    BITMANIP_HAS_BUILTIN(__builtin_rotateleft64)
#define BITMANIP_HAS_BUILTIN_ROTL
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint> && (sizeof(Uint) <= 8), int> = 0>
Uint rotl(Uint x, unsigned char rot) noexcept
{
    if constexpr (sizeof(x) == 1) return __builtin_rotateleft8(static_cast<uint8_t>(x), rot);
    if constexpr (sizeof(x) == 2) return __builtin_rotateleft16(static_cast<uint16_t>(x), rot);
    if constexpr (sizeof(x) == 4) return __builtin_rotateleft32(static_cast<uint32_t>(x), rot);
    if constexpr (sizeof(x) == 8) return __builtin_rotateleft64(static_cast<uint64_t>(x), rot);
}
#endif

// uintXX_t bswap(uintXX_t ...):
//     Swaps the bytes of any uintXX type.
//     This reverses the byte order (little-endian/big-endian).
//     This does nothing for byteSwap(uint8_t).
#if defined(BITMANIP_GNU_OR_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_bswap16) && \
    BITMANIP_HAS_BUILTIN(__builtin_bswap32) && BITMANIP_HAS_BUILTIN(__builtin_bswap64)
#define BITMANIP_HAS_BUILTIN_BSWAP
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint> && (sizeof(Uint) <= 8), int> = 0>
Uint bswap(Uint x) noexcept
{
    if constexpr (sizeof(x) == 1) return x;
    if constexpr (sizeof(x) == 2) return __builtin_bswap16(x);
    if constexpr (sizeof(x) == 4) return __builtin_bswap32(x);
    if constexpr (sizeof(x) == 8) return __builtin_bswap64(x);
}

#elif defined(BITMANIP_MSVC)
#define BITMANIP_HAS_BUILTIN_BSWAP
__forceinline uint8_t bswap(uint8_t val) noexcept
{
    return val;
}

__forceinline unsigned short bswap(unsigned short val) noexcept
{
    return _byteswap_ushort(val);
}

__forceinline unsigned int bswap(unsigned int val) noexcept
{
    static_assert(sizeof(unsigned int) == sizeof(unsigned short) || sizeof(unsigned int) == sizeof(unsigned long),
                  "No viable _byteswap implementation for unsigned int");
    if constexpr (sizeof(unsigned int) == sizeof(unsigned short)) {
        return static_cast<unsigned int>(_byteswap_ushort(static_cast<unsigned short>(val)));
    }
    else {
        return static_cast<unsigned int>(_byteswap_ulong(static_cast<unsigned long>(val)));
    }
}

__forceinline unsigned long bswap(unsigned long val) noexcept
{
    return _byteswap_ulong(val);
}

#ifdef BITMANIP_64_BIT
__forceinline uint64_t bswap(uint64_t val) noexcept
{
    return _byteswap_uint64(val);
}
#else
[[noreturn]] __forceinline uint64_t bswap(uint64_t) noexcept
{
    trap();
}
#endif
#endif

// uintXX_t bitrev(uintXX_t ...):
//     Reverse the order of bits in an integer
#if defined(BITMANIP_CLANG) && BITMANIP_HAS_BUILTIN(__builtin_bitreverse64)
#define BITMANIP_HAS_BUILTIN_BITREV
template <typename Uint, std::enable_if_t<std::is_unsigned_v<Uint> && (sizeof(Uint) <= 8), int> = 0>
Uint bitrev(Uint x) noexcept
{
    if constexpr (sizeof(x) == 1) return __builtin_bitreverse8(x);
    if constexpr (sizeof(x) == 2) return __builtin_bitreverse16(x);
    if constexpr (sizeof(x) == 4) return __builtin_bitreverse32(x);
    if constexpr (sizeof(x) == 8) return __builtin_bitreverse64(x);
}
#endif

// BMI2 BITWISE OPS ====================================================================================================

// uintXX_t pdep(uintXX_t val, uintXX_t mask):
//     See https://www.felixcloutier.com/x86/pdep
#if defined(BITMANIP_X86_OR_X64) && (defined(BITMANIP_MSVC) || defined(BITMANIP_GNU_OR_CLANG) && defined(__BMI2__))
#define BITMANIP_HAS_BUILTIN_PDEP
template <typename Uint, std::enable_if_t<(std::is_unsigned_v<Uint> && sizeof(Uint) <= 8), int> = 0>
Uint pdep(Uint val, Uint mask)
{
    if constexpr (sizeof(Uint) < 8) {
        return _pdep_u32(static_cast<uint32_t>(val), static_cast<uint32_t>(mask));
    }
    else {
        return _pdep_u64(val, mask);
    }
}

// uintXX_t pext(uintXX_t val, uintXX_t mask):
//     See https://www.felixcloutier.com/x86/pext
#define BITMANIP_HAS_BUILTIN_PEXT
template <typename Uint, std::enable_if_t<(std::is_unsigned_v<Uint> && sizeof(Uint) <= 8), int> = 0>
Uint pext(Uint val, Uint mask)
{
    if constexpr (sizeof(Uint) < 8) {
        return _pext_u32(static_cast<uint64_t>(val), static_cast<uint64_t>(mask));
    }
    else {
        return _pext_u64(val, mask);
    }
}
#endif

}  // namespace bitmanip::builtin

#endif  // BUILTIN_HPP
