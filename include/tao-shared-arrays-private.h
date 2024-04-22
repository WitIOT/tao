// tao-shared-arrays-private.h -
//
// Private definitions for TAO shared arrays.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2018-2022, Éric Thiébaut.

#ifndef TAO_SHARED_ARRAYS_PRIVATE_H_
#define TAO_SHARED_ARRAYS_PRIVATE_H_ 1

#include <tao-rwlocked-objects-private.h>
#include <tao-shared-arrays.h>
#include <tao-arrays.h>
#include <tao-macros.h>

TAO_BEGIN_DECLS

struct tao_shared_array {
    tao_rwlocked_object       base;///< Base structure.
    const long               nelem;///< Number of elements.
    const int                ndims;///< Number of dimensions.
    const long dims[TAO_MAX_NDIMS];///< Length of each dimension (dimensions
                                   ///  beyond `ndims` are assumed to be `1`)
    const tao_eltype        eltype;///< Type of the elements.
    volatile tao_serial     serial;///< Counter (used for posted images).
    volatile tao_time ts[TAO_SHARED_ARRAY_TIMESTAMPS];///< Time stamps.
};

#define TAO_SHARED_ARRAY_DATA_OFFSET \
    TAO_ROUND_UP(sizeof(tao_shared_array), TAO_ALIGNMENT)

#define TAO_SHARED_ARRAY_SHMID(arr)   ((arr)->base.base.shmid) // FIXME: use tao_..._cast
#define TAO_SHARED_ARRAY_ELTYPE(arr)  ((arr)->eltype)
#define TAO_SHARED_ARRAY_NDIMS(arr)   ((arr)->ndims)
#define TAO_SHARED_ARRAY_NELEM(arr)   ((arr)->nelem)
#define TAO_SHARED_ARRAY_DIM(arr, d)  ((arr)->dims[(d)-1])
#define TAO_SHARED_ARRAY_DATA(arr) \
    TAO_COMPUTED_ADDRESS(arr, TAO_SHARED_ARRAY_DATA_OFFSET)

TAO_END_DECLS

#endif // TAO_SHARED_ARRAYS_PRIVATE_H_
