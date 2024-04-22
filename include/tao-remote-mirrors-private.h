// tao-remote-mirrors-private.h -
//
// Private definitions for remote mirrors in TAO library.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_REMOTE_MIRRORS_PRIVATE_H_
#define TAO_REMOTE_MIRRORS_PRIVATE_H_ 1

#include <tao-remote-objects-private.h>
#include <tao-remote-mirrors.h>

TAO_BEGIN_DECLS

/**
 * Structure storing the data shared by a deformable mirror.
 *
 * A remote mirror instance is used to communicate with the server owning the
 * deformable mirror, it includes work-spaces to store the actuators commands,
 * reference values, perturbation values, etc., the actuators layout, and a
 * cyclic list of output buffers with the output data-frames (telemetry).
 *
 * In shared memory, the data are ordered as follows:
 *
 * - The base `tao_remote_object` structure.
 *
 * - Members specific to a remore deformable mirror.
 *
 * - The actuators layout (an array of `dims[0]*dims[1]` indices).
 *
 * - The actuators reference (an array of `nacts` double precision
 *   floating-point values).
 *
 * - The actuators perturbation (an array of `nacts` double precision
 *   floating-point values).
 *
 * - The requested actuators commands (an array of `nacts` double precision
 *   floating-point values).
 *
 * - The actual actuators commands (an array of `nacts` double precision
 *   floating-point values) which accounts for the deformable mirror
 *   limitations.
 *
 * - The output data-frames (a cyclic list of `base.nbufs` buffers of maximal
 *   size `base.stride` and starting at `base.offset` bytes from the base
 *   address of the structure.
 */
struct tao_remote_mirror {
    tao_remote_object   base;///< Common part for all shared objects.
    const long         nacts;///< Number of actuators.
    const long       dims[2];///< Dimensions of actuator grid.
    const size_t vals_offset;///< Offset to actuators reference (in bytes).
    const double        cmin;///< Minimal value for an actuator command.
    const double        cmax;///< Maximal value for an actuator command.
    tao_serial          mark;///< Serial number of last data-frame.
    // Must be last member, actual size is `base.nacts`.
    const long        inds[];///< Indices of the actuators layout.
};

TAO_END_DECLS

#endif // TAO_REMOTE_MIRRORS_PRIVATE__H_
