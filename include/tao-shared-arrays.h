// tao-shared-arrays.h -
//
// Definitions for shared arrays in TAO library.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_SHARED_ARRAYS_H_
#define TAO_SHARED_ARRAYS_H_ 1

#include <tao-arrays.h>
#include <tao-utils.h>
#include <tao-shared-objects.h>

TAO_BEGIN_DECLS

/**
 * @defgroup SharedArrays  Shared arrays
 *
 * @ingroup Arrays
 *
 * @brief Process-shared multi-dimensional arrays
 *
 * Header file @ref tao-shared-arrays.h provides definitions for
 * multi-dimensional shared arrays in TAO library.  These arrays implement
 * read/write access and their contents can be shared between processes.
 *
 * The elements of a multi-dimensional shared array are all of the same type
 * and are stored contiguously in [colum-major
 * order](https://en.wikipedia.org/wiki/Row-_and_column-major_order).  That is,
 * with their index along the first dimension varying the fastest and their
 * index along the last dimension varying the slowest.
 *
 * Once created, the element type and dimensions of a shared array will remain
 * unchanged.  The values of the elements on a shared array can be modified by
 * the processes providing the shared array is attached to their address space.
 *
 * Like any othe shared objects, each shared array has a unique numerical
 * identifier which is used to retrieve the array and attach it to the address
 * space of the caller.
 *
 * @{
 */

/**
 * @brief Opaque structure to a shared array.
 *
 * Shared arrays are read/write locked objects (see @ref tao_rwlocked_object)
 * used to share multi-dimensional array data in TAO.

 * @see tao_shared_array_.
 */
typedef struct tao_shared_array tao_shared_array;

/**
 * Create a new shared array.
 *
 * This function creates a new multi-dimensional array whose contents can be
 * shared between processes.  The returned array is attached to the address
 * space of the caller which is responsible for detaching the shared array when
 * no longer needed by calling tao_shared_array_detach().
 *
 * @param eltype Type of the elements of the array.
 * @param ndims  Number of dimensions of the array.
 * @param dims   Lengths of the dimensions of the array.
 * @param flags  Options and permissions granted to the group and to the others.
 *
 * @return The address of a new shared array; `NULL` in case of failure.
 *
 * @see tao_shared_object_create.
 */
extern tao_shared_array* tao_shared_array_create(
    tao_eltype eltype,
    int        ndims,
    const long dims[],
    unsigned   flags);

/**
 * Create a new mono-dimensional shared array.
 *
 * This function creates a new mono-dimensional array whose contents can be
 * shared between processes.  The returned array is attached to the address
 * space of the caller which is responsible for detaching the shared array when
 * no longer needed by calling tao_shared_array_detach().
 *
 * @param eltype Type of the elements of the array.
 * @param dim    Length of the array.
 * @param flags  Options and permissions granted to the group and to the others.
 *
 * @return The address of a new shared array; `NULL` in case of failure.
 *
 * @see tao_shared_object_create, tao_shared_array_create.
 */
extern tao_shared_array* tao_shared_array_create_1d(
    tao_eltype eltype,
    long       dim,
    unsigned   flags);

/**
 * Create a new two-dimensional shared array.
 *
 * This function creates a new two-dimensional array whose contents can be
 * shared between processes.  The returned array is attached to the address
 * space of the caller which is responsible for detaching the shared array when
 * no longer needed by calling tao_shared_array_detach().
 *
 * @param eltype Type of the elements of the array.
 * @param dim1   Length of the first dimension.
 * @param dim2   Length of the second dimension.
 * @param flags  Options and permissions granted to the group and to the others.
 *
 * @return The address of a new shared array; `NULL` in case of failure.
 *
 * @see tao_shared_object_create, tao_shared_array_create.
 */
extern tao_shared_array* tao_shared_array_create_2d(
    tao_eltype eltype,
    long       dim1,
    long       dim2,
    unsigned   flags);

/**
 * Create a new three-dimensional shared array.
 *
 * This function creates a new three-dimensional array whose contents can be
 * shared between processes.  The returned array is attached to the address
 * space of the caller which is responsible for detaching the shared array when
 * no longer needed by calling tao_shared_array_detach().
 *
 * @param eltype Type of the elements of the array.
 * @param dim1   Length of the first dimension.
 * @param dim2   Length of the second dimension.
 * @param dim3   Length of the third dimension.
 * @param flags  Options and permissions granted to the group and to the others.
 *
 * @return The address of a new shared array; `NULL` in case of failure.
 *
 * @see tao_shared_object_create, tao_shared_array_create.
 */
extern tao_shared_array* tao_shared_array_create_3d(
    tao_eltype eltype,
    long       dim1,
    long       dim2,
    long       dim3,
    unsigned   flags);

/**
 * @brief Attach an existing shared array to the address space of the caller.
 *
 * This function behaves as tao_shared_object_attach() which to see.  The
 * caller is reponsible of calling tao_shared_array_detach() to detach the
 * array from its address space.
 *
 * @param shmid  Shared memory identifier.
 *
 * @return The address of the shared array in the address space of the caller;
 *         `NULL` in case of failure.
 */
extern tao_shared_array* tao_shared_array_attach(
    tao_shmid shmid);

/**
 * @brief Detach a shared array from the address space of the caller.
 *
 * This function behaves as tao_shared_object_detach() which to see, it must be
 * eventually called for each reference returned by tao_shared_array_detach()
 * or by tao_shared_array_create() (or equivalent).
 *
 * @param arr    Pointer to a shared array attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_shared_array_detach(
    tao_shared_array* arr);

/**
 * @brief Get the size used by a shared array.
 *
 * This function behaves as tao_shared_object_get_size() which to see.
 */
extern size_t tao_shared_array_get_size(
    const tao_shared_array* arr);

/**
 * @brief Get the object type of a shared array.
 *
 * This function behaves as tao_shared_object_get_type() which to see.
 */
extern uint32_t tao_shared_array_get_type(
    const tao_shared_array* arr);

/**
 * @brief Get the unique identifier of a shared array.
 *
 * This function behaves as tao_shared_object_get_shmid() which to see.
 */
extern tao_shmid tao_shared_array_get_shmid(
    const tao_shared_array* arr);

/**
 * Get the type of elements of a shared array.
 *
 * @param arr    Pointer to a shared array attached to the address space of
 *               the caller.
 *
 * @return One of the `tao_eltype` values.
 */
extern tao_eltype tao_shared_array_get_eltype(
    const tao_shared_array* arr);

/**
 * Get the number of elements of a shared array.
 *
 * @param arr    Pointer to a shared array attached to the address space of
 *               the caller.
 *
 * @return The number of elements in the array.
 */
extern long tao_shared_array_get_length(
    const tao_shared_array* arr);

/**
 * Get the number of dimensions of a shared array.
 *
 * @param arr    Pointer to a shared array attached to the address space of
 *               the caller.
 *
 * @return The number of dimensions of the array.
 */
extern int tao_shared_array_get_ndims(
    const tao_shared_array* arr);

/**
 * Get the length of a dimension of a shared array.
 *
 * @param arr    Pointer to a shared array attached to the address space of
 *               the caller.
 * @param d      Index of dimension of interest (`1` is the first dimension).
 *
 * @return The number of elements along the given dimension.
 */
extern long tao_shared_array_get_dim(
    const tao_shared_array* arr,
    int d);

/**
 * Get the address of the first element of a shared array.
 *
 * @param arr    Pointer to a shared array attached to the address space of
 *               the caller.
 *
 * @return The address of the first element of the array.
 */
extern void* tao_shared_array_get_data(
    const tao_shared_array* arr);

/**
 * Fill a shared with a given value.
 *
 * @param arr    Pointer to a shared array attached to the address space of
 *               the caller.
 *
 * @param val    Value.
 *
 * @return The argument @a arr.  This function leaves the caller's last error
 *         unchanged.
 */
extern tao_shared_array* tao_shared_array_fill(
    tao_shared_array* arr,
    double val);

/**
 * Get the serial number of a shared array.
 *
 * @param arr    Pointer to a shared array attached to the address space of
 *               the caller and locked by the caller.
 *
 * @warning Since the serial number is variable, the caller must have locked
 *          the shared array, at least for read-only access.  For efficiency
 *          reasons, this function does not perform error checking and left the
 *          caller's last error unchanged.
 *
 * @return The serial number of the shared array.
 *
 * @see tao_shared_array_rdlock.
 */
extern tao_serial tao_shared_array_get_serial(
    const tao_shared_array* arr);

/**
 * Set the serial number of a shared array.
 *
 * @param arr    Pointer to a shared array attached to the address space of
 *               the caller and locked by the caller.
 *
 * @param cnt    Serial number to set.
 *
 * @warning The caller must have locked the shared array for read-write access.
 *          For efficiency reasons, this function does not perform error
 *          checking and left the caller's last error unchanged.
 *
 * @see tao_shared_array_wrlock.
 */
extern void tao_shared_array_set_serial(
    tao_shared_array* arr,
    tao_serial cnt);

/**
 * @brief Number of shared array time-stamps.
 *
 */
#define TAO_SHARED_ARRAY_TIMESTAMPS 5

/**
 * Get one of the time-stamps of a shared array.
 *
 * The time-stamp of a shared array is divided in two integer parts, one gives
 * the integer number of seconds, the other gives the rest as an integer number
 * of nanoseconds.  Thus the resolution of the time-stamp is one nanosecond
 * at best.
 *
 * @param arr     Pointer to a shared array attached to the address space of
 *                the caller and locked by the caller.
 *
 * @param idx     Index of time-stamp to retrieve (in the range 0 to
 *                @ref TAO_SHARED_ARRAY_TIMESTAMPS - 1).
 *
 * @param ts      Address to store the time-stamp.
 *
 * @warning Since the time-stamp is variable, the caller must have locked the
 *          shared array for reading or for writing.  For efficiency reasons,
 *          this function does not report errors, invalid arguments result in
 *          having @ref TAO_UNKNOWN_TIME stored in @a ts (if not `NULL`).
 *
 * @see tao_shared_array_lock.
 */
extern void tao_shared_array_get_timestamp(
    const tao_shared_array* restrict arr,
    int                              idx,
    tao_time*               restrict ts);

/**
 * Set one of the time-stamps of a shared array.
 *
 * @param arr     Pointer to a shared array attached to the address space of
 *                the caller and locked by the caller.
 *
 * @param idx     Index of time-stamp to set (in the range 0 to
 *                @ref TAO_SHARED_ARRAY_TIMESTAMPS - 1).
 *
 * @param ts      Time-stamp.
 *
 * @warning Since the time-stamp is variable, the caller must have locked the
 *          shared array for writing.  For efficiency reasons, this function
 *          does not report errors (invalid address and index have no
 *          incidence).
 *
 * @see tao_shared_array_lock.
 */
extern void tao_shared_array_set_timestamp(
    tao_shared_array* restrict arr,
    int                        idx,
    const tao_time*   restrict ts);

/**
 * @brief Lock a shared array for read-only access.
 *
 * This function behaves like tao_rwlocked_object_rdlock().
 */
extern tao_status tao_shared_array_rdlock(
    tao_shared_array* arr);

/**
 * @brief Lock a shared array for read-write access.
 *
 * This function behaves like tao_rwlocked_object_wrlock().
 */
extern tao_status tao_shared_array_wrlock(
    tao_shared_array* arr);

/**
 * @brief Attempt to lock a shared array for read-only access without blocking.
 *
 * This function behaves like tao_rwlocked_object_try_rdlock().
 */
extern tao_status tao_shared_array_try_rdlock(
    tao_shared_array* arr);

/**
 * @brief Attempt to lock a shared array for read-write access without
 * blocking.
 *
 * This function behaves like tao_rwlocked_object_try_wrlock().
 */
extern tao_status tao_shared_array_try_wrlock(
    tao_shared_array* arr);

/**
 * @brief Attempt to lock a shared array for read-only access without blocking
 * more than a given duration.
 *
 * This function behaves like tao_rwlocked_object_timed_rdlock().
 *
 * @param arr    Pointer to a shared array attached to the address space of
 *               the caller.
 *
 * @param secs   Maximum time to wait (in seconds).  If this amount of time is
 *               very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling tao_shared_array_rdlock().  If
 *               this amount of time is very short, the effect is the same as
 *               calling tao_shared_array_try_rdlock().
 *
 * @return @ref TAO_OK on success; @ref TAO_TIMEOUT if the shared array cannot
 * be locked before the time limit, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_shared_array_timed_rdlock(
    tao_shared_array* arr,
    double secs);

/**
 * @brief Attempt to lock a shared array for read-write access without blocking
 * more than a given duration.
 *
 * This function behaves like tao_rwlocked_object_timed_wrlock().
 *
 * @param arr    Pointer to a shared array attached to the address space of
 *               the caller.
 *
 * @param secs   Maximum time to wait (in seconds).  If this amount of time is
 *               very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling tao_shared_array_wrlock().  If
 *               this amount of time is very short, the effect is the same as
 *               calling tao_shared_array_try_wrlock().
 *
 * @return @ref TAO_OK on success; @ref TAO_TIMEOUT if the shared array cannot
 * be locked before the time limit, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_shared_array_timed_wrlock(
    tao_shared_array* arr,
    double secs);

/**
 * @brief Attempt to lock a shared array for read-only access without blocking
 * longer than a time limit.
 *
 * This function behaves like tao_rwlocked_object_abstimed_rdlock().
 *
 * @param arr      Pointer to a shared array attached to the address space of
 *                 the caller.
 * @param abstime  Time limit (using `CLOCK_REALTIME` clock).
 *
 * @return @ref TAO_OK on success; @ref TAO_TIMEOUT if the shared array cannot
 * be locked before the time limit, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_shared_array_abstimed_rdlock(
    tao_shared_array* arr,
    const tao_time* abstime);

/**
 * @brief Attempt to lock a shared array for read-write access without blocking
 * longer than a time limit.
 *
 * This function behaves like tao_rwlocked_object_abstimed_wrlock().
 *
 * @param arr      Pointer to a shared array attached to the address space of
 *                 the caller.
 * @param abstime  Time limit (using `CLOCK_REALTIME` clock).
 *
 * @return @ref TAO_OK on success; @ref TAO_TIMEOUT if the shared array cannot
 * be locked before the time limit, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_shared_array_abstimed_wrlock(
    tao_shared_array* arr,
    const tao_time* abstime);

/**
 * Unlock a shared array.
 *
 * This function behaves like tao_rwlocked_object_unlock().
 *
 * @param arr    Pointer to a shared array attached to the address space of
 *               the caller and locked by the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_shared_object_unlock(), tao_shared_array_rdlock(),
 *      tao_shared_array_try_rdlock(), tao_shared_array_timed_rdlock(),
 *      tao_shared_array_abstimed_rdlock(), tao_shared_array_wrlock(),
 *      tao_shared_array_try_wrlock(), tao_shared_array_timed_wrlock(),
 *      tao_shared_array_abstimed_wrlock().
 */
extern tao_status tao_shared_array_unlock(
    tao_shared_array* arr);

/**
 * @}
 */

/**
 * @ingroup ArrayTools
 *
 * @{
 */

/**
 * Set a region into a shared array.
 */
extern tao_status tao_copy_to_shared_array(
    tao_shared_array* restrict dst,
    const long*       restrict dstoffs,
    const void*       restrict srcdata,
    tao_eltype                 srctype,
    const long*       restrict srcdims,
    const long*       restrict srcoffs,
    const long*       restrict lens,
    int                        ndims);

/**
 * Extract a region from a shaded array.
 */
extern tao_status tao_copy_from_shared_array(
    void*             restrict dstdata,
    tao_eltype                 dsttype,
    const long*       restrict dstdims,
    const long*       restrict dstoffs,
    tao_shared_array* restrict src,
    const long*       restrict srcoffs,
    const long*       restrict lens,
    int                        ndims);

/**
 * Copy a region of an array into a shared array.
 */
extern tao_status tao_copy_array_to_shared_array(
    tao_shared_array* restrict dst,
    const long*       restrict dstoffs,
    tao_array*        restrict src,
    const long*       restrict srcoffs,
    const long*       restrict lens,
    int                        ndims);

/**
 * Copy a region of a shared array into an array.
 */
extern tao_status tao_copy_shared_array_to_array(
    tao_array*        restrict dst,
    const long*       restrict dstoffs,
    tao_shared_array* restrict src,
    const long*       restrict srcoffs,
    const long*       restrict lens,
    int                        ndims);

/**
 * Copy a region of a shared array into another shared array.
 */
extern tao_status tao_copy_shared_array_to_shared_array(
    tao_shared_array* restrict dst,
    const long*       restrict dstoffs,
    tao_shared_array* restrict src,
    const long*       restrict srcoffs,
    const long*       restrict lens,
    int                        ndims);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_SHARED_ARRAYS_H_
