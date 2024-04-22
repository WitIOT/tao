// tao-macros.h -
//
// Macros definitions for TAO library needed for source code (not headers).
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_MACROS_H_
#define TAO_MACROS_H_ 1

#include <tao-basics.h>

#include <limits.h>
#include <inttypes.h>

/**
 * @defgroup Macros Macros
 *
 * @ingroup Utilities
 *
 * @brief Useful macro definitions.
 *
 * @{
 */

/**
 * @def TAO_MIN
 *
 * @brief Get the smallest of two values.
 *
 * This macro yields its first argument if any of its arguments is a NaN.
 *
 * @warning One of the arguments (the smallest one) is evaluated twice, use
 *          tao_min() to avoid that.
 */
#define TAO_MIN(x, y)   ((x) > (y) ? (y) : (x))

/**
 * @def TAO_MAX
 *
 * @brief Get the greatest of two values.
 *
 * This macro yields its first argument if any of its arguments is a NaN.
 *
 * @warning One of the arguments (the greatest one) is evaluated twice, use
 *          tao_max() to avoid that.
 */
#define TAO_MAX(x, y)   ((x) < (y) ? (y) : (x))

/**
 * @def TAO_STRLEN
 *
 * @brief Get the lenght of a null-terminated string.
 *
 * @warning The argument is evaluated more than once.
 *
 * @see tao_strlen().
 */
#define TAO_STRLEN(str)   ((str) == NULL || (str)[0] == '\0' ? 0 : strlen(str))

/**
 * @def TAO_STATIC_ARRAY_LENGTH(arr)
 *
 * @brief Get the number of elements of a static array.
 *
 * This macro yields the number of elements of its argument which must be a
 * static array.
 */
#define TAO_STATIC_ARRAY_LENGTH(arr) (sizeof(arr)/sizeof(arr[0]))

/**
 * @def TAO_OFFSET_OF
 *
 * @brief Get the offset of a member in a structure.
 *
 * @param type    Structure type.
 * @param member  Structure member.
 *
 * @return A number of bytes.
 */
#define TAO_OFFSET_OF(type, member) ((char*)&((type*)0)->member - (char*)0)

/**
 * @def TAO_COMPUTED_ADDRESS(addr, off)
 *
 * @brief Yield base address `addr` plus offset `off` in bytes.
 *
 * @return A `void*` pointer.
 */
#define TAO_COMPUTED_ADDRESS(addr, off) ((void*)((char*)(addr) + (off)))

/**
 * @def TAO_ROUND_UP(a, b)
 *
 * @brief Yields the least multiple of `b` which is greater or equal `a`.
 *
 * @warning Both arguments must be integers, `a` must be nonnegative, `b` must
 *          be positive.
 */
#define TAO_ROUND_UP(a, b)  ((((a) + ((b) - 1))/(b))*(b))

/**
 * @def TAO_ROUND_DOWN(a, b)
 *
 * @brief Yields the largest multiple of `b` which is less or equal `a`.
 *
 * @warning Both arguments must be integers, `a` must be nonnegative, `b` must
 *          be positive.
 */
#define TAO_ROUND_DOWN(a, b)  (((a)/(b))*(b))

/**
 * @def TAO_MIN_SIGNED_INT
 *
 * @brief Yields the minimum value of a signed integer.
 *
 * @param T   A signed integer type.
 *
 * @see TAO_MAX_SIGNED_INT, TAO_MIN_UNSIGNED_INT.
 */
#define TAO_MIN_SIGNED_INT(T) \
    ((-((T)1 << (sizeof(T)*CHAR_BIT - 2)))*(T)2)

/**
 * @def TAO_MAX_SIGNED_INT
 *
 * @brief Yields the maximum value of a signed integer.
 *
 * @param T   A signed integer type.
 *
 * @see TAO_MIN_SIGNED_INT, TAO_MAX_UNSIGNED_INT.
 */
#define TAO_MAX_SIGNED_INT(T) \
    ((((T)1 << (sizeof(T)*CHAR_BIT - 2)) - (T)1)*(T)2 + (T)1)
// The above expression is 2^(n-1) - 1 with n the number of bits in T and
// avoiding overflows, CHAR_BIT is defined in <limits.h>.

/**
 * @def TAO_MIN_UNSIGNED_INT
 *
 * @brief Yields the minimum value of an unsigned integer.
 *
 * @param T   An unsigned integer type.
 *
 * @see TAO_MAX_UNSIGNED_INT, TAO_MIN_SIGNED_INT.
 */
#define TAO_MIN_UNSIGNED_INT(T) ((T)0)

/**
 * @def TAO_MAX_UNSIGNED_INT
 *
 * @brief Yields the maximum value of an unsigned integer.
 *
 * @param T   An unsigned integer type.
 *
 * @see TAO_MIN_UNSIGNED_INT, TAO_MAX_SIGNED_INT.
 */
#define TAO_MAX_UNSIGNED_INT(T) ((T)(-1))
// Note: (~((T)0)) does not work for integers smaller than `int`

// Constants with maximal suffix to ensure pre-processor does not overflow.
#define TAO_UINT32_MAX_           4294967295ULL
#define TAO_UINT64_MAX_ 18446744073709551615ULL

// Constants for extremum integer values (with minimal suffix).
/**
 * @def TAO_INT8_MIN
 *
 * @brief Minimal value of an 8 bits signed integer.
 *
 * @see TAO_INT8_MAX, TAO_UINT8_MAX.
 */
#define TAO_INT8_MIN (-128)

/**
 * @def TAO_INT8_MAX
 *
 * @brief Maximal value of an 8 bits signed integer.
 *
 * @see TAO_INT8_MIN, TAO_UINT8_MAX.
 */
#define TAO_INT8_MAX   127

/**
 * @def TAO_UINT8_MAX
 *
 * @brief Maximal value of an 8 bits unsigned integer.
 *
 * @see TAO_INT8_MIN, TAO_INT8_MAX.
 */
#define TAO_UINT8_MAX  255

/**
 * @def TAO_INT16_MIN
 *
 * @brief Minimal value of an 16 bits signed integer.
 *
 * @see TAO_INT16_MAX, TAO_UINT16_MAX.
 */
#define TAO_INT16_MIN (-32768)

/**
 * @def TAO_INT16_MAX
 *
 * @brief Maximal value of an 16 bits signed integer.
 *
 * @see TAO_INT16_MIN, TAO_UINT16_MAX.
 */
#define TAO_INT16_MAX   32767

/**
 * @def TAO_UINT16_MAX
 *
 * @brief Maximal value of an 16 bits unsigned integer.
 *
 * @see TAO_INT16_MIN, TAO_INT16_MAX.
 */
#define TAO_UINT16_MAX  65535

/**
 * @def TAO_INT32_MIN
 *
 * @brief Minimal value of an 32 bits signed integer.
 *
 * @see TAO_INT32_MAX, TAO_UINT32_MAX.
 */
#if defined(TAO_DOXYGEN_)
#  define TAO_INT32_MIN ...
#endif

/**
 * @def TAO_INT32_MAX
 *
 * @brief Maximal value of an 32 bits signed integer.
 *
 * @see TAO_INT32_MIN, TAO_UINT32_MAX.
 */
#if defined(TAO_DOXYGEN_)
#  define TAO_INT32_MAX ...
#endif

/**
 * @def TAO_UINT32_MAX
 *
 * @brief Maximal value of an 32 bits unsigned integer.
 *
 * @see TAO_INT32_MIN, TAO_INT32_MAX.
 */
#if defined(TAO_DOXYGEN_)
#  define TAO_UINT32_MAX ...
#elif UINT_MAX >= TAO_UINT32_MAX_ // `int` is at least 32-bit
#  define TAO_INT32_MIN   (-TAO_INT32_MAX - 1)
#  define TAO_INT32_MAX    2147483647
#  define TAO_UINT32_MAX   4294967295
#elif ULONG_MAX >= TAO_UINT32_MAX_ // `long` is at least 32-bit
#  define TAO_INT32_MIN   (-TAO_INT32_MAX - 1L)
#  define TAO_INT32_MAX    2147483647L
#  define TAO_UINT32_MAX   4294967295UL
#elif ULLONG_MAX >= TAO_UINT32_MAX_ // `long long` is at least 32-bit
#  define TAO_INT32_MIN   (-TAO_INT32_MAX - 1LL)
#  define TAO_INT32_MAX    2147483647LL
#  define TAO_UINT32_MAX   4294967295ULL
#else
#  error no integer constant can store extremum `int32_t` values
#endif

/**
 * @def TAO_INT64_MIN
 *
 * @brief Minimal value of an 64 bits signed integer.
 *
 * @see TAO_INT64_MAX, TAO_UINT64_MAX.
 */
#if defined(TAO_DOXYGEN_)
#  define TAO_INT64_MIN ...
#endif

/**
 * @def TAO_INT64_MAX
 *
 * @brief Maximal value of an 64 bits signed integer.
 *
 * @see TAO_INT64_MIN, TAO_UINT64_MAX.
 */
#if defined(TAO_DOXYGEN_)
#  define TAO_INT64_MAX ...
#endif

/**
 * @def TAO_UINT64_MAX
 *
 * @brief Maximal value of an 64 bits unsigned integer.
 *
 * @see TAO_INT64_MIN, TAO_INT64_MAX.
 */
#if defined(TAO_DOXYGEN_)
#  define TAO_UINT64_MAX ...
#elif UINT_MAX >= TAO_UINT64_MAX_ // `int` is at least 64-bit
#  define TAO_INT64_MIN   (-TAO_INT64_MAX - 1)
#  define TAO_INT64_MAX     9223372036854775807
#  define TAO_UINT64_MAX   18446744073709551615U
#elif ULONG_MAX >= TAO_UINT64_MAX_ // `long` is at least 64-bit
#  define TAO_INT64_MIN   (-TAO_INT64_MAX - 1L)
#  define TAO_INT64_MAX     9223372036854775807L
#  define TAO_UINT64_MAX   18446744073709551615UL
#elif ULLONG_MAX >= TAO_UINT64_MAX_ // `long long` is at least 64-bit
#  define TAO_INT64_MIN   (-TAO_INT64_MAX - 1LL)
#  define TAO_INT64_MAX     9223372036854775807LL
#  define TAO_UINT64_MAX   18446744073709551615ULL
#else
#  error no integer constant can store extremum `int64_t` values
#endif

// Figure out the number of bits of standard C integer types.

/**
 * @def TAO_CHAR_BITS
 *
 * @brief Number of bits in a `char`.
 *
 * @see TAO_SHORT_BITS, TAO_INT_BITS, TAO_LONG_BITS, TAO_LLONG_BITS.
 */
#if defined(TAO_DOXYGEN_)
#  define TAO_CHAR_BITS ...
#elif UCHAR_MAX == TAO_UINT8_MAX
#  define TAO_CHAR_BITS 8
#elif UCHAR_MAX == TAO_UINT16_MAX
#  define TAO_CHAR_BITS 16
#elif UCHAR_MAX == 4294967295UL
#  define TAO_CHAR_BITS 32
#elif UCHAR_MAX == 18446744073709551615ULL
#  define TAO_CHAR_BITS 64
#else
#  error `sizeof(char)` is none of 8, 16, 32 or 64
#endif

/**
 * @def TAO_SHORT_BITS
 *
 * @brief Number of bits in a `short` integer.
 *
 * @see TAO_CHAR_BITS, TAO_INT_BITS, TAO_LONG_BITS, TAO_LLONG_BITS.
 */
#if defined(TAO_DOXYGEN_)
#  define TAO_SHORT_BITS ...
#elif USHRT_MAX == TAO_UINT8_MAX
#  define TAO_SHORT_BITS 8
#elif USHRT_MAX == TAO_UINT16_MAX
#  define TAO_SHORT_BITS 16
#elif USHRT_MAX == 4294967295UL
#  define TAO_SHORT_BITS 32
#elif USHRT_MAX == 18446744073709551615ULL
#  define TAO_SHORT_BITS 64
#else
#  error `sizeof(short)` is none of 8, 16, 32 or 64
#endif

/**
 * @def TAO_INT_BITS
 *
 * @brief Number of bits in an `int` integer.
 *
 * @see TAO_CHAR_BITS, TAO_SHORT_BITS, TAO_LONG_BITS, TAO_LLONG_BITS.
 */
#if defined(TAO_DOXYGEN_)
#  define TAO_INT_BITS ...
#elif UINT_MAX == TAO_UINT8_MAX
#  define TAO_INT_BITS 8
#elif UINT_MAX == TAO_UINT16_MAX
#  define TAO_INT_BITS 16
#elif UINT_MAX == TAO_UINT32_MAX
#  define TAO_INT_BITS 32
#elif UINT_MAX == TAO_UINT64_MAX
#  define TAO_INT_BITS 64
#else
#  error `sizeof(int)` is none of 8, 16, 32 or 64
#endif

/**
 * @def TAO_LONG_BITS
 *
 * @brief Number of bits in a `long` integer.
 *
 * @see TAO_CHAR_BITS, TAO_SHORT_BITS, TAO_INT_BITS, TAO_LLONG_BITS.
 */
#if defined(TAO_DOXYGEN_)
#  define TAO_LONG_BITS ...
#elif ULONG_MAX == TAO_UINT8_MAX
#  define TAO_LONG_BITS 8
#elif ULONG_MAX == TAO_UINT16_MAX
#  define TAO_LONG_BITS 16
#elif ULONG_MAX == TAO_UINT32_MAX
#  define TAO_LONG_BITS 32
#elif ULONG_MAX == TAO_UINT64_MAX
#  define TAO_LONG_BITS 64
#else
#  error `sizeof(long)` is none of 8, 16, 32 or 64
#endif

/**
 * @def TAO_LLONG_BITS
 *
 * @brief Number of bits in a `long long` integer.
 *
 * @see TAO_CHAR_BITS, TAO_SHORT_BITS, TAO_INT_BITS, TAO_LONG_BITS.
 */
#if defined(TAO_DOXYGEN_)
#  define TAO_LLONG_BITS ...
#elif ULLONG_MAX == TAO_UINT8_MAX
#  define TAO_LLONG_BITS 8
#elif ULLONG_MAX == TAO_UINT16_MAX
#  define TAO_LLONG_BITS 16
#elif ULLONG_MAX == TAO_UINT32_MAX
#  define TAO_LLONG_BITS 32
#elif ULLONG_MAX == TAO_UINT64_MAX
#  define TAO_LLONG_BITS 64
#else
#  error `sizeof(long long)` is none of 8, 16, 32 or 64
#endif

// Format for `printf` like functions.

/**
 * @def TAO_INT16_FORMAT(p,x)
 *
 * @brief Yields the `printf` for a 16-bit integer.
 *
 * @see TAO_INT64_FORMAT.
 */
#define TAO_INT16_FORMAT(p,x)  "%" #p TAO_JOIN3(PRI,x,32)

/**
 * @def TAO_INT32_FORMAT(p,x)
 *
 * @brief Yields the `printf` for a 32-bit integer.
 *
 * @see TAO_INT64_FORMAT.
 */
#define TAO_INT32_FORMAT(p,x)  "%" #p TAO_JOIN3(PRI,x,32)

/**
 * @def TAO_INT64_FORMAT(p,x)
 *
 * @brief Yields the `printf` for a 64-bit integer.
 *
 * @param p   Precision prefix for format.
 * @param x   Format, one of d, i, u, x or X.
 *
 * Example
 * ~~~~~{.c}
 * int64_t i64 = -32915;
 * int64_t u64 =  32915;
 * printf("i64 = " TAO_INT64_FORMAT( ,d) "\n", i64);
 * printf("i64 = " TAO_INT64_FORMAT(8,d) "\n", i64);
 * printf("u64 = " TAO_INT64_FORMAT( ,u) "\n", u64);
 * printf("u64 = " TAO_INT64_FORMAT(8,u) "\n", u64);
 * ~~~~~
 *
 * @see TAO_INT16_FORMAT, TAO_INT32_FORMAT.
 */
#define TAO_INT64_FORMAT(p,sfx)  "%" #p TAO_JOIN3(PRI,sfx,64)

// Helpers for branch prediction.  See
// http://blog.man7.org/2012/10/how-much-do-builtinexpect-likely-and.html
// and https://stackoverflow.com/questions/109710/how-does-the-likely-unlikely-macros-in-the-linux-kernel-works-and-what-is-their.

#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#  define TAO_LIKELY(expr)      (__builtin_expect(!!(expr), 1))
#  define TAO_UNLIKELY(expr)    (__builtin_expect(!!(expr), 0))
#else
#  define TAO_LIKELY(expr)      (expr)
#  define TAO_UNLIKELY(expr)    (expr)
#endif

/**
 * @def TAO_LIKELY(expr)
 *
 * @brief Indicate to the compiler that an expression is expected to be true.
 *
 * The @ref TAO_LIKELY and @ref TAO_UNLIKELY macros let the programmer give
 * hints to the compiler about the expected result of an expression.  Some
 * compilers can use this information for optimizations.
 *
 * @param expr   The expression to test.
 *
 * @see TAO_UNLIKELY.
 */

/**
 * @def TAO_UNLIKELY(expr)
 *
 * @brief Indicate to the compiler that an expression is expected to be false.
 *
 * @param expr   The expression to test.
 *
 * @see TAO_LIKELY.
 */

/**
 * @def TAO_NEW(numb, type)
 *
 * @brief Allocate zero-filled memory for a given number of items of the same
 *        type.
 *
 * Each argument is evaluated once.
 *
 * @param numb   Number of items.
 * @param type   Type of items.
 *
 * @return A `type*` pointer, `NULL` in case of failure.
 *
 * @see tao_calloc().
 */
#define TAO_NEW(numb, type) ((type*)tao_calloc((numb), sizeof(type)))

/**
 * @def TAO_JOIN
 *
 * @brief Join 2 arguments literally.
 *
 * This macro joins its arguments without applying macro expansions unless
 * called by another macro.
 *
 * @see TAO_XJOIN.
 */
#define TAO_JOIN(a,b)        a##b

/**
 * @def TAO_JOIN2
 *
 * @brief Join 2 arguments literally.
 *
 * This macro joins its arguments without applying macro expansions unless
 * called by another macro.
 *
 * @see TAO_XJOIN.
 */
#define TAO_JOIN2(a,b)       a##b

/**
 * @def TAO_JOIN3
 *
 * @brief Join 3 arguments literally.
 *
 * This macro joins its arguments without applying macro expansions unless
 * called by another macro.
 *
 * @see TAO_XJOIN.
 */
#define TAO_JOIN3(a,b,c)     a##b##c

/**
 * @def TAO_JOIN4
 *
 * @brief Join 4 arguments literally.
 *
 * This macro joins its arguments without applying macro expansions unless
 * called by another macro.
 *
 * @see TAO_XJOIN.
 */
#define TAO_JOIN4(a,b,c,d)   a##b##c##d

/**
 * @def TAO_JOIN5
 *
 * @brief Join 5 arguments literally.
 *
 * This macro joins its arguments without applying macro expansions unless
 * called by another macro.
 *
 * @see TAO_XJOIN.
 */
#define TAO_JOIN5(a,b,c,d,e) a##b##c##d##e

/**
 * @def TAO_XJOIN
 *
 * @brief Join 2 arguments with expansions.
 *
 * This macro joins its arguments after applying macro expansions.
 *
 * @see TAO_XJOIN.
 */
#define TAO_XJOIN(a,b) TAO_JOIN(a,b)

/**
 * @def TAO_XJOIN2
 *
 * @brief Join 2 arguments with expansions.
 *
 * This macro joins its arguments after applying macro expansions.
 *
 * @see TAO_XJOIN.
 */
#define TAO_XJOIN2(a,b) TAO_JOIN2(a,b)

/**
 * @def TAO_XJOIN3
 *
 * @brief Join 3 arguments with expansions.
 *
 * This macro joins its arguments after applying macro expansions.
 *
 * @see TAO_XJOIN.
 */
#define TAO_XJOIN3(a,b,c) TAO_JOIN3(a,b,c)

/**
 * @def TAO_XJOIN4
 *
 * @brief Join 4 arguments with expansions.
 *
 * This macro joins its arguments after applying macro expansions.
 *
 * @see TAO_XJOIN.
 */
#define TAO_XJOIN4(a,b,c,d) TAO_JOIN4(a,b,c,d)

/**
 * @def TAO_XJOIN5
 *
 * @brief Join 5 arguments with expansions.
 *
 * This macro joins its arguments after applying macro expansions.
 *
 * @see TAO_XJOIN.
 */
#define TAO_XJOIN5(a,b,c,d,e) TAO_JOIN5(a,b,c,d,e)

/**
 * @def TAO_ALIGNMENT
 *
 * This macro gives the preferred number of bytes for memory alignment.  Good
 * memory alignment can improve vectorization and false sharing
 * (https://en.wikipedia.org/wiki/False_sharing).
 *
 * Alignment of data for vectorization depends on the chosen compilation
 * settings.  The following table summarizes the value of macro
 * `__BIGGEST_ALIGNMENT__` with different settings:
 *
 * | Alignment (bytes) | Compilation Options |
 * |:-----------------:|:--------------------|
 * |        16         | -ffast-math -msse   |
 * |        16         | -ffast-math -msse2  |
 * |        16         | -ffast-math -msse3  |
 * |        16         | -ffast-math -msse4  |
 * |        16         | -ffast-math -mavx   |
 * |        32         | -ffast-math -mavx2  |
 *
 * The address of attached shared memory is a multiple of memory page size
 * (`PAGE_SIZE` which is 4096 on the Linux machine I tested) and so much larger
 * than `TAO_ALIGNMENT` (defined below).  So, in principle, it is sufficient to
 * align the shared array data to a multiple of ALIGNMENT relative to the
 * address of the attached shared memory to have correct alignment for all
 * processes.
 *
 * To avoid false sharing, it is necessary to align on multiple of the size of
 * the cache lines.  These sizes can be queried on Linux by the following
 * command:
 *
 *     LANG=C getconf -a | grep CACHE_LINESIZE
 *
 * On recent Intel processors, the cache lines have 64 bytes.
 */
#define TAO_ALIGNMENT 64
#if defined(__BIGGEST_ALIGNMENT__) && __BIGGEST_ALIGNMENT__ > TAO_ALIGNMENT
#  warning TAO_ALIGNMENT is smaller than __BIGGEST_ALIGNMENT__
#endif

/**
 * @}
 */

#endif // TAO_MACROS_H_
