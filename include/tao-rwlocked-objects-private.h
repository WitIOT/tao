// tao-rwlocked-objects-private.h -
//
// Private definitions for TAO read/write locked objects.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2018-2022, Éric Thiébaut.

#ifndef TAO_RWLOCKED_OBJECTS_PRIVATE_H_
#define TAO_RWLOCKED_OBJECTS_PRIVATE_H_ 1

#include <tao-shared-objects-private.h>
#include <tao-rwlocked-objects.h>

TAO_BEGIN_DECLS

struct tao_rwlocked_object {
    tao_shared_object base;///< Base structure.
    int64_t        writers;///< Number of waiting writers.  Must be
                           ///  nonnegative.
    int64_t          users;///< Number of active users: 0 if none, > 0 if some
                           ///  active readers, -1 is one active writer.
                           ///  Anything else is an error.
};

TAO_END_DECLS

#endif // TAO_RWLOCKED_OBJECTS_PRIVATE_H_
