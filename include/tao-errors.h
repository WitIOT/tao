// tao-errors.h -
//
// Definitions for management of errors in TAO.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_ERRORS_H_
#define TAO_ERRORS_H_ 1

#include <tao-basics.h>
#include <tao-buffers.h>

#include <stdio.h>

TAO_BEGIN_DECLS

/**
 * @defgroup Errors  Errors
 *
 * @ingroup Utilities
 *
 * @brief Management of errors.
 *
 * In case of failure, **error prone functions** in TAO Library call
 * tao_store_error(), tao_store_system_error(), or tao_store_other_error() to
 * memorize the error information on a per-thread basis and yield a result
 * indicating that a failure occurred.
 *
 * Function tao_get_last_error() yields the address of the structure storing
 * the last TAO error that may have occurred in the calling thread.  This
 * address will never change during the thread life.
 *
 * Functions tao_clear_error() and tao_any_errors() cab be used to clear error
 * and check whether some errors occurred.
 *
 * Functions tao_report_error(), tao_report_error_to_stderr(),
 * tao_report_error_to_stream(), and tao_report_error_to_buffer() may be used
 * to report or print an error message in various ways.  Function tao_panic()
 * may be called in case of a fatal error: it reports the last error that
 * occurred in the thread and causes the process to exit.
 *
 * Function tao_set_error_handler() may be used to set the error handler called
 * by tao_report_error() to format and print an error message.  To implement an
 * error handler, function tao_retrieve_error_details() may be called to
 * retrieve the pieces of error information for the different kinds of errors.
 *
 * For fine tuning or to avoid the complexity of calling
 * tao_retrieve_error_details(), function tao_report_error_with_reporter() may
 * be called to report an error in a custom way.
 *
 * At a lower level, functions tao_get_error_reason() and tao_get_error_name()
 * may be called to get a textual explanation or a symbolic name for a given
 * error code of the standard C library or of the TAO library.

 * @see tao_status.
 *
 * @{
 */

/**
 * Error codes.
 *
 * Errors in TAO Library are identified by an integer.  Strictly positive
 * values indicate a system error using the same identifiers as `errno`
 * (defined in the standard header `errno.h`).  Strictly negative values
 * indicate a TAO error.  Zero (that is, `TAO_SUCCESS`) indicates a successful
 * operation.  Do not confuse error code and status (\ref tao_status)
 * returned by many functions of the TAO Library.
 */
typedef enum tao_error_code {
    TAO_SUCCESS               =   0,///< Operation was successful
    TAO_ACQUISITION_RUNNING   =  -1,///< Acquisition running
    TAO_ALREADY_EXIST         =  -2,///< Destination already exists
    TAO_ALREADY_IN_USE        =  -3,///< Resource already in use
    TAO_ASSERTION_FAILED      =  -4,///< Assertion failed
    TAO_BAD_ADDRESS           =  -5,///< Invalid address
    TAO_BAD_ALGORITHM         =  -6,///< Invalid algorithm
    TAO_BAD_ARGUMENT          =  -7,///< Invalid argument
    TAO_BAD_ATTACHMENTS       =  -8,///< Invalid number of attachments
    TAO_BAD_BIAS              =  -9,///< Invalid detector bias
    TAO_BAD_BOUNDING_BOX      = -10,///< Invalid bounding box
    TAO_BAD_BUFFERS           = -11,///< Bad number of buffers
    TAO_BAD_CHANNELS          = -12,///< Invalid number of channels
    TAO_BAD_CHARACTER         = -13,///< Illegal character
    TAO_BAD_COMMAND           = -14,///< Invalid command
    TAO_BAD_CONNECTION        = -15,///< Invalid connection
    TAO_BAD_DEPTH             = -16,///< Invalid bits per pixel
    TAO_BAD_DEVICE            = -17,///< Invalid device
    TAO_BAD_ENCODING          = -18,///< Bad encoding
    TAO_BAD_ESCAPE            = -19,///< Unknown escape sequence
    TAO_BAD_EXPOSURETIME      = -20,///< Invalid exposure time
    TAO_BAD_FANSPEED          = -21,///< Invalid fan speed
    TAO_BAD_FILENAME          = -22,///< Invalid file name
    TAO_BAD_FORGETTING_FACTOR = -23,///< Invalid forgetting factor
    TAO_BAD_FRAMERATE         = -24,///< Invalid acquisition frame rate
    TAO_BAD_GAIN              = -25,///< Invalid detector gain
    TAO_BAD_MAGIC             = -26,///< Invalid magic number
    TAO_BAD_MAX_EXCURSION     = -27,///< Invalid maximum excursion
    TAO_BAD_NAME              = -28,///< Bad parameter name
    TAO_BAD_NUMBER            = -29,///< Invalid number of values
    TAO_BAD_PIXELTYPE         = -30,///< Bad pixel type
    TAO_BAD_PREPROCESSING     = -31,///< Bad pre-processing settings
    TAO_BAD_RANGE             = -32,///< Invalid interval of values
    TAO_BAD_RANK              = -33,///< Invalid number of dimensions
    TAO_BAD_REFERENCE         = -34,///< Invalid reference
    TAO_BAD_RESTORING_FORCE   = -35,///< Invalid restoring force
    TAO_BAD_ROI               = -36,///< Invalid region of interest
    TAO_BAD_SERIAL            = -37,///< Invalid serial number
    TAO_BAD_SIZE              = -38,///< Invalid size
    TAO_BAD_SPEED             = -39,///< Invalid connection speed
    TAO_BAD_STAGE             = -40,///< Invalid or unexpected stage
    TAO_BAD_TEMPERATURE       = -41,///< Invalid temperature
    TAO_BAD_TYPE              = -42,///< Invalid type
    TAO_BAD_VALUE             = -43,///< Invalid parameter value
    TAO_BROKEN_CYCLE          = -44,///< Broken cycle or unordered operations
    TAO_CANT_TRACK_ERROR      = -45,///< Insufficient memory to track errors
    TAO_CORRUPTED             = -46,///< Corrupted structure
    TAO_DESTROYED             = -47,///< Resource has been destroyed
    TAO_EXHAUSTED             = -48,///< Resource exhausted
    TAO_FORBIDDEN_CHANGE      = -49,///< Forbidden change of parameter(s)
    TAO_INEXACT_CONVERSION    = -50,///< Inexact conversion
    TAO_MISSING_SEPARATOR     = -51,///< Separator missing
    TAO_MUST_RESET            = -52,///< Device must be reset
    TAO_NOT_ACQUIRING         = -53,///< Acquisition not started
    TAO_NOT_FOUND             = -54,///< Item not found
    TAO_NOT_LOCKED            = -55,///< Resource not locked by caller
    TAO_NOT_READY             = -56,///< Device not ready
    TAO_NOT_RUNNING           = -57,///< Server or thread is not running
    TAO_NOT_YET_IMPLEMENTED   = -58,///< Not yet implemented
    TAO_NO_DATA               = -59,///< No data available
    TAO_NO_FITS_SUPPORT       = -60,///< Compiled with no FITS support
    TAO_OUT_OF_RANGE          = -61,///< Out of range argument
    TAO_OVERWRITTEN           = -62,///< Contents has been overwritten
    TAO_SYSTEM_ERROR          = -63,///< Unknown system error
    TAO_UNCLOSED_STRING       = -64,///< Unclosed string
    TAO_UNREADABLE            = -65,///< Not readable
    TAO_UNRECOVERABLE         = -66,///< Unrecoverable error
    TAO_UNSUPPORTED           = -67,///< Unsupported feature
    TAO_UNWRITABLE            = -68,///< Not writable
} tao_error_code;

/**
 * Callback to retrieve error details.
 *
 * Such a callback is called to retrieve details about an error not due to a
 * call of a C library function nor to a TAO Library function.  This is useful
 * for interfacing external libraries, which may have their own policy for
 * handling errors, with TAO.
 *
 * @param code     Error code.
 *
 * @param reason   Address of string pointer to store the *reason* of the
 *                 error.
 *
 * @param info     Address of string pointer to store the textual equivalent of
 *                 the error code.
 *
 * The callback shall set the value pointed by @b reason and/or @b info to the
 * address of a static string if the corresponding information can be provided
 * and to `NULL` otherwise.  Any of these pointers may be `NULL` to indicate
 * that the corresponding information is not requested.
 *
 * The provided information is used to print an error message of the form:
 *
 * > $prefix $reason in `$func` [$info]
 *
 * where `$prefix` is usually a string like `"ERROR:"`, `$reason` is the string
 * provided by the callback (or `"Some error occurred"` if `NULL`), `$func` is
 * the name of the function which raised the error and `$info` is the string
 * provided by the callback (or the textual value of error code if `NULL`).
 *
 * The following example shows such a callback:
 *
 * ~~~~~{.c}
 * void get_error_details(int code, const char** reason, const char** info)
 * {
 *     if (reason != NULL) {
 *         *reason = "Some frame-grabber error occurred";
 *     }
 *     if (info != NULL) {
 *         *info = NULL; // textual value of error code will be used
 *     }
 * }
 * ~~~~~
 *
 * @see tao_retrieve_error_details.
 */
typedef void tao_error_getter(
    int code,
    const char** reason,
    const char** info);

/**
 * Structure to store error information.
 *
 * @warning Member @b func must be a static string.
 *
 */
typedef struct tao_error {
    const char*       func; ///< Name of function where error occurred.
    int               code; ///< Numerical identifier of the error.
    tao_error_getter* proc; ///< Callback to retrieve error details.
} tao_error;

/*
 * Initializer for a @ref tao_error structure.
 */
#define TAO_ERROR_INITIALIZER \
    { (const char*)0, TAO_SUCCESS, (tao_error_getter*)0 }

/**
 * Register error due to a foreign function call.
 *
 * This function is called to set the information about the last error that
 * occurred in the calling thread.  This information consist in the name @b
 * func of the function where the error occurred, the numerical identifier @b
 * code of the error, and an optional procedure @b proc to retrieve textual
 * details given the error code.
 *
 * If argument @b proc is `NULL`, it is assumed that the error code follows the
 * convention in TAO Library (nonnegative codes are for system errors while
 * strictly negative codes are for errors in TAO functions); otherwise, @b proc
 * is the callback which can be called to retrieve error details from the error
 * code.
 *
 * @warning @b func must be a static string.
 *
 * @param func   Name of the function where the error occurred.
 *
 * @param code   Error identifier.
 *
 * @param proc   Callback to retrieve information (can be `NULL`).
 */
extern void tao_store_other_error(
    const char* func,
    int code,
    tao_error_getter* proc);

/**
 * Register error due to a function call.
 *
 * This function is equivalent to:
 *
 * ~~~~~{.c}
 * tao_store_other_error(func, code, NULL);
 * ~~~~~
 *
 * @warning @b func must be a static string.
 *
 * @param func   Name of the function where the error occurred.
 *
 * @param code   Error identifier.
 *
 * @see tao_store_other_error().
 */
extern void tao_store_error(
    const char* func,
    int code);

/**
 * Register error due to a system function call.
 *
 * This function is equivalent to:
 *
 * ~~~~~{.c}
 * #include <errno.h>
 * tao_store_error(func, errno);
 * ~~~~~
 *
 * @param func   Name of the function where the error occurred.
 *
 * @see tao_store_error().
 */
extern void tao_store_system_error(
    const char* func);

/**
 * Error handler.
 *
 * An error handler is called by tao_report_error() with the address of the
 * last error that occurred in the calling thread as argument.
 *
 * The error handler shall call tao_retrieve_error_details() to retrieve
 * pieces of error information in textual form.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
typedef tao_status tao_error_handler(
    const tao_error* err);

/**
 * Set error handler.
 *
 * This function sets the error handler of the calling thread and returns the
 * previously installed error handler.  This function always returns a valid
 * result.  The default error handler prints errors to the standard output
 * error stream formatted as `(TAO-ERROR) $ERROR` where `$ERROR` is the error
 * message.
 *
 * The error handler is used by tao_report_error() to report errors, not by
 * tao_panic() which has its own way to print unreported errors.
 *
 * @param func   The new error handler.  If `NULL` the default error handler
 *               is re-installed.
 *
 * @return The error handler previously installed.
 *
 * @see tao_report_error().
 */
extern tao_error_handler* tao_set_error_handler(
    tao_error_handler* func);

/**
 * Report last error.
 *
 * This function calls the error handler of the calling thread to report the
 * last error that occurred in the thread.
 *
 * @see tao_panic(), tao_set_error_handler().
 */
extern void tao_report_error(
    void);

/**
 * Report last error and exit.
 *
 * This function prints the last error that occurred for the calling thread to
 * the standard error stream and then calls `exit(1)`.
 *
 * @see tao_report_error().
 */
extern void tao_panic(
    void) TAO_NORETURN;

/**
 * Print an error to the standard error output stream.
 *
 * This function prints a short message describing the error to the standard
 * error output stream.
 *
 * @param err  Address of error information.  If `NULL`, the last error of the
 *             calling thread is assumed.
 *
 * @param pfx  Prefix for the message.  If `NULL`, '"(TAO-ERROR) "` is assumed.
 *
 * @param sfx  Suffix for the message.  If `NULL`, a newline is assumed.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_report_error(), tao_report_error_to_stream(),
 *      tao_report_error_to_buffer(), tao_report_error_with_reporter().
 */
extern tao_status tao_report_error_to_stderr(
    const tao_error* err,
    const char*      pfx,
    const char*      sfx);

/**
 * Print an error to a given output stream.
 *
 * This function prints a short message describing the error to the given
 * output stream.
 *
 * @param file Output stream, `stderr` is assumed if `NULL`.
 *
 * @param err  Address of error information.  If `NULL`, the last error of the
 *             calling thread is assumed.
 *
 * @param pfx  Prefix for the message.  If `NULL`, '"(TAO-ERROR) "` is assumed.
 *
 * @param sfx  Suffix for the message.  If `NULL`, a newline is assumed.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_report_error(), tao_report_error_to_stderr(),
 *      tao_report_error_to_buffer(), tao_report_error_with_reporter().
 */
extern tao_status tao_report_error_to_stream(
    FILE*            file,
    const tao_error* err,
    const char*      pfx,
    const char*      sfx);

/**
 * Print an error into a dynamic buffer.
 *
 * This function prints a short message describing the error to the given
 * bynamic buffer.
 *
 * @param buf  The address of the output dynamic buffer.
 *
 * @param err  Address of error information.  If `NULL`, the last error of the
 *             calling thread is assumed.
 *
 * @param pfx  Prefix for the message.  If `NULL`, '"(TAO-ERROR) "` is assumed.
 *
 * @param sfx  Suffix for the message.  If `NULL`, an empty string is assumed.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_report_error(), tao_report_error_to_stderr(),
 *      tao_report_error_to_buffer(), tao_report_error_with_reporter().
 */
extern tao_status tao_report_error_to_buffer(
    tao_buffer*      buf,
    const tao_error* err,
    const char*      pfx,
    const char*      sfx);

/**
 * Callback to report errors.
 *
 * This callback is to be used by tao_report_error_with_reporter().
 *
 * @param ctx      Contextual data needed by the callback.
 * @param reason   String describing the error.
 * @param func     Name of the function where the error occurred.
 * @param info     Textual equivalent of the error code.
 * @param code     Code of the error.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
typedef tao_status tao_error_reporter(
    void* ctx,
    const char* reason,
    const char* func,
    const char* info,
    int code);

/**
 * Report an error via a user defined callback.
 *
 * This function may be used to implement an error handler or to report an
 * error in a custom way without the complexity of calling
 * tao_retrieve_error_details().
 *
 * @param reporter    Callback for reporting errors.
 *
 * @param ctx         Contextual data for the callback, this value is provided
 *                    unmodified to the callback.
 *
 * @param err         Address of error information.  If `NULL`, the last error
 *                    of the calling thread is assumed.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_error_reporter, tao_report_error(),
 *      tao_report_error_to_stream(), tao_report_error_to_buffer(),
 *      tao_retrieve_error_details(), tao_set_error_handler().
 */
extern tao_status tao_report_error_with_reporter(
    tao_error_reporter* reporter,
    void* ctx,
    const tao_error* err);

/**
 * Get last error.
 *
 * This function yields the address of the structure storing the last error
 * that may have occurred in the calling thread.  This address will not change
 * during the life of thread.
 *
 * @return The address where is stored the last error of the calling thread,
 *         this address is always valid and will not change during the life of
 *         thread.
 */
extern tao_error* tao_get_last_error(
    void);

/**
 * Clear error information.
 *
 * @param err       Address of the error structure.  If `NULL`, the last error
 *                  of the calling thread is cleared.
 */
extern void tao_clear_error(
    tao_error* err);

/**
 * Check whether errors occurred.
 *
 * @param err       Address of the error structure.  If `NULL`, the last error
 *                  of the calling thread is cleared.
 *
 * @return Non-zero if `err->code` is not `TAO_SUCCESS`; zero otherwise.
 */
extern int tao_any_errors(
    const tao_error* err);

/**
 * Retrieve details about a given error code.
 *
 * @param code      Error code.
 *
 * @param reason    Address of a variable to store the reason of the error.
 *                  Can be `NULL` to not retrieve this information.
 *
 * @param info      Address of a variable to store a textual description of
 *                  the error code.  Can be `NULL` to not retrieve this
 *                  information.
 *
 * @param proc      Callback to retrieve details about an error given its code.
 *                  Can be `NULL` to assume the convention in TAO Library
 *                  (nonnegative codes are for system errors while strictly
 *                  negative codes are for errors in TAO functions).
 *
 * @param buffer    Address of a small text buffer to print the decimal value
 *                  of the error code if no better description can be obtained.
 *                  This buffer is only useful if @b infoptr is not `NULL`.
 *                  Can be `NULL` to not use this fallback; otherwise must have
 *                  at least 20 characters (enough to print any value of a
 *                  64-bit signed integer in decimal form).
 *
 * @see tao_error_getter.
 */
extern void tao_retrieve_error_details(
    int               code,
    const char**      reason,
    const char**      info,
    tao_error_getter* proc,
    char*             buffer);

/**
 * Get error message.
 *
 * This function yields the error message associated to a given error code of
 * the standard C library or of the TAO library.
 *
 * @param code   Error identifier.
 *
 * @return A permanent string.
 *
 * @remark This function is based on `Tcl_ErrnoMsg` in the
 *         [Tcl/Tk](http://www.tcl.tk) library.
 */
extern const char* tao_get_error_reason(
    int code);

/**
 * Get human readable error identifier.
 *
 * Given one of the error codes in the standard C library or in the TAO
 * library, this function returns a string with the symbolic name of the code.
 * For instance, `tao_get_error_name(EINVAL)` yields the string `"EINVAL"`.
 *
 * @param code   Error identifier.
 *
 * @return A permanent string.
 *
 * @remark This function is based on `Tcl_ErrnoId` in the
 *         [Tcl/Tk](http://www.tcl.tk) library.
 *
 * @see tao_get_error_reason.
 */
extern const char* tao_get_error_name(
    int code);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_ERRORS_H_
