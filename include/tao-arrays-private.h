// tao-arrays-private.h -
//
// Private definitions for TAO multi-dimensional arrays.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2018-2022, Éric Thiébaut.

#ifndef TAO_ARRAYS_PRIVATE_H_
#define TAO_ARRAYS_PRIVATE_H_ 1

#include <tao-arrays.h>

TAO_BEGIN_DECLS

/**
 * @ingroup Arrays
 *
 * @brief Private structure to store a multi-dimensional array.
 *
 * The definition of this structure is exposed in `<tao-arrays-private.h>` but
 * its members should be considered as read-only.  It is recommended to use the
 * public API to manipulate a multi-dimensional array (@ref tao_array).
 */
struct tao_array {
    int                nrefs; ///< Number of references on the object.
    int                ndims; ///< Number of dimensions.
    void*               data; ///< Address of first array element.
    long               nelem; ///< Number of elements.
    long dims[TAO_MAX_NDIMS]; ///< Length of each dimension (dimensions beyond
                              ///  `ndims` are assumed to be `1`).
    tao_eltype        eltype; ///< Element type.
    void      (*free)(void*); ///< If non-NULL, function to call to release
                              ///  resources.
    void*                ctx; ///< Context used as argument to free.
};

TAO_END_DECLS

#endif // TAO_ARRAYS_PRIVATE_H_
