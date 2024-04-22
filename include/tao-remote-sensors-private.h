// tao-remote-sensors-private.h -
//
// Private definitions for remote wavefront sensors in TAO library.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_REMOTE_SENSORS_PRIVATE_H_
#define TAO_REMOTE_SENSORS_PRIVATE_H_ 1

#include <tao-remote-objects-private.h>
#include <tao-remote-sensors.h>

TAO_BEGIN_DECLS

/**
 * Structure describing the complete configuration of a wavefront sensor.
 */
typedef struct tao_remote_sensor_config {
    tao_shackhartmann_config base;
    struct {
        long width;
        long height;
        tao_shmid shmid;
        char owner[TAO_OWNER_SIZE];
    } camera;
    const long     max_ninds;///< Maximum number of nodes in sub-image grid.
    const long     max_nsubs;///< Maximum number of sub-images.
    const size_t subs_offset;///< Offset to the sub-image definitions.

    // Last member, actual size large enough for `max_ninds` elements.
    long inds[1];///< Layout indices.
} tao_remote_sensor_config;

// This structure has a fixed size part followed by two variable size parts:
// the layout of the sub-images grid and the list of sub-images definitions.
// Since the structure is meant to be stored in shared memory, offsets relative
// to the address of the structure (and not absolute addresses in memory) of
// the variable size parts may be in the structure.  If the first variable size
// part immediately (with some padding for correct alignment) follows the fixed
// size part, it can also be the last member of the structure.
//
// Having the offsets relative to the base structure make it possible to copy
// the structure without worries.
struct tao_remote_sensor {
    tao_remote_object          base;///< Common part for all shared objects.
    const long            max_ninds;///< Maximum number of nodes in sub-image
                                    ///  grid.
    const long            max_nsubs;///< Maximum number of sub-images.
    const size_t     config2_offset;///< Offset to the secondary configuration.

    // Last member.
    tao_remote_sensor_config config;///< Primary configuration.
};

/**
 * @brief Data-frame of remote wavefront sensor as stored in shared memory.
 */
typedef struct tao_remote_sensor_dataframe {
    tao_dataframe_header base;
    long nsubs;
    tao_shackhartmann_data data[1];
} tao_remote_sensor_dataframe;

TAO_END_DECLS

#endif // TAO_REMOTE_SENSORS_PRIVATE__H_
