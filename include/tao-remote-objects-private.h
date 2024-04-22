// tao-remote-objects-private.h -
//
// Private definitions for TAO remote objects.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2018-2022, Éric Thiébaut.

#ifndef TAO_REMOTE_OBJECTS_PRIVATE_H_
#define TAO_REMOTE_OBJECTS_PRIVATE_H_ 1

#include <tao-shared-objects-private.h>
#include <tao-remote-objects.h>

TAO_BEGIN_DECLS

/**
 * @ingroup RemoteObjects
 *
 * @{
 */

/**
 * Report command timeout when server killed?
 *
 * This macros specifies whether a timeout result is returned
 * when a command failed because the server has been killed.
 */
#define TAO_ASSUME_TIMOUT_IF_SERVER_KILLED 1

/**
 * Remote object struture.
 *
 * This structure is stored in shared memory and represent the base of
 * a TAO shared object used to communicate with a remote server.
 *
 * The number of queued commands is incremented by the client when queuing a
 * new command (see tao_remote_object_lock_for_command()).  When the pending
 * command has been processed by the server, it sets the number of processed
 * commands to the number of queued commands (this may not just amount to
 * incrementing the number of processed commands by one because there may be
 * commands overriding others with lower priority) and set the prending command
 * to @ref TAO_COMMAND_NONE to indicate that a new command has been processed.
 */
struct tao_remote_object {
    tao_shared_object           base;///< Base structure.
    const long                 nbufs;///< Number of output buffers.
    const long                offset;///< Offset to output buffer (in bytes).
    const long                stride;///< Stride between successive output
                                     ///  buffers (in bytes).
    tao_atomic tao_serial     serial;///< Serial number of last output buffer.
    tao_atomic tao_state       state;///< Current state.
    tao_command              command;///< Pending command.
    tao_atomic tao_serial      ncmds;///< Number of processed commands.
    const char owner[TAO_OWNER_SIZE];///< Server name.
};

/**
 * @typedef tao_dataframe_header
 *
 * @brief Data-frame descriptor as written in shared memory.
 *
 * This structure is the header of each data-frame as stored in shared memory
 * in a @ref tao_remote_mirror or @ref tao_remote_sensor.  It is followed
 * (perhaps after some padding bytes to preserve alignment) by the data values.
 *
 * The end-user shall not directly deal with instances of this structure but
 * shall use @ref tao_dataframe_info instead.
 */
typedef struct tao_dataframe_header {
    tao_atomic tao_serial serial; ///< Serial number.
    tao_serial              mark; ///< User-defined mark.
    tao_time                time; ///< Time-stamp.
} tao_dataframe_header;

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_REMOTE_OBJECTS_PRIVATE_H_
