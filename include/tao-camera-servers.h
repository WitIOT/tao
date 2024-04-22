// tao-camera-servers.h -
//
// Definitions for camera servers in TAO.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2022, Éric Thiébaut.

#ifndef TAO_CAMERA_SERVERS_H_
#define TAO_CAMERA_SERVERS_H_ 1

#include <tao-cameras.h>
#include <tao-remote-cameras.h>

//-----------------------------------------------------------------------------
// REMOTE CAMERAS

/**
 * @defgroup CameraServers Camera servers
 *
 * @ingroup Cameras
 *
 * @brief Structures and functions for camera servers.
 *
 * A camera server manage a camera device and deliver images in shared memory.
 * It communicates with clients via a remote camera instance.
 *
 * @{
 */

/**
 * Opaque definition of a camera server structure.
 */
typedef struct tao_camera_server tao_camera_server;

/**
 * Opaque definition of the structure storing image processing parameters.
 */
typedef struct tao_pixels_processor_context tao_pixels_processor_context;

/**
 * Prototype of callback function to perform image processing.
 */
typedef void tao_pixels_processor(
    const tao_pixels_processor_context* ctx);

/**
 * Structure storing image processing parameters.
 */
struct tao_pixels_processor_context {
    tao_preprocessing preprocessing;///> Pre-processing method.
    tao_encoding     bufferencoding;///> Encoding of acquisition buffer.
    tao_eltype               eltype;///> Output pixel type.
    long                      width;///> Image width.
    long                     height;///> Image height.
    long                     stride;///> Raw image stride.
    long                 stride_min;///> Minimum raw image stride.
    void*                       dat;///> Output image pixels.
    void*                       wgt;///> Output image weights.
    const void*                 raw;///> Raw pixels.
    const void*          preproc[4];///> Pre-processing parameters.
    tao_pixels_processor *processor;///> Callback.
};

/**
 * Camera server structure.
 *
 * The mutex and condition variable of a camera server are to control the
 * access to the resources of the camera server which are shared between two
 * threads: the "server", that is the calling thread of
 * tao_camera_server_run_loop(), and the "worker".  The "worker" is in charge
 * of the camera device.
 *
 * One important information are the run-levels of the camera device and of the
 * worker thread which should be as follows:
 *
 * | Worker | Camera | Worker state                              |
 * |:-------|:-------|:------------------------------------------|
 * | 0      | 1      | Worker has not yet started                |
 * | 1      | 1      | Worker is idle                            |
 * | 2      | 2      | Worker is acquiring and processing images |
 * | 3      | x -> 1 | Worker has exited (i.e., joinable)        |
 * | 4      | x -> 1 | Joining a terminated worker has failed    |
 */
struct tao_camera_server {
    tao_mutex              mutex;///> Exclusive lock.
    tao_cond                cond;///> Condition variable to notify changes.
    tao_remote_camera*    remote;///> Remote camera to communicate with clients.
    tao_camera*           device;///> Unified camera device.
    tao_camera_config     config;///> Copy of camera device configuration.
    FILE*                logfile;///> File to print log messages.
    tao_message_level   loglevel;///> Level of details for log messages.
    bool                   fancy;///> Use ANSI escape codes.
    tao_thread            worker;///> Identifier of the worker thread.
    int                 runlevel;///> Run-level of the worker thread.
    tao_state              state;///> Worker state.
    tao_command             task;///> Task to be executed by worker.
    union {
        tao_camera_config config;///> Configuration to use.
    }                        arg;///> Argument for the command to execute.
    int                     drop;///> Drop exceeding images?
    unsigned               flags;///> Permission flags for output images and
                                 ///  remote camera.
    double               timeout;///> Maximum time to wait for images.
    const long             nbufs;///> Number of output images.
    tao_serial            serial;///> Number of published images. FIXME:
    tao_shared_array*     locked;///> Shared array currently locked to be used
                                 ///  as the next output image, or `NULL`.
    tao_shared_array* preproc[4];///> Pre-processing parameters.
    tao_pixels_processor_context proc;///> All informations to process pixels.
    tao_shmid*            shmids;///> Cyclic list of shared memory identifiers.
    tao_shared_array*  images[1];///> Cyclic list of output images.  Must be
                                 ///  last.
};

/**
 * Create a new camera server.
 *
 * This function creates a new camera server to operate a given camera device.
 * The server can be run by calling tao_camera_server_run_loop() and is
 * evetually destroyed by calling tao_camera_server_destroy().
 *
 * @warning The camera device is considered as borrowed by the server.  It is
 *          the caller's responsibility to eventually destroy the camera device
 *          after the server has been destoyed.
 *
 * Arguments @a owner, @a nbufs, and @a flags are used to create the remote
 * camera associated with the server (see @ref tao_remote_camera_create).  This
 * remote camera is automatically detached by tao_camera_server_destroy().
 *
 * @param owner   Short string identifying the server.  Maximal size including
 *                the final null character is @ref TAO_OWNER_SIZE.
 *
 * @param device  The camera device to operate on.
 *
 * @param nbufs   Number of output buffers.
 *
 * @param flags   Permissions granted to the group and to the others.  At
 *                least, read and write access (that is bits `S_IRUSR` and
 *                `S_IWUSR`) are granted for the caller.  Unless bit @ref
 *                TAO_PERSISTENT is set in `flags`, the shared memory backing
 *                the storage of the shared data will be destroyed upon last
 *                detach.
 *
 * @return The address of a new camera server, `NULL` in case of failure.
 */
extern tao_camera_server* tao_camera_server_create(
    const char* owner,
    tao_camera* device,
    long        nbufs,
    unsigned    flags);

/**
 * @brief Destroy a camera server.
 *
 * This function can be called to destroy a camera server and release all
 * resources associated with the server (notably the remote camera and the
 * worker thread).  The camera device is not closed/destroyed.  This function
 * may called to perform cleanup in case of foirced exit (fatal error or signal
 * received).
 *
 * The server structure, the camera device, and the remote camera must not be
 * locked by the caller.
 *
 * @param srv   Pointer to a camera server.
 */
extern tao_status tao_camera_server_destroy(
    tao_camera_server* srv);

/**
 * @brief Run the main loop of a camera server.
 *
 * This high-level function runs the main loop of a camera server.  None of the
 * resources controlled by the server (the server itself, the camera device,
 * and the remote camera) shall be locked by the caller.
 *
 * @param srv   Pointer to a camera server.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of errors.
 */
extern tao_status tao_camera_server_run_loop(
    tao_camera_server* srv);

/**
 * @brief Get the owner of a camera server.
 *
 * This information is immutable so there are no needs to lock the server
 * structure.
 *
 * @param srv   Pointer to a camera server.
 *
 * @return The name of the owner of the camera server, an empty string if
 *         argument is `NULL` or has not yet an associated remote camera.
 *         Whatever the result, this getter function leaves the caller's last
 *         error unchanged.
 *
 * @see tao_remote_camera_get_owner.
 */
extern const char* tao_camera_server_get_owner(
    const tao_camera_server* srv);

/**
 * @}
 */

#endif // TAO_CAMERA_SERVERS_H_
