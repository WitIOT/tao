// tao-cameras-private.h -
//
// Private definitions for cameras in TAO libraries.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_CAMERAS_PRIVATE_H_
#define TAO_CAMERAS_PRIVATE_H_ 1

#include <tao-threads.h>
#include <tao-cameras.h>

TAO_BEGIN_DECLS

/**
 * @ingroup UnifiedCameras
 *
 * @{
 */

/**
 * Table of virtual methods for a camera.
 *
 * An instance of this structure is specific to each combination of camera
 * model and frame-grabber.
 *
 * Camera @b run-level has only 5 levels: @b 0 when initialization is not
 * complete, @b 1 when initialized but acquisition not started, @b 2 when
 * acquiring, @b 3 if an error occurred that requires an explicit reset to
 * return to run-level 1 or @b 4 if camera is no longer usable and can only be
 * finalized (e.g. upon unrecoverable error).
 *
 * The following transitions are possible:
 *
 *    0 -> 1 is done when successfully calling the "initialize" virtual method
 *           which is only called once during the lifetime of the camera
 *           instance; so `runlevel != 0` means camera has been initialized;
 *
 *    1 -> 2 is done when successfully calling the "start" virtual method;
 *
 *    2 -> 1 is done when successfully calling the "stop" virtual method;
 *
 *    3 -> 1 is done when successfully calling the "reset" virtual method;
 *
 *    x -> 3 (with x neither 0, nor 4) may result from a recoverable error that
 *           requires an explicit reset;
 *
 *    x -> 4 (with x not 0) may result from an unrecoverable error.
 *
 * Except in case of errors (and when explicitly indicated), the virtual
 * methods are not allowed to change the camera run-level.  Changing the
 * run-level is done by the higher level TAO functions which call virtual
 * methods as needed and which also ensure that the camera is in a proper
 * run-level before calling a specific virtual method.  These rules are to
 * simplify the writting of virtual methods and to implement a common and
 * consistent behavior.
 *
 * Virtual methods must be all implemented (provided a `NULL` address is not
 * allowed).  If some method makes no sense for a specific kind of camera, the
 * implemented method shall returns an error.
 *
 * Initialization failure results in run-level being stuck at 0, subsequent
 * operations on the camera yield @ref TAO_NOT_READY error code.
 *
 */
struct tao_camera_ops {
    const char* name; ///< Camera model/family name.

    tao_status (*initialize)(
        tao_camera* cam,
        void* ctx);
    ///< Initialize the members of this structure (including the
    ///  configuration).  This method is only called once during the lifetime
    ///  of the camera instance.  It shall return @ref TAO_OK on success or
    ///  @ref TAO_ERROR on failure.  It is assumed that any allocated specific
    ///  resources are destroyed in case of failure (the `finalize` method is
    ///  not called if initialization fails).  Argument `ctx` can be used to
    ///  receive specific contextual data, it is the value of the `ctx`
    ///  argument of `tao_camera_create`.

    tao_status (*finalize)(
        tao_camera* cam);
    ///< Free device resources.  This method is only called once at the end of
    ///  the lifetime of the camera instance.  This method is not called by the
    ///  generic constructor tao_camera_create() in case of errors during the
    ///  construction.

    tao_status (*reset)(
        tao_camera* cam);
    ///< Reset camera to run-level 1 (sleeping) in case of recoverable error.
    ///  This method shall only be called when the camera run-level is 3.  On
    ///  success, the method shall set the run-level to 1 and return @ref
    ///  TAO_OK; otherwise it shall return @ref TAO_ERROR to indicate a
    ///  failure.

    tao_status (*update_config)(
        tao_camera* cam);
    ///< Retrieve camera current device settings, never called while
    ///  acquisition is running.  It shall return @ref TAO_OK on success or
    ///  @ref TAO_ERROR on failure.

    tao_status (*check_config)(
        tao_camera* cam,
        const tao_camera_config* cfg);
    ///< Check camera settings.  This virtual method is called by
    ///  tao_camera_check_configuration() and by
    ///  tao_camera_set_configuration().  The pixel conversion and
    ///  pre-processing parameters (`pixeltype`, `preprocessing`, and
    ///  `bufferencoding`) are checked before calling this virtual method.
    ///  Caller's last error may be used to report which parameters are
    ///  invalid.  This virtual method shall return @ref TAO_OK on success or
    ///  @ref TAO_ERROR on failure.

    tao_status (*set_config)(
        tao_camera* cam,
        const tao_camera_config* cfg);
    ///< Set camera settings.  This virtual method is called to set the camera
    ///  configuration by tao_camera_set_configuration() after having checked
    ///  the validity of the configuration in `cfg`.  This virtual method is
    ///  never called while acquiring.  This virtual method shall return @ref
    ///  TAO_OK on success or @ref TAO_ERROR on failure.

    tao_status (*start)(
        tao_camera* cam);
    ///< Start acquisition.  This method is only called after initialization
    ///  and if the camera is not acquiring.  It shall return @ref TAO_OK on
    ///  success or @ref TAO_ERROR on failure.

    tao_status (*stop)(
        tao_camera* cam);
    ///< Stop acquisition.  This method shall stop acquisition immediately,
    ///  without waiting for the current frame.  This method is only called
    ///  when the camera is acquiring.  It shall return @ref TAO_OK on success
    ///  or @ref TAO_ERROR on failure.

    tao_status (*wait_buffer)(
        tao_camera* cam,
        tao_acquisition_buffer* buf,
        double secs,
        int drop);
    ///< Wait for the next frame.  This method is only called when the camera
    ///  is acquiring.  It shall not wait more than `secs` seconds.  It shall
    ///  return @ref TAO_OK on success, @ref TAO_TIMEOUT on timeout or @ref
    ///  TAO_ERROR on failure.  This method may assume that arguments have been
    ///  checked for correctness.  Whatever the result, this method shall
    ///  increment `cam->info.frames` (because it is not possible for the
    ///  caller to know whether the error prevented acquiring a new buffer or
    ///  not).
    ///
    ///  Input pointer `buf` shall be assumed as always valid and zero-filled.
    ///
    ///  Argument `drop` is to deal with pending acquisition buffers if any.
    ///  See tao_camera_wait_acquisition_buffer().
};

/**
 * Generic camera.
 *
 * A camera has its own mutex and condition variable because it is often
 * necessary to operate the camera in separate threads.  However these threads
 * belongs to the same process so the mutex and condition variable associated
 * to a camera are not sharable between processes.
 *
 * The number of acquisition buffers is set to `info.config.buffers` at the
 * latest when acquisition is started.  The minimum number of acquisition
 * buffers should be 2.
 *
 * @note The `runlevel` member may be related to the enumeration @ref tao_state
 *       but has fewer possible values because @ref tao_state can also
 *       represent transient states, see @ref tao_camera_ops for details.
 */
struct tao_camera {
    tao_mutex            mutex;///< Lock to protect this structure.
    tao_cond              cond;///< Condition variable to signal events.
    const tao_camera_ops*  ops;///< Table of virtual methods for the camera.
    tao_camera_config   config;///< Current configuration.
    int               runlevel;///< Run-level of the camera.
};

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_CAMERAS_PRIVATE_H_
