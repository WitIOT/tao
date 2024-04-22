// tao-remote-cameras-private.h -
//
// Private definitions for remote cameras.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2018-2022, Éric Thiébaut.

#ifndef TAO_REMOTE_CAMERAS_PRIVATE_H_
#define TAO_REMOTE_CAMERAS_PRIVATE_H_ 1

#include <tao-remote-cameras.h>
#include <tao-remote-objects-private.h>

TAO_BEGIN_DECLS

/**
 * @brief Remote camera information.
 *
 * @ingroup RemoteCameras
 *
 * This structure describes the shared data storing the resources of a remote
 * camera.  After querying the shared memory identifier to the server (the
 * frame grabber), clients can attach this shared data part with
 * tao_remote_camera_attach().  When a client no longer needs this shared data,
 * it shall call tao_remote_camera_detach().
 *
 * This structure **must** be considered as read-only by the clients and
 * information provided by this structure is only valid as long as the client
 * locks this shared structure by calling tao_remote_camera_lock() and until
 * the client unlock the structure by calling tao_remote_camera_unlock().
 * Beware to not call tao_remote_camera_detach() while the shared data is
 * locked by the caller.
 *
 * If @n serial > 0, the index in the list of shared arrays memorized by the
 * virtual frame-grabber owning the remote camera is given by:
 *
 * ~~~~~{.c}
 * index = (serial - 1) % nbufs
 * ~~~~~
 *
 */
struct tao_remote_camera {
    tao_remote_object        base;///< Shared object backing the storage of the
                                  ///  structure.
    tao_camera_config      config;///< Camera information.
    union {
        tao_camera_config config;///< Configuration for
                                 ///  @ref TAO_COMMAND_CONFIG.
    } arg;                       ///< Argument of command.
    tao_shmid         preproc[4];///< Shared memory identifiers of shared
                                 ///  arrays storing pre-processing parameters.
                                 ///  It is assumed that clients can only
                                 ///  change the contents of these arrays while
                                 ///  owning an exclusive access to the remote
                                 ///  camera.  Pre-processing parameters are
                                 ///  distinct arrays for optimal memory
                                 ///  alignment.
};


TAO_END_DECLS

#endif // TAO_REMOTE_CAMERAS_PRIVATE_H_
