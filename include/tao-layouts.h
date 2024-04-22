// tao-layouts.h -
//
// Definitions and API for 2-dimensional layouts of active nodes in TAO.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2022, Éric Thiébaut.

#ifndef TAO_LAYOUTS_H_
#define TAO_LAYOUTS_H_ 1

#include <tao-basics.h>

TAO_BEGIN_DECLS

/**
 * @addtogroup Laoyuts Two-dimensional layouts of active nodes.
 *
 * @ingroup Utilities
 *
 * @brief Common types and methods for 2-dimensional layouts of active nodes.
 *
 * @{
 */

/**
 * Build an indexed layout given a mask.
 *
 * This function computes the number and the indices of active nodes in a 2-D
 * array.
 *
 * The input mask `msk` is a `dim1` by `dim2` array of bytes where active nodes
 * have a non-zero value.
 *
 * The destination array is a `dim1` by `dim2` array of integers where active
 * nodes are set with their nonnegative indices while inactive nodes are set to
 * `-1`.  The active indices are unique and vary from `0` to `n - 1` with `n`
 * the number of active nodes.
 *
 * The least significant bits of `orient` specify how to order the numbering of
 * the active nodes:
 *
 * - If the 1st bit of `orient` is set, the numbering is decreasing along the
 *   1st dimension; otherwise, the numbering is increasing along the 1st
 *   dimension.
 *
 * - If the 2nd bit of `orient` is set, the numbering is decreasing along the
 *   2nd dimension; otherwise, the numbering is increasing along the 2nd
 *   dimension.
 *
 * - If the 3rd bit of `orient` is set, the numbering is in row-major order;
 *   otherwise, the numbering is in column-major order.
 *
 * The mask and the destination array are assumed to have column-major storage
 * order.
 *
 * If the destination array is `NULL`, indices are not stored.  This is useful
 * to count the number of active nodes.
 *
 * @param inds    Destination array of indices (may be `NULL`).
 *
 * @param msk     Mask of active nodes.
 *
 * @param dim1    The first dimension of the layout grid.
 *
 * @param dim2    The second dimension of the layout grid.
 *
 * @param orient  Orientation of the numbering.
 *
 * @return The number `n ≥ 0` of active nodes; `-1` in case of errors.
 */
extern long tao_indexed_layout_build(
    long* inds,
    const uint8_t* msk,
    long dim1,
    long dim2,
    unsigned int orient);

/**
 * Check a 2-dimensional indexed layout.
 *
 * To be valid, the indices of active nodes must be strictly less than the
 * number of active nodes.
 *
 * @param inds    The indices of nodes.
 *
 * @param dim1    The first dimension of the grid of nodes.
 *
 * @param dim2    The second dimension of the grid of nodes.
 *
 * @return The number of active nodes in the layout; -1 in case of errors.
 */
extern long tao_indexed_layout_check(
    const long* inds,
    long dim1,
    long dim2);

/**
 * Create a mask of active nodes given a human readable shape.
 *
 * This function create a layout mask given an array of strings like:
 *
 * ~~~~~{.c}
 * static char const* shape[] = {
 *     "  xxxxxx  ",
 *     " xxxxxxxx ",
 *     "xxxxxxxxxx",
 *     "xxxx  xxxx",
 *     "xxx    xxx",
 *     "xxx    xxx",
 *     "xxxx  xxxx",
 *     "xxxxxxxxxx",
 *     " xxxxxxxx ",
 *     "  xxxxxx  ",
 *     NULL,
 * };
 * ~~~~~
 *
 * where the non-space characters indicate the location of the active nodes.
 *
 * The caller is responsible of eventually calling `free` to release the
 * dynamic memory storing the returned mask.
 *
 * @param shape   An array of strings.
 *
 * @param nrows   The number of strings in `shape`.  If `nrows = -1`, it is
 *                assumed that the end of the list is marked by a `NULL` entry
 *                in `shape` (as in the example).
 *
 * @param dims    An optional array of 2 integers to store the dimensons of
 *                the result.
 *
 * @return The address of the mask, `NULL` in case of failure.
 */
extern uint8_t* tao_layout_mask_create_from_text(
    char const* shape[],
    long        nrows,
    long        dims[2]);

/**
 * Create a mask of active nodes in a 2-dimensional layout.
 *
 * The caller is responsible of eventually calling `free` to release the
 * dynamic memory storing the returned mask.
 *
 * @param dim1    The first dimension of the grid of nodes.
 *
 * @param dim2    The second dimension of the grid of nodes.
 *
 * @param nacts   The number of active nodes.
 *
 * @return The address of the mask, `NULL` in case of failure.
 */
extern uint8_t* tao_layout_mask_create(
    long dim1,
    long dim2,
    long nacts);

/**
 * Instantiate a mask of active nodes in a 2-dimensional layout.
 *
 * This function instantiates a mask of `nacts` active nodes in a 2-dimensional
 * grid of `dim1×dim2` evenly spaced nodes. The nodes are assumed centered on
 * the grid cells and the mask is geometrically centeredd on the grid.
 *
 * If the algorithm (described below) fails to found an exact match, the
 * closest approximation is returned.
 *
 * The mask has size `(dim1,dim2)` and its center is at coordinates `(c1,c2)`
 * given by (assuming 0-based indices and floating-point arithmetic):
 *
 *     c1 = (dim1 - 1)/2
 *     c2 = (dim2 - 1)/2
 *
 * The disk is defined by the nodes at integer coordinates `(i1,i2)` whose
 * distance to the center at coordinates `(c1,c2)` is less than some maximal
 * radius `rm`:
 *
 *     sqrt((i1 - c1)^2 + (i2 - c2)^2) ≤ rm
 *
 * which is equivalent to
 *
 *     f(i1,i2) ≥ c1^2 + c2^2 - rm^2
 *
 * with
 *
 *    f(i1,i2) = (q1 - i1)*i1 + (q2 - i2)*i2
 *    q1 = 2*c1 = dim1 + 1
 *    q2 = 2*c2 = dim2 + 1
 *
 * Since `q1` and `q2` are integers, `f(i1,i2)` in the above inequality is
 * integer.  Hence the mask is defined by:
 *
 *     f(i1, i2) ≥ t
 *
 * where `t` is an integer threshold `t` which depends on the number `nacts` of
 * active nodes.  The smaller `t`, the larger `nacts`.
 *
 * Since the inactive nodes are in the corners, it is faster to start by the
 * smallest possible `t` and then augment `t` until a match is found.  The
 * minimal value of `f(i1,i2)` is at the corners:
 *
 *     fmin = f(dim1-1,dim2-1) = f(0,0) = f(dim1-1,0) = f(0,dim2-1)
 *          = dim1 + dim2
 *
 * @param mask    The destination mask (of size `dim1*dim2`).
 *
 * @param dim1    The first dimension of the grid of nodes.
 *
 * @param dim2    The second dimension of the grid of nodes.
 *
 * @param nacts   The number of active nodes.
 *
 * @param work    An optional workspace to store `f(i1,i2)`.  If non-`NULL` must
 *                have at least `dim1*dim2` nodes.
 *
 * @return The address of the mask, `NULL` in case of failure.
 */
extern uint8_t* tao_layout_mask_instantiate(
    uint8_t* mask,
    long     dim1,
    long     dim2,
    long     nacts,
    long*    work);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_LAYOUTS_H_
