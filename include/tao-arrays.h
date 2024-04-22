// tao-arrays.h -
//
//  Definitions for multi-dimensional arrays in TAO.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_ARRAYS_H_
#define TAO_ARRAYS_H_ 1

#include <tao-basics.h>
#include <tao-encodings.h>

TAO_BEGIN_DECLS

/**
 * @defgroup SimpleArrays Simple arrays
 *
 * @ingroup Arrays
 *
 * @brief Multi-dimensional arrays in conventional memory
 *
 * Multi-dimensional arrays have homogeneous element type and may have up to
 * @ref TAO_MAX_NDIMS dimensions.  Elements of a multi-dimensional array are
 * contiguous in memory and are stored in
 * [column-major](https://en.wikipedia.org/wiki/Row-_and_column-major_order)
 * order (that is the index along the first dimension varies the fastest).
 *
 * @{
 */

/**
 * Region of interest (ROI) in an image.
 *
 * A region of interest (ROI for short) is defined with respect to a *parent*.
 * This parent can be another ROI, the sensor of a camera, an image, etc.
 */
typedef struct tao_image_roi {
    long xoff;    ///< Horizontal offset with respect to parent
    long yoff;    ///< Vertical offset with respect to parent
    long width;   ///< Horizontal size
    long height;  ///< Vertical size
} tao_image_roi;

/**
 * Count the number of elements of a multi-dimensional array.
 *
 * This function counts the number of elements of a multi-dimensional array
 * given its dimensions checking for the validity of the arguments and for
 * integer overflow.
 *
 * @param ndims  Number of dimensions of the array.
 * @param dims   Lengths of the dimensions of the array.
 *
 * @return The product of the dimensions in @b dims, assuming that a
 *         zer-dimensional array has just one element.  Normally, this value is
 *         at least `1`; `0` is returned in case of failure.
 *
 * @see tao_unreference_array(), tao_wrap_array(), tao_get_array_eltype(),
 *      tao_get_array_length(), tao_get_array_ndims(), tao_get_array_dim(),
 *      tao_get_array_data().
 */
extern long tao_count_elements(
    int ndims,
    const long dims[]);

/**
 * Maximun number of dimensions of (shared) arrays.
 */
#define TAO_MAX_NDIMS 5

/**
 * Opaque structure for a multi-dimensional array.
 */
typedef struct tao_array tao_array;

/**
 * Create a new array.
 *
 * This function creates a new multi-dimensional array.  The returned array has
 * a reference count of 1, the caller is responsible for unreferencing the
 * array when no longer needed by calling tao_unreference_array().  All the
 * contents of the returned array is stored in a single block of dynamic
 * memory, the first element is stored at an address aligned so as to allow for
 * fast [vectorized
 * operations](https://fr.wikipedia.org/wiki/Single_instruction_multiple_data).
 *
 * @param eltype Type of the elements of the array.
 * @param ndims  Number of dimensions of the array.
 * @param dims   Lengths of the dimensions of the array.
 *
 * @return The address of a new array; `NULL` in case of failure.
 *
 * @see tao_unreference_array(), tao_wrap_array(), tao_get_array_eltype(),
 *      tao_get_array_length(), tao_get_array_ndims(), tao_get_array_dim(),
 *      tao_get_array_data().
 */
extern tao_array* tao_create_array(
    tao_eltype eltype,
    int ndims,
    const long dims[]);

/**
 * Create a new mono-dimensional array.
 *
 * This function creates a new mono-dimensional array.  The returned array has
 * a reference count of 1, the caller is responsible for unreferencing the
 * array when no longer needed by calling tao_unreference_array().
 *
 * @param eltype Type of the elements of the array.
 * @param dim    Length of the array.
 *
 * @return The address of a new array; `NULL` in case of failure.
 *
 * @see tao_create_array(), tao_unreference_array().
 */
extern tao_array* tao_create_1d_array(
    tao_eltype eltype,
    long dim);

/**
 * Create a new two-dimensional array.
 *
 * This function creates a new two-dimensional array.  The returned array has a
 * reference count of 1, the caller is responsible for unreferencing the array
 * when no longer needed by calling tao_unreference_array().
 *
 * @param eltype Type of the elements of the array.
 * @param dim1   Length of the first dimension.
 * @param dim2   Length of the second dimension.
 *
 * @return The address of a new array; `NULL` in case of failure.
 *
 * @see tao_create_array(), tao_unreference_array().
 */
extern tao_array* tao_create_2d_array(
    tao_eltype eltype,
    long dim1,
    long dim2);

/**
 * Create a new three-dimensional array.
 *
 * This function creates a new three-dimensional array.  The returned array has
 * a reference count of 1, the caller is responsible for unreferencing the
 * array when no longer needed by calling tao_unreference_array().
 *
 * @param eltype Type of the elements of the array.
 * @param dim1   Length of the first dimension.
 * @param dim2   Length of the second dimension.
 * @param dim3   Length of the third dimension.
 *
 * @return The address of a new array; `NULL` in case of failure.
 *
 * @see tao_create_array(), tao_unreference_array().
 */
extern tao_array* tao_create_3d_array(
    tao_eltype eltype,
    long dim1,
    long dim2,
    long dim3);

/**
 * Wrap existing data into a multi-dimensional array.
 *
 * This function creates a multi-dimensional array whose elements are stored in
 * a provided memory area.  The returned array has a reference count of 1, the
 * caller is responsible for unreferencing the array when no longer needed by
 * calling tao_unreference_array().  When the array is eventually destroyed,
 * the callback @b free is called with the context argument @b ctx.
 *
 * @param eltype Type of the elements of the array.
 * @param ndims  Number of dimensions of the array.
 * @param dims   Lengths of the dimensions of the array.
 * @param data   Address of the first element of the array in memory.
 * @param free   Function to call to release the provided resources.
 * @param ctx    Argument of @b free to release the provided resources.
 *
 * @return The address of a new wrapped array; `NULL` in case of failure.
 *
 * @see tao_unreference_array(), tao_create_array(), tao_get_array_eltype(),
 *      tao_get_array_length(), tao_get_array_ndims(), tao_get_array_dim(),
 *      tao_get_array_data().
 */
extern tao_array* tao_wrap_array(
    tao_eltype eltype,
    int ndims,
    const long dims[],
    void* data,
    void (*free)(void*),
    void* ctx);

/**
 * Wrap existing data into a mono-dimensional array.
 *
 * This function creates a mono-dimensional array whose elements are stored in
 * a provided memory area.  The returned array has a reference count of 1, the
 * caller is responsible for unreferencing the array when no longer needed by
 * calling tao_unreference_array().  When the array is eventually destroyed,
 * the callback @b free is called with the context argument @b ctx.
 *
 * @param eltype Type of the elements of the array.
 * @param dim    Length of the mono-dimensional array.
 * @param data   Address of the first element of the array in memory.
 * @param free   Function to call to release the provided resources.
 * @param ctx    Argument of @b free to release the provided resources.
 *
 * @return The address of a new wrapped array; `NULL` in case of failure.
 *
 * @see tao_wrap_array(), tao_unreference_array().
 */
extern tao_array* tao_wrap_1d_array(
    tao_eltype eltype,
    long dim,
    void* data,
    void (*free)(void*),
    void* ctx);

/**
 * Wrap existing data into a two-dimensional array.
 *
 * This function creates a two-dimensional array whose elements are stored in a
 * provided memory area.  The returned array has a reference count of 1, the
 * caller is responsible for unreferencing the array when no longer needed by
 * calling tao_unreference_array().  When the array is eventually destroyed,
 * the callback @b free is called with the context argument @b ctx.
 *
 * @param eltype Type of the elements of the array.
 * @param dim1   Length of the first dimension.
 * @param dim2   Length of the second dimension.
 * @param data   Address of the first element of the array in memory.
 * @param free   Function to call to release the provided resources.
 * @param ctx    Argument of @b free to release the provided resources.
 *
 * @return The address of a new wrapped array; `NULL` in case of failure.
 *
 * @see tao_wrap_array(), tao_unreference_array().
 */
extern tao_array* tao_wrap_2d_array(
    tao_eltype eltype,
    long dim1,
    long dim2,
    void* data,
    void (*free)(void*),
    void* ctx);

/**
 * Wrap existing data into a three-dimensional array.
 *
 * This function creates a three-dimensional array whose elements are stored in
 * a provided memory area.  The returned array has a reference count of 1, the
 * caller is responsible for unreferencing the array when no longer needed by
 * calling tao_unreference_array().  When the array is eventually destroyed,
 * the callback @b free is called with the context argument @b ctx.
 *
 * @param eltype Type of the elements of the array.
 * @param dim1   Length of the first dimension.
 * @param dim2   Length of the second dimension.
 * @param dim3   Length of the third dimension.
 * @param data   Address of the first element of the array in memory.
 * @param free   Function to call to release the provided resources.
 * @param ctx    Argument of @b free to release the provided resources.
 *
 * @return The address of a new wrapped array; `NULL` in case of failure.
 *
 * @see tao_wrap_array(), tao_unreference_array().
 */
extern tao_array* tao_wrap_3d_array(
    tao_eltype eltype,
    long dim1,
    long dim2,
    long dim3,
    void* data,
    void (*free)(void*),
    void* ctx);

/**
 * Add a reference to an existing multi-dimensional array.
 *
 * This function increments the reference count of a multi-dimensional array
 * by one.  The caller is responsible for eventually unreferencing the array by
 * calling tao_unreference_array().
 *
 * @param arr Pointer to an array (must not be `NULL`).
 *
 * @return The address of the array @b arr.
 *
 * @see tao_create_array(), tao_wrap_array(), tao_unreference_array().
 */
extern tao_array* tao_reference_array(
    tao_array* arr);

/**
 * Drop a reference from a multi-dimensional array.
 *
 * This function decrements the reference count of a multi-dimensional array by
 * one.  When the reference count reach the value of zero the multi-dimensional
 * array is effectively destroyed.
 *
 * @param arr    Pointer to an array referenced by the caller.
 *
 * @see tao_create_array(), tao_wrap_array(), tao_reference_array().
 */
extern void tao_unreference_array(
    tao_array* arr);

/**
 * Get the type of elements of an array.
 *
 * @param arr    Pointer to an array referenced by the caller.
 *
 * @return One of the `tao_eltype` values.
 */
extern tao_eltype tao_get_array_eltype(
    const tao_array* arr);

/**
 * Get the number of elements of an array.
 *
 * @param arr    Pointer to an array referenced by the caller.
 *
 * @return The number of elements in the array.
 */
extern long tao_get_array_length(
    const tao_array* arr);

/**
 * Get the number of dimensions of an array.
 *
 * @param arr    Pointer to an array referenced by the caller.
 *
 * @return The number of dimensions of the array.
 */
extern int tao_get_array_ndims(
    const tao_array* arr);

/**
 * Get the length of a dimension of an array.
 *
 * All dimensions beyond the number of dimensions of @b arr are assumed to have
 * unit length.
 *
 * @param arr    Pointer to an array referenced by the caller.
 * @param d      Index of dimension of interest (`1` is the first dimension).
 *
 * @return The number of elements along the given dimension if @b d is greater
 *         or equal `1` and less or equal the number of dimensions of @b arr;
 *         `0` if @b d is less than `1` and `1` if @b d is greater than the
 *         number of dimensions of @b arr.
 */
extern long tao_get_array_dim(
    const tao_array* arr, int d);

/**
 * Get the address of the first element of an array.
 *
 * Elements of a multi-dimensional array are contiguous in memory and are
 * stored in
 * [column-major](https://en.wikipedia.org/wiki/Row-_and_column-major_order)
 * order (that is the index along the first dimension varies the fastest).
 *
 * @param arr    Pointer to an array referenced by the caller.
 *
 * @return The address of the first element of the array.
 */
extern void* tao_get_array_data(
    const tao_array* arr);

/**
 * @}
 */

//-----------------------------------------------------------------------------

/**
 * @defgroup ArrayTools  Array tools
 *
 * @ingroup Arrays
 *
 * @brief Useful functions for multi-dimensional arrays.
 *
 * @{
 */

/**
 * Set a region into an array.
 */
extern tao_status tao_copy_to_array(
    tao_array*  restrict dst,
    const long* restrict dstoffs,
    const void* restrict srcdata,
    tao_eltype           srctype,
    const long* restrict srcdims,
    const long* restrict srcoffs,
    const long* restrict lens,
    int                  ndims);

/**
 * Extract a region from an array.
 */
extern tao_status tao_copy_from_array(
    void*       restrict dstdata,
    tao_eltype           dsttype,
    const long* restrict dstdims,
    const long* restrict dstoffs,
    tao_array*  restrict src,
    const long* restrict srcoffs,
    const long* restrict lens,
    int                  ndims);

/**
 * Copy a region of an array into another array.
 */
extern tao_status tao_copy_array_to_array(
    tao_array*  restrict dst,
    const long* restrict dstoffs,
    tao_array*  restrict src,
    const long* restrict srcoffs,
    const long* restrict lens,
    int                  ndims);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_ARRAYS_H_
