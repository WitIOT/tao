// tao-utils.h -
//
// Definitions for utility functions in TAO (dynamic memory, strings, time).
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_UTILS_H_
#define TAO_UTILS_H_ 1

#include <tao-basics.h>
#include <tao-buffers.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

TAO_BEGIN_DECLS

/**
 * @defgroup DynamicMemory  Dynamic memory
 *
 * @ingroup Utilities
 *
 * @brief Management of conventional dynamic memory.
 *
 * These functions are provided to report errors like other functions in TAO
 * Library.
 *
 * @see SharedMemory
 *
 * @{
 */

/**
 * Allocate dynamic memory.
 *
 * This function behaves as malloc() except that, in case of failure, the last
 * error of the calling thread is updated.  The caller is responsible for
 * calling free() or tao_free() to free the allocated memory when no longer
 * needed.
 *
 * @param size   Number of bytes to allocate.
 *
 * @return The address of allocated dynamic memory; `NULL` in case of failure.
 *
 * @see tao_free, tao_calloc.
 */
extern void* tao_malloc(
    size_t size);

/**
 * Reallocate dynamic memory.
 *
 * This function behaves as realloc() except that, in case of failure, the last
 * error of the calling thread is updated.  The caller is responsible for
 * calling free() or tao_free() to free the allocated memory when no longer
 * needed.
 *
 * @param ptr    Address of dynamic memory to reallocate.
 * @param size   Number of bytes to allocate.
 *
 * @return The address of allocated dynamic memory; `NULL` in case of failure.
 *
 * @see tao_free, tao_malloc, tao_calloc.
 */
extern void* tao_realloc(
    void* ptr,
    size_t size);

/**
 * Allocate dynamic memory.
 *
 * This function behaves as calloc() except that, in case of failure, the last
 * error of the calling thread is updated.  The caller is responsible for
 * calling free() or tao_free() to free the allocated memory when no longer
 * needed.
 *
 * @param nelem  Number of elements to allocate.
 * @param elsize Number of bytes per element.
 *
 * @return The address of allocated dynamic memory; `NULL` in case of failure.
 *
 * @see tao_free, tao_malloc.
 */
extern void* tao_calloc(
    size_t nelem,
    size_t elsize);

/**
 * Free dynamic memory.
 *
 * This function behaves as free() except that it accepts a `NULL` pointer.
 *
 * @param ptr    Address of dynamic memory (can be `NULL`).
 *
 * @see tao_malloc, tao_calloc.
 */
extern void tao_free(
    void* ptr);

/**
 * @}
 */

/**
 * @defgroup StringTools  String tools
 *
 * @ingroup Utilities
 *
 * @brief Useful functions for C-strings.
 *
 * @{
 */

/**
 * Get the length of a string.
 *
 * This function behaves as strlen() except that a `NULL` argument yield 0.
 *
 * @param str   String.
 *
 * @return The length of the string; 0 if @b str is `NULL`.
 *
 * @see strlen.
 */
extern size_t tao_strlen(
    const char* str);

/**
 * Strip directory part of a path.
 *
 * @param path   String.
 *
 * @return The path after the last `'/'` if any; `NULL` if @b path is `NULL`.
 */
extern const char* tao_basename(
    const char* path);

/**
 * @}
 */

/**
 * @defgroup Messages  Messages
 *
 * @ingroup Utilities
 *
 * @brief Management of messages.
 *
 * @{
 */

/**
 * @brief Level of message for logging.
 *
 * These enumeration values are in increasing order of seriousness so that it
 * is possible to use them to set a threshold for filtering messages.
 */
typedef enum tao_message_level {
    // Values must be in ascending order.
    TAO_MESG_DEBUG  = 0, ///< Debug message.
    TAO_MESG_INFO   = 1, ///< Information message.
    TAO_MESG_WARN   = 2, ///< Warning message.
    TAO_MESG_ERROR  = 3, ///< Runtime error.
    TAO_MESG_ASSERT = 4, ///< Assertion error or bug.
    TAO_MESG_FATAL  = 5, ///< Fatal error causing the process to exit.
    TAO_MESG_QUIET  = 6  ///< Suppress all messages.
} tao_message_level;

/**
 * Print a formatted message.
 *
 * Depending on the current threshold for printing messages, this function
 * either does nothing, or print a formatted message.
 *
 * This function is thread-safe.  Errors are ignored.
 *
 * @param output  File stream to print the message.  `stdout` is assumed if
 *                `NULL`.
 *
 * @param level   Seriousness of the message.  The message is printed if this
 *                value is greater of equal the current threshold set by
 *                tao_message_threshold_set().
 *
 * @param format  Format string (as for `printf`).
 *
 * @param ...     Other arguments.
 */
extern void tao_inform(
    FILE* output,
    tao_message_level level,
    const char* format,
    ...) TAO_FORMAT_PRINTF(3,4);

/**
 * Get the minimum level of printed messages.
 *
 * This function is thread-safe.
 *
 * @return The current minimum level for messages printed by tao_inform().
 */
extern tao_message_level tao_message_threshold_get(
    void);

/**
 * Set the minimum level of printed messages.
 *
 * This function is thread-safe.
 *
 * @param level   The minimum level of messages printed by tao_inform().  For
 *                instance, `TAO_MESG_QUIET` to suppress printing of messages
 *                or `TAO_MESG_DEBUG` to print everything.
 */
extern void tao_message_threshold_set(
    tao_message_level level);

/**
 * @}
 */

/**
 * @defgroup Time  Time tools
 *
 * @ingroup Utilities
 *
 * @brief Measurement of time and time intervals.
 *
 * Time is stored in a structure of type @ref tao_time and which has a
 * nanosecond resolution.  The actual precision however depends on the
 * resolution of the functions provided by the system to get a time.  The
 * maximum time amplitude that can be represented is
 * @f$\approx\pm2.9\times10^{11}@f$ years (nearly 20 times the age of the
 * Universe).  So it is probably sufficient to represent any absolute time.
 *
 * The function tao_get_monotonic_time() can be used to precisely measure time
 * intervals, while the function tao_get_current_time() can be called to get
 * the current time.
 *
 * @{
 */

/**
 * Type for the members of a time structure.
 */
typedef int64_t tao_time_member;

/**
 * Structure to store time with nanosecond resolution.
 */
typedef struct tao_time {
    tao_time_member  sec; ///< Number of seconds
    tao_time_member nsec; ///< Number of nanoseconds
} tao_time;

/**
 * Build a time value.
 *
 * The call `TAO_TIME(s,ns)` yields a compound literal of type @ref tao_time
 * for `s` seconds and `ns` nanoseconds.
 */
#define TAO_TIME(s, ns) ((tao_time){.sec = (s), .nsec = (ns)})

/**
 * Time value when unknown/unset.
 *
 * This macro yields the constant of type @ref tao_time used to represent
 * unknown or unset time.
 */
#define TAO_UNKNOWN_TIME TAO_TIME(0,0)

/**
 * Maximum number of seconds in a TAO time structure.
 *
 * This macro yields the maximum number of seconds that fit in a
 * `tao_time_member` integer.
 *
 * @see TAO_MAX_TIME_SECONDS.
 */
#define TAO_MAX_TIME_SECONDS TAO_MAX_SIGNED_INT(tao_time_member)

/**
 * Minimum number of seconds in a TAO time structure.
 *
 * This macro yields the minimum number of seconds that fit in a
 * `tao_time_member` integer.
 *
 * @see TAO_MAX_TIME_SECONDS.
 */
#define TAO_MIN_TIME_SECONDS TAO_MIN_SIGNED_INT(tao_time_member)


/**
 * Number of nanoseconds per second.
 */
#define TAO_NANOSECONDS_PER_SECOND  (1000000000)

/**
 * Number of microseconds per second.
 */
#define TAO_MICROSECONDS_PER_SECOND  (1000000)

/**
 * Number of milliseconds per second.
 */
#define TAO_MILLISECONDS_PER_SECOND  (1000)

/**
 * One nanosecond in SI units (seconds).
 */
#define TAO_NANOSECOND (1e-9*TAO_SECOND)

/**
 * One microsecond in SI units (seconds).
 */
#define TAO_MICROSECOND (1e-6*TAO_SECOND)

/**
 * One millisecond in SI units (seconds).
 */
#define TAO_MILLISECOND (1e-3*TAO_SECOND)

/**
 * One second in SI units (seconds).
 */
#define TAO_SECOND (1.0)

/**
 * One minute in SI units (seconds).
 */
#define TAO_MINUTE (60*TAO_SECOND)

/**
 * One hour in SI units (seconds).
 */
#define TAO_HOUR (60*TAO_MINUTE)

/**
 * One day in SI units (seconds).
 */
#define TAO_DAY (24*TAO_HOUR)

/**
 * One year in SI units (seconds).
 */
#define TAO_YEAR (365.25*TAO_DAY)

/**
 * @def TAO_TIMEOUT_MIN
 *
 * Macro `TAO_TIMEOUT_MIN` yields the minimum relative timeout defined as the
 * smallest positive value which, when rounded to the assumed clock resolution
 * (one nanosecond), is equal to the assumed clock resolution.
 */
#define TAO_TIMEOUT_MIN  0.5e-9

/**
 * Possible values returned by tao_get_absolute_timeout()
 *
 * The function tao_get_absolute_timeout()
 */
typedef enum tao_timeout {
    TAO_TIMEOUT_ERROR  = -2,
    TAO_TIMEOUT_PAST   = -1,
    TAO_TIMEOUT_NOW    =  0,
    TAO_TIMEOUT_FUTURE =  1,
    TAO_TIMEOUT_NEVER  =  2
} tao_timeout;

/**
 * Sleep for a specified hight-resolution number of seconds.
 *
 * This function causes the calling thread to sleep either until the number of
 * specified seconds have elapsed or until a signal arrives which is not
 * ignored.
 *
 * @param secs   The amount of time to sleep in seconds.  Must be nonnegative
 *               and less than the maximum value of a `tao_time_member`
 *               integer which is is fairly large (at least about 68 years).
 *               Can have a high resolution, at least one microsecond but,
 *               depepending on the operationg system, it may be as good as one
 *               nanosecond.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure or
 *         interrupt.
 */
extern tao_status tao_sleep(
    double secs);

/**
 * Copy time.
 *
 * Copy time in structure pointed by @a src to structure pointed by @a dst.
 *
 * @note It is probably faster (and simpler) to just do `*dst = *src` as is
 * allowed by ANSI C to copy structures.
 *
 * @param dst   The address of the destination time structure.
 * @param src   The address of the source time structure.
 *
 * @return @a dst.
 *
 * @see TAO_COPY_TIME.
 */
extern tao_time* tao_copy_time(
    tao_time* dst,
    const tao_time* src);

/**
 * Get monotonic time.
 *
 * This function yields a monotonic time since some unspecified starting point
 * but which is not affected by discontinuous jumps in the system time (e.g.,
 * if the system administrator manually changes the clock), but is affected by
 * the incremental adjustments performed by adjtime() and NTP.
 *
 * @warning For systems where it is not possible to retrieve a monotonic time,
 *          the time given by tao_get_current_time() is returned instead.
 *
 * @param dest   Address to store the time.  In case of failure,
 *               @ref TAO_UNKNOWN_TIME is stored in @b dest.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_get_monotonic_time(
    tao_time* dest);

/**
 * Get the current time.
 *
 * This function yields the current time since the
 * [Epoch](https://en.wikipedia.org/wiki/Unix_time), that is 00:00:00 UTC, 1
 * January 1970.
 *
 * @param dest   Address to store the time.  In case of failure,
 *               @ref TAO_UNKNOWN_TIME is stored in @b dest.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_get_current_time(
    tao_time* dest);

/**
 * @def TAO_TIME_NORMALIZE(T, s, ns)
 *
 * This macro nomrlize the time specified by the L-values `s` and `ns`
 * (respectively the number of seconds and of nanoseconds).  Argument `T` is
 * the integer type to use for temporaries.  This macro ensures that the number
 * of nanoseconds is in the range 0 to 999,999,999.
 *
 * @see tao_time_normalize.
 */
#define TAO_TIME_NORMALIZE(T, s, ns)                            \
    do {                                                        \
        if ((ns) < 0) {                                         \
            T __adj = 1 - (ns)/TAO_NANOSECONDS_PER_SECOND;      \
            (s) -= __adj;                                       \
            (ns) += __adj*TAO_NANOSECONDS_PER_SECOND;           \
        } else if ((ns) >= TAO_NANOSECONDS_PER_SECOND) {        \
            T __adj = ((ns)/TAO_NANOSECONDS_PER_SECOND);        \
            (s) += __adj;                                       \
            (ns) -= __adj*TAO_NANOSECONDS_PER_SECOND;           \
        }                                                       \
    } while (false)

/**
 * Normalize time.
 *
 * This function adjusts the members of a @ref tao_time structure so that the
 * time stored in the structure is correct.  More specifically it ensures that
 * the number of nanoseconds is in the range 0 to 999,999,999.
 *
 * @param ts   Address of a  @ref tao_time structure to adjust.
 *
 * @return The address @a ts.
 *
 * @see TAO_TIME_NORMALIZE.
 */
extern tao_time* tao_time_normalize(
    tao_time* ts);

/**
 * Add times.
 *
 * This function adds 2 times.
 *
 * @warning This function is meant to be fast.  It makes no checking about the
 * validity of the arguments nor integer oveflows.  Normally the destination
 * time is such that the number of nanoseconds is nonnegative and strictly less
 * than 1,000,000,000.
 *
 * @param dest   Address to store the result.
 * @param a      Address of first time value.
 * @param b      Address of second time value.
 *
 * @return The address @a dest.
 */
extern tao_time* tao_time_add(
    tao_time* dest,
    const tao_time* a,
    const tao_time* b);

/**
 * Subtract times.
 *
 * This function subtracts 2 times.
 *
 * @warning This function is meant to be fast.  It makes no checking about the
 * validity of the arguments nor integer oveflows.  Normally the destination
 * time is such that the number of nanoseconds is nonnegative and strictly less
 * than 1,000,000,000.
 *
 * @param dest   Address to store the result.
 * @param a      Address of first time value.
 * @param b      Address of second time value.
 *
 * @return The address @a dest.
 */
extern tao_time* tao_time_subtract(
    tao_time* dest,
    const tao_time* a,
    const tao_time* b);

/**
 * Convert time in seconds.
 *
 * @param t      Address of time value.
 *
 * @return The number of seconds given by the time stored in @b t.
 */
extern double tao_time_to_seconds(
    const tao_time* t);

/**
 * Convert time in milliseconds.
 *
 * @param t      Address of time value.
 *
 * @return The number of milliseconds given by the time stored in @b t.
 */
extern double tao_time_to_milliseconds(
    const tao_time* t);

/**
 * Convert time in microseconds.
 *
 * @param t      Address of time value.
 *
 * @return The number of microseconds given by the time stored in @b t.
 */
extern double tao_time_to_microseconds(
    const tao_time* t);

/**
 * Convert time in nanoseconds.
 *
 * @param t      Address of time value.
 *
 * @return The number of nanoseconds given by the time stored in @b t.
 */
extern double tao_time_to_nanoseconds(
    const tao_time* t);

/**
 * Elapsed time in seconds.
 *
 * @param t      Address of time value.
 * @param t0     Address of time value at origin.
 *
 * @return The number of elapsed seconds at time @b t since time @b t0.
 */
extern double tao_elapsed_seconds(
    const tao_time* t,
    const tao_time* t0);

/**
 * Elapsed time in milliseconds.
 *
 * @param t      Address of time value.
 * @param t0     Address of time value at origin.
 *
 * @return The number of elapsed milliseconds at time @b t since time @b t0.
 */
extern double tao_elapsed_milliseconds(
    const tao_time* t,
    const tao_time* t0);

/**
 * Elapsed time in microseconds.
 *
 * @param t      Address of time value.
 * @param t0     Address of time value at origin.
 *
 * @return The number of elapsed microseconds at time @b t since time @b t0.
 */
extern double tao_elapsed_microseconds(
    const tao_time* t,
    const tao_time* t0);

/**
 * Elapsed time in nanoseconds.
 *
 * @param t      Address of time value.
 * @param t0     Address of time value at origin.
 *
 * @return The number of elapsed nanoseconds at time @b t since time @b t0.
 */
extern double tao_elapsed_nanoseconds(
    const tao_time* t,
    const tao_time* t0);

/**
 * Convert a number of seconds into a time structure.
 *
 * @param dest   Address to store the result.
 * @param secs   A fractional number of seconds.
 *
 * @return The address @b dest.
 *
 * @warning This function never fails.  If @b secs is too large (in amplitude)
 * to be represented, `INT64_MAX` or `INT64_MIN` seconds and 0 nanoseconds are
 * assumed.  If @b secs is a NaN (Not a Number), 0 seconds and -1 nanoseconds
 * are assumed.  Otherwise, the number of seconds stored in @b dest is strictly
 * greater than `INT64_MIN` and strictly less than `INT64_MAX` while the number
 * of nanoseconds is greater or equal 0 and strictly less than 1,000,000,000.
 * It is therefore always possible guess from the stored time whether @b secs
 * was representable as a time structure with nanosecond precision.
 */
extern tao_time* tao_seconds_to_time(
    tao_time* dest,
    double secs);

/**
 * Convert a timespec structure into a time structure.
 *
 * This function converts a `timespec` structure into a time structure taking
 * care of normalizing the result.
 *
 * @param dst   Address to store the result.
 * @param src   Address of timespec structure (must not be `NULL`).
 *
 * @return The address @b dst.
 */
extern tao_time* tao_timespec_to_time(
    tao_time* dst,
    const struct timespec* src);

/**
 * Convert a timeval structure into a time structure.
 *
 * This function convert a `timeval` structure into a time structure taking
 * care of normalizing the result.
 *
 * @param dst   Address to store the result.
 * @param src   Address of timeval structure (must not be `NULL`).
 *
 * @return The address @b dst.
 */
extern tao_time* tao_timeval_to_time(
    tao_time* dst,
    const struct timeval* src);

/**
 * Possible formats for printing a time-stamp.
 */
typedef enum tao_time_format {
    TAO_TIME_FORMAT_FRACTIONAL_SECONDS,
    TAO_TIME_FORMAT_DATE_WITH_SECONDS,
    TAO_TIME_FORMAT_DATE_WITH_MILLISECONDS,
    TAO_TIME_FORMAT_DATE_WITH_MICROSECONDS,
    TAO_TIME_FORMAT_DATE_WITH_NANOSECONDS
} tao_time_format;

/**
 * Print a time-stamp in a human readable form to a string.
 *
 * @param str    Destination string (must have at least 64 characters
 *               including the terminating null).
 *
 * @param fmt    Format.
 *
 * @param ts     Time-stamp.  The current time is used if `NULL`.
 *
 * @return The address @b str.
 */
extern char* tao_time_sprintf(
    char* str,
    tao_time_format fmt,
    const tao_time* ts);

/**
 * Print a time-stamp in a human readable form to a string.
 *
 * If the destination @a str is non-`NULL`, this function writes as much as
 * possible of the resulting string in @a str but no more than `size` bytes and
 * with a terminating null.  In any cases, the length of the string
 * corresponding to the complete result (excluding the terminating null) is
 * returned.
 *
 * If `format` is @ref TAO_TIME_FORMAT_FRACTIONAL_SECONDS, the result is of the
 * form `seconds.nanoseconds`; otherwise, the format is `"%Y-%m-%d %H:%M:%S"`
 * followed by the number of nanoseconds.
 *
 * @param str    Destination string (nothing is written there if `NULL`).
 *
 * @param size   Number of bytes available in the destination string.
 *
 * @param fmt    Format.
 *
 * @param ts     Time-stamp.  The current time is used if `NULL`.
 *
 * @return The number of bytes needed to store the complete formatted string
 *         (excluding the terminating null).  Thus, a return value of @a size
 *         or more means that the output was truncated.
 */
extern long tao_time_snprintf(
    char* str,
    long size,
    tao_time_format fmt,
    const tao_time* ts);

/**
 * Print a time-stamp in a human readable form to a file stream.
 *
 * See tao_time_snprintf() for details.
 *
 * @param stream Destination stream.
 *
 * @param fmt    Format.
 *
 * @param ts     Time-stamp.  The current time is used if `NULL`.
 */
extern void tao_time_fprintf(
    FILE* stream,
    tao_time_format fmt,
    const tao_time* ts);

/**
 * Compute absolute timeout.
 *
 * This function computes an absolute timeout given a duration relative to the
 * current time (as given by the clock `CLOCK_REALTIME`).  The function tries
 * to avoid overflows (integers would wrap with negative values in that case)
 * and stores the maximum possible value in @a ts.  The value returned by the
 * function indicates whether that happens.
 *
 * @param t      Address of `tao_time` structure.
 * @param secs   Number of seconds from now.
 *
 * @return @ref TAO_TIMEOUT_NEVER if timeout is too long, @ref
 *         TAO_TIMEOUT_FUTURE if timeout may occur in the future, @ref
 *         TAO_TIMEOUT_NOW if absolute value of @a secs is smaller than @ref
 *         TAO_TIMEOUT_MIN, @ref TAO_TIMEOUT_PAST if @a secs is negative and
 *         @ref TAO_TIMEOUT_ERROR if some errors occurred.
 *
 * @see tao_timeout, TAO_TIMEOUT_MIN, tao_get_maximum_absolute_time().
 */
extern tao_timeout tao_get_absolute_timeout(
    tao_time* t,
    double secs);

/**
 * Maximum number of seconds since the Epoch.
 *
 * This function yields the maximum number of seconds that fit in a
 * `tao_time_member` integer.  Type `tao_time_member` is 64-bit signed
 * integer which gives a maximum of about 9.223372036854776e18 seconds (more
 * than 2.9e11 years) since the Epoch.
 *
 * @return A large number of seconds.
 *
 * @see TAO_MAX_TIME_SECONDS.
 */
extern double tao_get_maximum_absolute_time(
    void);

/**
 * Structure to collect time statistics data.
 */
typedef struct tao_time_stat_data tao_time_stat_data;
struct tao_time_stat_data {
    double  min; ///< Minimum time
    double  max; ///< Maximum time
    double sum1; ///< Sum of times
    double sum2; ///< Sum of squared times
    size_t numb; ///< Number of collected samples
};

/**
 * Structure to store time statistics.
 */
typedef struct tao_time_stat tao_time_stat;
struct tao_time_stat {
    double  min; ///< Minimum time
    double  max; ///< Maximum time
    double  avg; ///< Mean time
    double  std; ///< Standard deviation
    size_t numb; ///< Number of tests
};

/**
 * Initialize or reset time statistics data.
 *
 * Call this function before integrating time statistics with
 * tao_update_time_statistics().
 *
 * @param tsd  Pointer to time statistics data structure.
 */
extern void tao_initialize_time_statistics(
    tao_time_stat_data* tsd);

/**
 * Integrate time statistics data.
 *
 * This function accounts for a new time sample.
 *
 * @warning All time samples must be given with the same units.
 *
 * @param tsd  Pointer to time statistics data structure.
 * @param t    New time sample.
 */
extern void tao_update_time_statistics(
    tao_time_stat_data* tsd,
    double t);

/**
 * Compute time statistics.
 *
 * This function compute time statistics in the same temporal units as the
 * collected time samples.
 *
 * @param ts   Pointer to destination time statistics structure.
 * @param tsd  Pointer to source time statistics data structure.
 *
 * @return The destination.
 */
extern tao_time_stat* tao_compute_time_statistics(
    tao_time_stat* ts,
    tao_time_stat_data const* tsd);

/**
 * Print time statistics.
 *
 * It is assumed that time samples are in seconds.
 *
 * @param out    Output file stream.
 * @param pfx    Suffix for each printed lines.
 * @param ts     Pointer to time statistics structure.
 */
extern void tao_print_time_statistics(
    FILE* out,
    char const* pfx,
    tao_time_stat const* ts);

/**
 * @}
 */

//-----------------------------------------------------------------------------

/**
 * @defgroup Commands  Command parsing
 *
 * @ingroup Utilities
 *
 * @brief Parsing of textual commands.
 *
 * These functions are provided to split lines of commands into arguments and
 * conversely to pack arguments into a command-line.
 *
 * Just like the `argv` array in C `main` function, a command is a list of
 * "words" which are ordinary C-strings.  All characters are allowed in a word
 * except the null character `\0` as it serves as a marker of the end of the
 * string.
 *
 * In order to communicate, processes may exchange commands packed into a
 * single string and sent through communication channels.  In order to allow
 * for sendind/receiving several successive commands and for coping with
 * partially transmitted commands, their size must be part of the sent data or
 * they must be terminated by some given marker.  In order to make things
 * simple, it has been chosen that successive commands be separated by a single
 * line-feed character (`'\n'`, ASCII code `0x0A`).  This also simplify the
 * writing of commands into scripts.
 *
 * String commands have to be parsed into words before being used.  Since any
 * character (but the null) is allowed in such words there must be means to
 * separate words in command strings and to allow for having a line-feed
 * character in a word (not a true one because it is used to indicate end of
 * the command string).
 *
 * The following rules are applied to parse a command string into words:
 *
 * 1- An end-of-line (EOL) sequence at the end of the command string is allowed
 *    and ignored.  To cope with different styles, an EOL can be any of the
 *    following sequences: a single carriage-return (CR, ASCII code `0x0D`)
 *    character, a single line-feed (LF, ASCII code `0x0A`) character or a
 *    sequence of the two characters CR-LF.
 *
 * 2- Leading and trailing spaces in a command string are ignored (trailing
 *    spaces may occur before the EOL sequence if any but not after).  Space
 *    characters are ordinary spaces `' '` (ASCII code `0x20`) or tabulations
 *    `'\t'` (ASCII code `0x09`).
 *
 * 3- Words are separated by one or more space characters.  A word is either a
 *    sequence of contiguous ordinary characters (non-space, non-quote,
 *    non-escape, non-forbidden) or a quoted string (see next).
 *
 * 6- Strings start with a quoting character (either a single or a double
 *    quote) and end with the same quoting character.  The opening and closing
 *    quotes are not part of the resulting word.  There must be at least one
 *    space character after (respectively before) the openning (respectively
 *    closing) quote if the string is not the first (respectively last) word of
 *    the sequence.  That is, quotes cannot be part of non-quoted words and are
 *    therefore not considered as ordinary characters.  There are 2 forms of
 *    quoted strings: strings enclosed in single quotes are extracted literaly
 *    (there may be double quotes inside but no single quotes and the escape
 *    character is considered as an ordinary character in literal strings) and
 *    strings enclosed in double quotes which may contain escape sequence to
 *    represent some special characters.  The following escape sequences are
 *    allowed and recognized in double quoted strings (any other occurrences of
 *    the escape character is considered as an error):
 *
 *   - `\t` yields an horizontal tabulation character;
 *   - `\n` yields a line-feed character;
 *   - `\r` yields a carriage-return character;
 *   - `\"` yields a double-quote character;
 *   - `\\` yields a backslash character.
 *
 * Thus quoted strings can have embedded spaces.  To have a zero-length word, a
 * quoted string like `''` or `""` must be used.
 *
 * The following errors may occur:
 *
 *  - Illegal characters: Anywhere but at the end of the command string, CR and
 *    LF characters are considered as an error.  This is because LF are used to
 *    separate successive commands in communication channels.  The null
 *    character must not appear in the command string (as it serves as end of
 *    string marker).
 *
 *  - Successive quoted strings not separated by a space.  Quotes appearing in
 *    a non-quoted word.  Unclosed quoted strings.  Unterminated escape
 *    sequences.
 *
 * @{
 */

/**
 * Split command in individual words.
 *
 * @param list   Address of a variable to store the list.  The list and its
 *               constitutive words are stored in a single block of memory.
 * @param cmd    Command line string to split.
 * @param len    Optional length of @b cmd.  If @b len is nonnegative, it is
 *               assumed to give the number of characters of the command line
 *               (which must not then be null-terminated).  Otherwise, @b len
 *               may be equal to `-1` and the command line must be
 *               null-terminated.
 *
 * The caller is responsible for properly initializing the list pointer to
 * `NULL` and calling free() when the list is no longer needed.  The list
 * argument can be re-used several times to parse different command lines.
 *
 * @fixme What are the advantages of this kind of allocation?  This is only
 * useful if we can avoid reallocating such lists.  There is no portable way to
 * query the size of a malloc'ed block of memory given its address.  The only
 * possibility is to make a hack an allocate more bytes to store the size
 * (however beware of alignment).  On my laptop, malloc() and free() of blocks
 * or random sizes (between 1 and 10,000 bytes) takes less than 100ns on
 * average.
 *
 * @return The number of words in the list; `-1` in case of failure.
 */
extern int tao_split_command(
    const char*** list,
    const char* cmd,
    long len);

/**
 * Pack words into a command-line.
 *
 * This function assembles given words into a single command-line.  This
 * function does the opposite of tao_unpack_words().
 *
 * @param dest   Address of an i/o buffer to store the result.  The resulting
 *               command-line is appended to any existing contents of @b dest.
 * @param argv   List of words.  The elements of the list must be non-null
 *               pointers to ordinary C-strings terminated by a null character.
 *               If @b argc is equal to `-1`, the list must have one more
 *               pointer then the number of words, this last pointer being set
 *               to `NULL` to mark the end of the list.
 * @param argc   Optional number of words in @b argv.  If @b argc is
 *               nonnegative, it is assumed to give the number of words in the
 *               list.  Otherwise, @b argc can be equal to `-1` to indicate
 *               that the first null-pointer in @b argv marks the end of the
 *               list.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @note In case of failure, the contents of @b dest existing prior to the call
 * is untouched but its location may have change.
 *
 * @see tao_unpack_words.
 */
extern tao_status tao_pack_words(
    tao_buffer* dest,
    const char* argv[],
    int argc);

/**
 * Read an `int` value in a word.
 *
 * This function is intended to parse an integer value from a word as obtained
 * by splitting commands with tao_split_command().  To be valid, the word must
 * contains single integer in a human redable form and which fits in an `int`.
 *
 * @param str    Input word to parse.
 * @param ptr    Address to write parsed value.  This value is left unchanged
 *               in case of failure.
 * @param base   Base to use for conversion, `0` to apply C-style conventions.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.  Whatever
 *         the result, the caller's last error is left unchanged.
 */
extern tao_status tao_parse_int(
    const char* str,
    int* ptr,
    int base);

/**
 * Read a `long` value in a word.
 *
 * This function is intended to parse an integer value from a word as obtained
 * by splitting commands with tao_split_command().  To be valid, the word must
 * contains single integer in a human redable form and which fits in a `long`.
 *
 * @param str    Input word to parse.
 * @param ptr    Address to write parsed value.  This value is left unchanged
 *               in case of failure.
 * @param base   Base to use for conversion, `0` to apply C-style conventions.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.  Whatever
 *         the result, the caller's last error is left unchanged.
 */
extern tao_status tao_parse_long(
    const char* str,
    long* ptr,
    int base);

/**
 * Read a `double` value in a word.
 *
 * This function is intended to parse a floating point value from a word as
 * obtained by splitting commands with tao_split_command().  To be valid, the
 * word must contains single floating value in a human redable form.
 *
 * @param str    Input word to parse.
 * @param ptr    Address to write parsed value.  This value is left unchanged
 *               in case of failure.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.  Whatever
 *         the result, the caller's last error is left unchanged.
 */
extern tao_status tao_parse_double(
    const char* str,
    double* ptr);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_UTILS_H_
