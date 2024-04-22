// tao-shared-objects-private.h -
//
// Private definitions for TAO shared objects.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2018-2022, Éric Thiébaut.

#ifndef TAO_SHARED_OBJECTS_PRIVATE_H_
#define TAO_SHARED_OBJECTS_PRIVATE_H_ 1

#include <tao-shared-objects.h>

TAO_BEGIN_DECLS

struct tao_shared_object {
    tao_mutex       mutex;///< Mutually exclusive lock to control access.
    tao_cond         cond;///< Condition variable to signal or wait for
                          ///  changes.

    // The number of attachments is an atomic variable so that it can be
    // fetched and incremented or decremented atomically to determine whether
    // the object has to be destroyed without locking the structure.
    tao_atomic
    int64_t         nrefs;///< Number of attachments.  When this becomes zero,
                          ///  the shared data is about to be destroyed.
    const size_t     size;///< Total number of bytes allocated for the shared
                          ///  memory segment.
    const tao_shmid shmid;///< Shared memory identifier.
    const uint32_t  flags;///< Options and granted access permissions.
    const uint32_t   type;///< Object type (or class) identifier.
};

TAO_END_DECLS

#endif // TAO_SHARED_OBJECTS_PRIVATE_H_
