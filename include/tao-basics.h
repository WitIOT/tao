// tao-basics.h -
//
// Basic definitions for TAO library that are needed by other headers.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2018-2022, Éric Thiébaut.

#ifndef TAO_BASICS_H_
#define TAO_BASICS_H_ 1

#include <stdbool.h> // to use booleans
#include <stdlib.h>  // for standard types like size_t
#include <stdint.h>  // for integer types like uint32_t

#ifdef __cplusplus
#  define TAO_BEGIN_DECLS extern "C" {
#  define TAO_END_DECLS }
#else
#  define TAO_BEGIN_DECLS
#  define TAO_END_DECLS
#endif

/**
 * @defgroup Basics   Basic definitions
 *
 * @ingroup Utilities
 *
 * @brief Basic header definitions.
 *
 * Header file @ref tao-basics.h provides definitions of basic types and macros
 * widely used in TAO library headers.
 *
 * @{
 */

/**
 * @def tao_atomic
 *
 * This macro is to mark atomic variables.
 */
#define tao_atomic volatile _Atomic // FIXME: `volatile` may be superfluous

/**
 * Status returned by most TAO Library functions.
 *
 * The values of @ref TAO_OK (0) and @ref TAO_ERROR (-1) reflect the
 * conventions of most functions in the standard C library (not the POSIX
 * Threads Library whose functions return 0 or the error code).  The other
 * possible value is @ref TAO_TIMEOUT (1) which may be returned by functions
 * that have a time limit.  Unless explicitly specified, a function returning
 * @ref TAO_ERROR has updated the caller's last error.
 */
typedef enum tao_status {
    TAO_ERROR   = -1, ///< An error occurred in called function.
    TAO_OK      =  0, ///< Called function completed successfully.
    TAO_TIMEOUT =  1  ///< Time-out occurred in called function.
} tao_status;

/**
 * Serial number.
 *
 * This integer type is used for serial numbers and counters.  It is a signed
 * 64-bit integer.  Having a signed integer is to simplify coding.  A 64-bit
 * signed integer is sufficient to run at 1kHz during more than 292Myr.
 */
typedef int64_t tao_serial;

/**
 * @def TAO_FORMAT_PRINTF(i,j)
 *
 * Mark a printf-like function with format given by i-th argument and arguments
 * to print starting at j-th argument.
 */
#if (defined(__GNUC__) && (__GNUC__ > 2)) || defined(__clang__)
#  define TAO_FORMAT_PRINTF(i,j) __attribute__((__format__(__printf__, i, j)))
#else
#  define TAO_FORMAT_PRINTF(i,j) // nothing
#endif

/**
 * @def TAO_FORMAT_SCANF(i,j)
 *
 * Mark a scanf-like function with format given by i-th argument and arguments
 * to print starting at j-th argument.
 */
#if (defined(__GNUC__) && (__GNUC__ > 2)) || defined(__clang__)
#  define TAO_FORMAT_SCANF(i,j) __attribute__((__format__(__scanf__, i, j)))
#else
#  define TAO_FORMAT_SCANF(i,j) // nothing
#endif

/**
 * @def TAO_NORETURN
 *
 * Mark a function that never returns.
 */
#if (defined(__GNUC__) && (__GNUC__ > 2)) || defined(__clang__)
#  define TAO_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER) && (_MSC_VER >= 1310)
#  define TAO_NORETURN _declspec(noreturn)
#else
#  define TAO_NORETURN // nothing
#endif

/**
 * @}
 */

#endif // TAO_BASICS_H_
