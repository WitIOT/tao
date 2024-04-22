// tao-buffers.h -
//
// Definitions for dynamic i/o buffers in TAO.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_BUFFERS_H_
#define TAO_BUFFERS_H_ 1

#include <tao-basics.h>

#include <stdarg.h>
#include <sys/types.h> // needed by POSIX for ssize_t

TAO_BEGIN_DECLS

/**
 * @defgroup IOBuffers I/o buffers
 *
 * @ingroup Utilities
 *
 * @brief Dynamic input/output buffers.
 *
 * I/O buffers are useful to store data of variable size (their contents may be
 * dynamically resized) and which may only be partially transferred during
 * read/write operations.  They are mostly useful for input/output but may be
 * used for other purposes.
 *
 * @{
 */

/**
 * Input/output buffer.
 *
 * This structure is used to buffer input/output (i/o) data.  This structure is
 * exposed so that callers may define static structures.  The members of the
 * structure may however change and users should only use the provided
 * functions to manipulate i/o buffers.
 */
typedef struct tao_buffer {
    char*         data; ///< Dynamic buffer
    size_t        size; ///< Number of allocated bytes
    size_t      offset; ///< Offset of first pending byte
    size_t     pending; ///< Number of pending bytes
    unsigned int flags; ///< Bitwise flags used internally
} tao_buffer;

/**
 * @def TAO_BUFFER_INITIALIZER
 *
 * This macro is intended to be used to initialize statically a variable of
 * type `tao_buffer`.  For instance:
 *
 * ~~~~~{.c}
 * static tao_buffer buf = TAO_BUFFER_INITIALIZER;
 * ~~~~~
 *
 * is an alternative to:
 *
 * ~~~~~{.c}
 * static tao_buffer buf;
 * tao_buffer_initialize(&buf);
 * ~~~~~
 */
#define TAO_BUFFER_INITIALIZER ((tao_buffer){ \
        .data = NULL, .size = 0, .offset = 0, .pending = 0, .flags = 0 })

/**
 * Initialize statically an i/o buffer.
 *
 * Use this function to initialize statically an i/o buffer structure.  When no
 * longer needed, the internal resources which may have been allocated must be
 * released by calling tao_buffer_destroy().  The structure itself will not be
 * freed by tao_buffer_destroy() which will reset its contents as if just
 * initialized instead.
 *
 * @warning Do not call this function on a buffer created by
 *          tao_buffer_create().
 *
 * @param buf   Address of a static i/o buffer structure.
 *
 * @see TAO_BUFFER_INITIALIZER, tao_buffer_create().
 */
extern void tao_buffer_initialize(
    tao_buffer* buf);

/**
 * Create a dynamic i/o buffer.
 *
 * This function creates a new i/o buffer.  Both the container (the buffer
 * structure) and the contents (the data stored by the buffer) will be
 * dynamically allocated.  When no longer needed, the caller is responsible for
 * calling tao_buffer_destroy() to release all the resources allocated for
 * the buffer (that is, the container and the contents).
 *
 * @param size   Initial number of bytes of the buffer (actual number of
 *               bytes may be larger but not smaller).
 *
 * @return The address of the new buffer; `NULL` in case of failure.
 */
extern tao_buffer* tao_buffer_create(
    size_t size);

/**
 * Destroy dynamic resources of an i/o buffer.
 *
 * This function frees any dynamic resources used by the i/o buffer @b buf.
 * If the buffer has been initialized by tao_buffer_initialize(), only
 * the contents of the buffer may be destroyed and the buffer is reset to have
 * an empty contents, just as done by tao_buffer_initialize(), and can
 * be safely re-used.  If the buffer has been created by tao_buffer_create(),
 * the contents and the container (that is, the structure itself) are destroyed
 * and @b buf must no longer be used.
 *
 * @param buf    Address of the i/o buffer to destroy (can be `NULL`).
 */
extern void tao_buffer_destroy(
    tao_buffer* buf);

/**
 * Resize an i/o buffer.
 *
 * This function has to be called to ensure that a given number of unused bytes
 * are available after the end of the contents (the pending data) stored by an
 * i/o buffer.
 *
 * This function checks the consistency of the buffer structure but, otherwise,
 * tries to make the minimal effort to ensure that the requested number of
 * unused bytes are available.  As needed, it may flush or re-allocate the
 * internal data buffer.  The size of the internal data can never decrease when
 * calling this function.
 *
 * @param buf    Address of the i/o buffer.
 *
 * @param cnt    Minimum size (in bytes) of unused space.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_buffer_resize(
    tao_buffer* buf,
    size_t cnt);

/**
 * Flush the contents of an i/o buffer.
 *
 * This function moves the contents (the pending data) of an i/o buffer to the
 * beginning of its internal storage area so as to left the maximum possible
 * amount of unused bytes after the pending bytes.
 *
 * @warning This function is meant to be fast.  It assumes its argument is
 * correct and makes no error checking.
 *
 * @param buf    Address of the i/o buffer.
 */
extern void tao_buffer_flush(
    tao_buffer* buf);

/**
 * Clear the contents of an i/o buffer.
 *
 * This function drops all the contents (the pending data) of an i/o buffer.
 *
 * @warning This function is meant to be fast.  It assumes its argument is
 * correct and makes no error checking.
 *
 * @param buf    Address of the i/o buffer.
 */
extern void tao_buffer_clear(
    tao_buffer* buf);

/**
 * Get the size of the contents of an i/o buffer.
 *
 * This function yields the size of the contents (the pending data) of an i/o
 * buffer.  Use tao_buffer_get_contents() to retrieve the address of the
 * contents of the buffer and tao_buffer_adjust_contents_size() to remove the
 * bytes you may have consumed.

 * @warning This function is meant to be fast.  It assumes its argument is
 * correct and makes no error checking.
 *
 * @param buf    Address of the i/o buffer.
 *
 * @return The number of pending bytes in the buffer.
 *
 * @see tao_buffer_get_contents, tao_buffer_adjust_contents_size.
 */
extern size_t tao_buffer_get_contents_size(
    tao_buffer* buf);

/**
 * Query the contents an i/o buffer.
 *
 * This function yields the address and, optionally the size, of the contents
 * (the pending bytes) stored in an i/o buffer and which has yet not been
 * consumed.  Call tao_buffer_adjust_contents_size() to remove the bytes you
 * may have consumed.
 *
 * @warning The returned information is only valid until no operations that may
 * change the i/o buffer are applied.  The funtions which take a `const
 * tao_buffer*` are safe in that respect.
 *
 * @warning This function is meant to be fast.  It assumes its arguments are
 * correct and makes no error checking.
 *
 * @param buf     Address of the i/o buffer.
 *
 * @param sizptr  Address to store the number of pending bytes.  Can be `NULL`
 *                to not retrieve this information.  The number of pending
 *                bytes is also given by tao_buffer_get_contents_size().
 *
 * @return The address of the first pending byte.
 *
 * @see tao_buffer_get_contents_size, tao_buffer_adjust_contents_size.
 */
extern void* tao_buffer_get_contents(
    const tao_buffer* buf,
    size_t* sizptr);

/**
 * Get the size of the unused space in an i/o buffer.
 *
 * This function returns the number of unused bytes after the contents of an
 * i/o buffer.  These bytes are directly available to add more contents to the
 * buffer.  You may call tao_buffer_resize() to make sure enough unused space
 * is available.  Call tao_buffer_get_unused_part() to retrieve the address of
 * the unused part and then, possibly, tao_buffer_adjust_contents_size() to
 * indicate the number of bytes that have been added.

 * @warning This function is meant to be fast.  It assumes its argument is
 * correct and makes no error checking.
 *
 * @param buf    Address of the i/o buffer.
 *
 * @return The number of unused bytes after the contents of the buffer.
 *
 * @see tao_buffer_resize, tao_buffer_flush, tao_buffer_get_unused_part,
 * tao_buffer_get_total_unused_size, tao_buffer_adjust_contents_size.
 */
extern size_t tao_buffer_get_unused_size(
    const tao_buffer* buf);

/**
 * Get the total number of unused bytes in an i/o buffer.
 *
 * This function yields the total number of unused bytes in an i/o buffer.
 * That is, the number of bytes that would be unused after the contents of the
 * buffer if tao_buffer_flush() have been called.
 *
 * @warning This function is meant to be fast.  It assumes its argument is
 * correct and makes no error checking.
 *
 * @param buf    Address of the i/o buffer.
 *
 * @return The total number of unused bytes in the buffer.
 *
 * @see tao_buffer_flush, tao_buffer_get_unused_size.
 */
extern size_t tao_buffer_get_total_unused_size(
    const tao_buffer* buf);

/**
 * Query the unused data at the end of an i/o buffer.
 *
 * This function yields the address and the size of the unused space at the end
 * an i/o buffer and which can directly be used to append more data.  If you
 * write some bytes in the returned space, call
 * tao_buffer_adjust_contents_size() to change the size of the contents of the
 * i/o buffer.
 *
 * @warning The returned information is only valid until no operations that may
 * change the i/o buffer are applied.  The functions which take a `const
 * tao_buffer*` are safe in that respect.
 *
 * @warning This function is meant to be fast.  It assumes its arguments are
 * correct and makes no error checking.
 *
 * @param buf    Address of the i/o buffer.
 *
 * @param data   Pointer where to store the address of the first unused byte.
 *               Must not be `NULL`,  call tao_buffer_get_unused_size() if you
 *               are only interested in getting the number of unused bytes.
 *
 * @return The number of unused bytes at the end of the internal data buffer.
 */
extern size_t tao_buffer_get_unused_part(
    const tao_buffer* buf,
    void** data);

/**
 * Adjust the size of the contents of an i/o buffer.
 *
 * Call this function to pretend that some bytes have been consumed at the
 * beginning of the contents (the pending data) of an i/o buffer or that some
 * bytes have been added to the end of the contents of the an i/o buffer.
 *
 * No more than the number of pending bytes can be consumed and no more than
 * the number of unused bytes after the pending data can be added.  If the
 * adjustment is too large, nothing is done and the function reports a
 * `TAO_OUT_OF_RANGE` error.
 *
 * @param buf    Address of the i/o buffer.
 *
 * @param adj    Number of bytes to consume (if negative) or add (if positive).
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_buffer_adjust_contents_size(
    tao_buffer* buf,
    ssize_t adj);

/**
 * Read bytes from a file descriptor to an i/o buffer.
 *
 * This functions attempt to read some bytes from a file descriptor and append
 * them to the contents of an i/o buffer.
 *
 * @param buf    Dynamic buffer to collect bytes read.
 *
 * @param fd     File descriptor to read from.
 *
 * @param cnt    Number of bytes to read.
 *
 * @return The number of bytes actually read, `-1` in case of failure.  The
 *         number of bytes read may be different from @b cnt: it may be smaller
 *         if the number of available bytes is insufficient (for instance
 *         because we are close to the end of the file or because the peer did
 *         not write that much on the other end of the bidirectional
 *         communication channel); it may also be larger because, for
 *         efficiency reasons, the function attempts to fill the space
 *         available in the buffer @b buf.
 *
 * @see tao_buffer_write_to_fd.
 */
extern ssize_t tao_buffer_read_from_fd(
    tao_buffer* buf,
    int fd,
    size_t cnt);

/**
 * Write the contents of an i/o buffer to a file descriptor.
 *
 * This function attempts to write the contents (the pending data) of an i/o
 * buffer to a file descriptor.  If some bytes are written, they are "removed"
 * from the contents (the pending data) of the i/o buffer.  A single attempt
 * may not be sufficient to write all contents, tao_buffer_get_contents_size()
 * can be used to figure out whether there are remaining unwritten bytes.
 *
 * @param buf    Dynamic buffer whose contents is to be written.
 *
 * @param fd     File descriptor to write to.
 *
 * @return The number of bytes actually written, `-1` in case of failure.  The
 *         returned value may be zero if the contents of the i/o buffer is
 *         empty or if the file descriptor is marked for being non-blocking and
 *         the operation would block.  To disentangle, call
 *         tao_buffer_get_contents_size() to check whether the contents was
 *         empty.
 *
 * @see tao_buffer_read_from_fd, tao_buffer_get_contents_size.
 */
extern ssize_t tao_buffer_write_to_fd(
    tao_buffer* buf,
    int fd);

/**
 * Append a formatted message to an i/o buffer.
 *
 * This function appends to the contents of the i/o buffer @b buf a formatted
 * message specified by @b format and subsequent arguments in the same way as
 * the sprintf() function.
 *
 * A final null byte is written in the buffer but is not considered as part of
 * the contents.  So that, if only tao_buffer_printf(),
 * tao_buffer_vprintf(), tao_buffer_append_string() or
 * tao_buffer_append_char() are used to build the contents of the buffer (since
 * creation, initialization or last call to tao_buffer_clear()), the size of
 * the buffer is also the length of the string and, thanks to the final null
 * byte, the buffer contents can be used as a regular string.
 *
 * @param buf    Dynamic buffer.
 *
 * @param format Format string.
 *
 * @param ...    Subsequent arguments.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 *
 * @see tao_buffer_get_contents(), tao_buffer_vprintf().
 */
extern tao_status tao_buffer_printf(
    tao_buffer* buf,
    const char* format,
    ...) TAO_FORMAT_PRINTF(2,3);

/**
 * Append a formatted message to an i/o buffer.
 *
 * This function is similar to tao_buffer_printf() except that it is called
 * with a `va_list` instead of a variable number of arguments.
 *
 * @param buf    Dynamic buffer.
 *
 * @param format Format string.
 *
 * @param ap     Arguments to print.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 *
 * @see tao_buffer_get_contents(), tao_buffer_printf().
 */
extern tao_status tao_buffer_vprintf(
    tao_buffer* buf,
    const char* format,
    va_list ap);

/**
 * Append bytes to an i/o buffer.
 *
 * This function appends some bytes to the contents of a dynamic i/o buffer.
 *
 * @param buf    Dynamic buffer.
 *
 * @param ptr    The address of the first byte to append.
 *
 * @param siz    Number of bytes to append.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_buffer_append_bytes(
    tao_buffer* buf,
    const void* ptr,
    long siz);

/**
 * Append a string to an i/o buffer.
 *
 * This function appends a string to the contents of a dynamic i/o buffer.
 * A final null byte is written in the buffer but is not considered as part of
 * the contents (see tao_buffer_printf()).
 *
 * @param buf    Dynamic buffer.
 *
 * @param str    The string to append.
 *
 * @param len    If nonnegative, number of characters to append; otherwise,
 *               the string @a str is assumed null terminated and its length
 *               if givan by calling tao_strlen().
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_buffer_append_string(
    tao_buffer* buf,
    const char* str,
    long len);

/**
 * Append a single character to an i/o buffer.
 *
 * This function appends a single character to the contents of a dynamic i/o
 * buffer.  A final null byte is written in the buffer but is not considered as
 * part of the contents (see tao_buffer_printf()).
 *
 * @param buf    Dynamic buffer.
 *
 * @param c      The character to append.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_buffer_append_char(
    tao_buffer* buf,
    int c);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_BUFFERS_H_
