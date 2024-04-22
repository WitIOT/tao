// tao-remote-cameras.h -
//
// Definitions for remote cameras and virtual frame-grabbers.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2018-2022, Éric Thiébaut.

#ifndef TAO_REMOTE_CAMERAS_H_
#define TAO_REMOTE_CAMERAS_H_ 1

#include <tao-remote-objects.h>
#include <tao-cameras.h>

//-----------------------------------------------------------------------------
// REMOTE CAMERAS

/**
 * @defgroup RemoteCameras Remote cameras
 *
 * @ingroup Cameras
 *
 * @brief Remote objects used to communicate with camera servers.
 *
 * A remote camera instance is a structure stored in shared memory which is
 * used to communicate with a camera server.
 *
 * @{
 */

/**
 * @brief Opaque structure to a remote camera.
 *
 * Remote cameras are remote objects (see @ref tao_remote_object)
 * used to communicate with camera servers in TAO.
 *
 * @see tao_remote_camera_.
 */
typedef struct tao_remote_camera tao_remote_camera;

/**
 * Create a new instance of a remote camera.
 *
 * This function creates the resources in shared memory to manage a remote
 * camera.  This function shall be called by the server in charge of a camera
 * device.  Clients shall call tao_remote_camera_attach() to connect to the
 * remote camera.  The clients and the server are responsible of eventually
 * calling tao_remote_camera_detach() to release the resources.
 *
 * @param owner   The name of the server.
 *
 * @param nbufs   The number of cyclic data-frame buffers.
 *
 * @param flags   Permissions for clients and options.
 *
 * @return The address of the new remote camera instance or `NULL` in case of
 *         errors.
 */
extern tao_remote_camera* tao_remote_camera_create(
    const char* owner,
    long        nbufs,
    unsigned    flags);

/**
 * @brief Attach an existing remote camera to the address space of the caller.
 *
 * This function attaches an existing remote camera to the address space of the
 * caller.  As a result, the number of attachments on the returned camera is
 * incremented by one.  When the camera is no longer used by the caller, the
 * caller is responsible of calling tao_remote_camera_detach() to detach the
 * camera from its address space, decrement its number of attachments by one
 * and eventually free the shared memory associated with the camera.
 *
 * In principle, the same process may attach a remote camera more than once but
 * each attachment, due to tao_remote_camera_attach() or to
 * tao_remote_camera_create(), should be matched by a
 * tao_remote_camera_detach() with the corresponding address in the caller's
 * address space.
 *
 * @param shmid  Shared memory identifier.
 *
 * @return The address of the remote camera in the address space of the caller;
 *         `NULL` in case of failure.  Even tough the arguments are correct, an
 *         error may arise if the camera has been destroyed before attachment
 *         completes.
 *
 * @see tao_remote_camera_detach().
 */
extern tao_remote_camera* tao_remote_camera_attach(
    tao_shmid shmid);

/**
 * @brief Detach a remote camera from the address space of the caller.
 *
 * This function detaches a remote camera from the address space of the caller
 * and decrements the number of attachments of the remote camera.  If the
 * number of attachements reaches zero, the shared memory segment backing the
 * storage of the camera is destroyed (unless bit @ref TAO_PERSISTENT was set
 * at camera creation).
 *
 * @warning Detaching a remote camera does not detach shared arrays backing the
 * storage of the images acquired by this camera.  They have to be explicitly
 * detached.
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_camera_attach().
 */
extern tao_status tao_remote_camera_detach(
    tao_remote_camera* cam);

/**
 * @brief Get the size of a remote camera.
 *
 * This function yields the number of bytes of shared memory occupied by the
 * remote camera.  The size is constant for the life of the camera, it is thus
 * not necessary to have locked the camera to retrieve its identifier.
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @return The number of bytes of the shared memory segment backing the storage
 *         of the remote camera, `0` if @a cam is `NULL`.  Whatever the result,
 *         this getter function leaves the caller's last error unchanged.
 */
extern size_t tao_remote_camera_get_size(
    const tao_remote_camera* cam);

/**
 * @brief Get the type identifier of a remote camera.
 *
 * This function yields the identifier of the type of the remote camera.  The
 * type identifier is constant for the life of the camera, it is thus not
 * necessary to have locked the camera to retrieve its identifier.
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @return The type identifier of the remote camera, `0` if @a cam is `NULL`.
 *         Whatever the result, this getter function leaves the caller's last
 *         error unchanged.
 */
extern uint32_t tao_remote_camera_get_type(
    const tao_remote_camera* cam);

/**
 * @brief Get the shared memory identifier of a remote camera.
 *
 * This function yields the shared memory identifier of the remote camera.
 * This value can be used by another process to attach to its address space the
 * remote camera.  The shared memory identifier is constant for the life of the
 * camera, it is thus not necessary to have locked the camera to retrieve its
 * identifier.
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @return The shared memory identifier of the remote camera data,
 *         `TAO_BAD_SHMID` if @a cam is `NULL`.  Whatever the result, this
 *         getter function leaves the caller's last error unchanged.
 *
 * @see tao_remote_camera_attach.
 */
extern tao_shmid tao_remote_camera_get_shmid(
    const tao_remote_camera* cam);

/**
 * Lock a remote camera for exclusive access.
 *
 * This function locks a remote camera for exclusive (read and write) access.
 * The camera must be attached to the address space of the caller.  In case of
 * success, the caller is responsible for calling tao_remote_camera_unlock()
 * to eventually release the lock.
 *
 * @warning The same thread/process must not attempt to lock the same camera
 * more than once and should unlock it as soon as possible.
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_remote_camera_lock(
    tao_remote_camera* cam);

/**
 * Unlock a remote camera.
 *
 * This function unlocks a remote camera that has been successfully locked by
 * the caller.
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_remote_camera_unlock(
    tao_remote_camera* cam);

/**
 * Attempt to immediately lock a remote camera for exclusive access.
 *
 * This function attempts to lock a remote camera for exclusive (read and
 * write) access without blocking.  The caller is responsible for eventually
 * releasing the lock with tao_remote_camera_unlock().
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success, @ref TAO_TIMEOUT if the lock cannot be
 *         immediately acquired, or @ref TAO_ERROR on failure.
 */
extern tao_status tao_remote_camera_try_lock(
    tao_remote_camera* cam);

/**
 * Attempt to lock a remote camera for exclusive access with an absolute time
 * limit.
 *
 * This function attempts to lock a remote camera for exclusive (read and
 * write) access without blocking beyond a given time limit.  The caller is
 * responsible for eventually releasing the lock with
 * tao_remote_camera_unlock().
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @param lim    Absolute time limit.
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_camera_abstimed_lock(
    tao_remote_camera* cam,
    const tao_time* lim);

/**
 * Attempt to lock a remote camera for exclusive access with a relative time
 * limit.
 *
 * This function attempts to lock a remote camera for exclusive (read and
 * write) access without blocking more than a given duration.  The caller is
 * responsible for eventually releasing the lock with
 * tao_remote_camera_unlock().
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @param secs   Maximum time to wait (in seconds).  If this amount of time is
 *               very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling tao_remote_camera_lock().  If
 *               this amount of time is very short, the effect is the same as
 *               calling tao_remote_camera_try_lock().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_camera_timed_lock(
    tao_remote_camera* cam,
    double secs);

/**
 * Signal a condition variable to at most one thread waiting on a remote camera.
 *
 * This function restarts one of the threads that are waiting on the condition
 * variable of the camera.  Nothing happens, if no threads are waiting on the
 * condition variable.
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_camera_broadcast_condition(),
 *      tao_remote_camera_wait_condition().
 */
extern tao_status tao_remote_camera_signal_condition(
    tao_remote_camera* cam);

/**
 * Signal a condition to all threads waiting on a remote camera.
 *
 * This function behaves like tao_remote_camera_signal_condition() except that
 * all threads waiting on the condition variable of the camera are restarted.
 * Nothing happens, if no threads are waiting on the condition variable.
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_camera_signal_condition(),
 *      tao_remote_camera_wait_condition().
 */
extern tao_status tao_remote_camera_broadcast_condition(
    tao_remote_camera* cam);

/**
 * Wait for a condition to be signaled for a remote camera.
 *
 * This function atomically unlocks the exclusive lock associated with the
 * remote camera and waits for its associated condition variable to be
 * signaled.  The thread execution is suspended and does not consume any CPU
 * time until the condition variable is signaled.  The mutex of the camera must
 * have been locked (e.g., with tao_remote_camera_lock()) by the calling thread
 * on entrance to this function.  Before returning to the calling thread, this
 * function re-acquires the mutex.
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_camera_lock(),
 *      tao_remote_camera_signal_condition().
 */
extern tao_status tao_remote_camera_wait_condition(
    tao_remote_camera* cam);

/**
 * Wait for a condition to be signaled for a remote camera without blocking
 * longer than an absolute time limit.
 *
 * This function behaves like tao_remote_camera_wait_condition() but blocks no
 * longer than a given duration.
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @param lim    Absolute time limit with the same conventions as
 *               tao_get_current_time().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_camera_abstimed_wait_condition(
    tao_remote_camera* cam,
    const tao_time* lim);

/**
 * Wait for a condition to be signaled for a remote camera without blocking
 * longer than a relative time limit.
 *
 * This function behaves like tao_remote_camera_wait_condition() but blocks no
 * longer than a given duration.
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller.
 *
 * @param secs   Maximum amount of time (in seconds).  If this amount of time
 *               is very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling
 *               tao_remote_camera_wait_condition().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_camera_timed_wait_condition(
    tao_remote_camera* cam,
    double secs);

/**
 * Get the name of the owner of a remote camera.
 *
 * This function yields the name of the owner of the remote camera.  This
 * information is immutable and the camera needs not be locked by the caller.
 *
 * @param cam     Pointer to a remote camera attached to the address space of
 *                the caller.
 *
 * @return The name of the remote camera owner or an empty string `""` for a
 *         `NULL` camera pointer.  Whatever the result, this getter function
 *         leaves the caller's last error unchanged.
 */
extern const char* tao_remote_camera_get_owner(
    const tao_remote_camera* cam);

/**
 * Get the number of output images of a remote camera.
 *
 * This function yields the length of the cyclic list of shared arrays
 * memorized by the owner of a remote camera.  This information is immutable
 * and the camera needs not be locked by the caller.
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller and locked by the caller.
 *
 * @return The length of the list of shared arrays memorized by the owner of
 *         the remote camera, `0` if @a cam is `NULL`.  Whatever the result,
 *         this getter function leaves the caller's last error unchanged.
 *
 * @see tao_remote_camera_lock.
 */
extern long tao_remote_camera_get_nbufs(
    const tao_remote_camera* cam);

/**
 * Get the serial number of the last available output image of a remote camera.
 *
 * This function yields the serial number of the last image available from a
 * remote camera.  This is also the number of images posted so far by the
 * server owning the remote camera.
 *
 * The serial number of last image may change (i.e., when acquisition is
 * running), but serial number is stored in an *atomic* variable, so the caller
 * needs not lock the remote camera.
 *
 * @param cam    Pointer to a remote camera attached to the address space of
 *               the caller and locked by the caller.
 *
 * @return A nonnegative integer.  A strictly positive value which is the
 *         serial number of the last available image if any, `0` if image
 *         acquisition has not yet started of if `cam` is `NULL`.  Whatever the
 *         result, this getter function leaves the caller's last error
 *         unchanged.
 *
 * @see tao_remote_camera_lock.
 */
extern tao_serial tao_remote_camera_get_serial(
    const tao_remote_camera* cam);

/**
 * Get the number of commands processed by the server owning a remote camera.
 *
 * This function yields the number of commands processed so far by the owner of
 * a remote camera.
 *
 * The number of processed commands is stored in an *atomic* variable, so the
 * caller needs not lock the remote camera.
 *
 * @param cam     Pointer to a remote camera attached to the address space of
 *                the caller.
 *
 * @return The number processed commands, a nonnegative integer which may be
 *         `0` if no commands have been ever processed or if `cam` is `NULL`.
 *         Whatever the result, this getter function leaves the caller's last
 *         error unchanged.
 */
extern tao_serial tao_remote_camera_get_ncmds(
    const tao_remote_camera* cam);

/**
 * Get the current state of the server owning a remote camera.
 *
 * This function yields the current state of the server owning the remote
 * camera.
 *
 * The server state is stored in an *atomic* variable, so the caller needs not
 * lock the remote camera.
 *
 * @param cam     Pointer to a remote camera attached to the address space of
 *                the caller.
 *
 * @return The state of the remote server, @ref TAO_STATE_UNREACHABLE if `cam`
 *         is `NULL`.  Whatever the result, this getter function leaves the
 *         caller's last error unchanged.
 */
extern tao_state tao_remote_camera_get_state(
    const tao_remote_camera* cam);

/**
 * Check whether the server owning a remote camera is alive.
 *
 * This function uses the current state of the server owning the remote camera
 * to determine whether the server is alive.
 *
 * The server state is stored in an *atomic* variable, so the caller needs not
 * lock the remote camera.
 *
 * @param cam     Pointer to a remote camera attached to the address space of
 *                the caller.
 *
 * @return A boolean result; `false` if `cam` is `NULL`.  Whatever the result,
 *         this getter function leaves the caller's last error unchanged.
 */
extern int tao_remote_camera_is_alive(
    const tao_remote_camera* cam);

/**
 * Get the pixel type for the captured images after pre-processing.
 *
 * The caller shall have locked the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The pixel type for the captured images after pre-processing, `-1`
 *         if @a cam is `NULL`.
 */
extern tao_eltype tao_remote_camera_get_pixeltype(
    const tao_remote_camera* cam);

/**
 * Get the encoding of pixels in images sent by the camera.
 *
 * The caller shall have locked the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The encoding of pixels in the raw captured images,
 *         `TAO_ENCODING_UNKNOWN` if @a cam is `NULL`.
 */
extern tao_encoding tao_remote_camera_get_sensorencoding(
    const tao_remote_camera* cam);

/**
 * Get the encoding of pixels in acquisition buffers.
 *
 * The caller shall have locked the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The encoding of pixels in acquisition buffers,
 *         `TAO_ENCODING_UNKNOWN` if @a cam is `NULL`.
 */
extern tao_encoding tao_remote_camera_get_bufferencoding(
    const tao_remote_camera* cam);

/**
 * Get the width of the detector.
 *
 * This function yields the number of pixels per line of the detector which is
 * the maximum width for captured iamges.
 *
 * The returned quantity should be immutable so the caller do not have to lock
 * the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The number of pixels per line of the detector, `0` if @a cam is
 *         `NULL`.
 */
extern long tao_remote_camera_get_sensorwidth(
    const tao_remote_camera* cam);

/**
 * Get the height of the detector.
 *
 * This function yields the number of lines of pixels of the detector which is
 * the maximum height for captured iamges.
 *
 * The returned quantity should be immutable so the caller do not have to lock
 * the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The number of lines of pixels of the detector, `0` if @a cam is
 *         `NULL`.
 */
extern long tao_remote_camera_get_sensorheight(
    const tao_remote_camera* cam);

/**
 * Get the horizontal binning factor.
 *
 * The caller shall have locked the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The horizontal binning factor in physical pixels, `0` if
 *         @a cam is `NULL`.
 */
extern long tao_remote_camera_get_xbin(
    const tao_remote_camera* cam);

/**
 * Get the vertical binning factor.
 *
 * The caller shall have locked the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The vertical binning factor in physical pixels, `0` if
 *         @a cam is `NULL`.
 */
extern long tao_remote_camera_get_ybin(
    const tao_remote_camera* cam);

/**
 * Get the horizontal offset of captured images.
 *
 * The caller shall have locked the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The horizontal offset in physical pixels of the region of interest
 *         set for the captured images, `0` if @a cam is `NULL`.
 */
extern long tao_remote_camera_get_xoff(
    const tao_remote_camera* cam);

/**
 * Get the vertical offset of captured images.
 *
 * The caller shall have locked the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The vertical offset in physical pixels of the region of interest
 *         set for the captured images, `0` if @a cam is `NULL`.
 */
extern long tao_remote_camera_get_yoff(
    const tao_remote_camera* cam);

/**
 * Get the width of the captured images.
 *
 * This function yields the number of macro-pixels per line of the captured
 * images.  If no sub-sampling nor re-binning of physical pixels is used a
 * macro-pixel corresponds to a physical pixel.
 *
 * The caller shall have locked the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The number of macro-pixels per line of the captured images, `0` if
 *         @a cam is `NULL`.
 */
extern long tao_remote_camera_get_width(
    const tao_remote_camera* cam);

/**
 * Get the height of the captured images.
 *
 * This function yields the number of lines of macro-pixels in the captured
 * images.  If no sub-sampling nor re-binning of physical pixels is used a
 * macro-pixel corresponds to a physical pixel.
 *
 * The caller shall have locked the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The number of lines of macro-pixels in the captured images, `0` if
 *         @a cam is `NULL`.
 */
extern long tao_remote_camera_get_height(
    const tao_remote_camera* cam);

/**
 * Get the frame rate.
 *
 * The caller shall have locked the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The number of frame per seconds, `0` if @a cam is `NULL`.
 */
extern double tao_remote_camera_get_framerate(
    const tao_remote_camera* cam);

/**
 * Get the duration of the exposure.
 *
 * The caller shall have locked the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The exposure time in seconds for the captured images, `0` if @a cam
 *         is `NULL`.
 */
extern double tao_remote_camera_get_exposuretime(
    const tao_remote_camera* cam);

/**
 * Get the current pre-processing level.
 *
 * The caller shall have locked the remote camera.
 *
 * @param cam   Address of remote camera in address space of caller.
 *
 * @return The pre-processing level, @ref TAO_PREPROCESSING_NONE if @a cam is
 *         `NULL`.  Whatever the result, this function leaves the caller's last
 *         error unchanged.
 */
extern tao_preprocessing tao_remote_camera_get_preprocessing(
    const tao_remote_camera* cam);

/**
 * Retrieve the shared memory identifier of one of the array storing the
 * pre-processing parameters.
 *
 * There are at most 4 arrays (@b a, @b b, @b q, and @b r) involved in the
 * pre-processing of images:
 *
 * - If the camera pre-processing level is @ref TAO_PREPROCESSING_NONE,
 *   the raw pixels are simply converted:
 *
 *   ```{.c}
 *   img[x,y] = raw[x,y];
 *   ```
 *
 * - If the camera pre-processing level is @ref TAO_PREPROCESSING_AFFINE or
 *   @ref TAO_PREPROCESSING_FULL, an affine correction is applied to the raw
 *   pixels:
 *
 *   ```{.c}
 *   img[x,y] = (raw[x,y] - b[x,y])*a[x,y];
 *   ```
 *
 * - In addition, if the camera pre-processing level is @ref
 *   TAO_PREPROCESSING_FULL, the precision of the pixels is computed by:
 *
 *   ```{.c}
 *   wgt[x,y] = q[x,y]/(max(img[x,y], 0) + r[x,y]);
 *   ```
 *
 * The caller shall have locked the remote camera.
 *
 * @param cam   Address of remote camera.
 *
 * @param idx   Index of the array to retrieve: `0` for the pixel correction
 *              factor @b a, `1` for the pixel correction bias @b b, `2` for
 *              the pixel precision numerator @b q, and `3` for the pixel
 *              precision denominator offset @b r.
 *
 * @return The shared memory identifier of the requested array, @ref
 *         TAO_BAD_SHMID if this array does not exist (whether this is because
 *         `cam` is `NULL`, the index `idx` is out of range, or the array is
 *         not needed at the current level of pre-processing).  Whatever the
 *         result, this function leaves the caller's last error unchanged.
 */
extern tao_shmid tao_remote_camera_get_preprocessing_shmid(
    const tao_remote_camera* cam,
    int idx);

/**
 * Retrieve the configuration of a remote camera.
 *
 * The caller shall have locked the remote camera.
 *
 * @param src   Address of source instance.
 *
 * @param dst   Address of destination structure.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR on failure.
 */
extern tao_status tao_remote_camera_get_configuration(
    const tao_remote_camera* src,
    tao_camera_config* dst);

/**
 * Configure remote camera settings.
 *
 * A client can call this function to configure the settings of a remote
 * camera.
 *
 * The command is executed asynchronously.  The returned value is the serial
 * number of the command so that the caller can call
 * tao_remote_camera_wait_command() to make sure that the command has been
 * completed.
 *
 * The caller must not have locked the remote camera.
 *
 * @param cam   Remote camera instance.
 *
 * @param cfg   New configuration settings.
 *
 * @param secs  Maximum number of seconds to wait.
 *
 * @return The serial number of the "*configure*" command, 0 if the command
 *         cannot be sent before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_camera_configure(
    tao_remote_camera* cam,
    const tao_camera_config* cfg,
    double secs);

/**
 * Start acquisition by a remote camera.
 *
 * A client can call this function to start image acquisition by a remote
 * camera.
 *
 * The command is executed asynchronously.  The returned value is the serial
 * number of the command so that the caller can call
 * tao_remote_camera_wait_command() to make sure that the command has been
 * completed.
 *
 * The caller must not have locked the remote camera.
 *
 * @param cam   Remote camera instance.
 *
 * @param secs  Maximum number of seconds to wait.
 *
 * @return The serial number of the "*start*" command, 0 if the command cannot
 *         be sent before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_camera_start(
    tao_remote_camera* cam,
    double secs);

/**
 * Stop acquisition by a remote camera.
 *
 * A client can call this function to stop image acquisition by a remote
 * camera.  The command is executed asynchronously.  The returned value is the
 * serial number of the command so that the caller can call
 * tao_remote_camera_wait_command() to make sure that the command has been
 * completed.
 *
 * The caller must not have locked the remote camera.
 *
 * @param cam   Remote camera instance.
 *
 * @param secs  Maximum number of seconds to wait.
 *
 * @return The serial number of the "*stop*" command, 0 if the command cannot
 *         be sent before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_camera_stop(
    tao_remote_camera* cam,
    double secs);

/**
 * Abort acquisition by a remote camera.
 *
 * A client can call this function to abort image acquisition by a remote
 * camera.  The command is executed asynchronously.
 *
 * The returned value is the serial number of the command so that the caller
 * can call tao_remote_camera_wait_command() to make sure that the command has
 * been completed.
 *
 * The caller must not have locked the remote camera.
 *
 * @param cam   Remote camera instance.
 *
 * @param secs  Maximum number of seconds to wait.
 *
 * @return The serial number of the "*abort*" command, 0 if the command cannot
 *         be sent before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_camera_abort(
    tao_remote_camera* cam,
    double secs);

/**
 * Reset a remote camera.
 *
 * A client can call this function to reset a remote camera.  This may be
 * useful when the camera is in a recoverable error state.  This stop
 * acquisition if it is running.
 *
 * The command is executed asynchronously.  The returned value is the serial
 * number of the command so that the caller can call
 * tao_remote_camera_wait_command() to make sure that the command has been
 * completed.
 *
 * The caller must not have locked the remote camera.
 *
 * @param cam   Remote camera instance.
 *
 * @param secs  Maximum number of seconds to wait.
 *
 * @return The serial number of the "*reset*" command, 0 if the command cannot
 *         be sent before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_camera_reset(
    tao_remote_camera* cam,
    double secs);

/**
 * Kill a remote camera server.
 *
 * A client can call this function to kill the server owning a remote camera.
 *
 * The command is executed asynchronously.  The returned value is the serial
 * number of the command so that the caller can call
 * tao_remote_camera_wait_command() to make sure that the command has been
 * completed.
 *
 * The caller must not have locked the remote camera.
 *
 * @param cam   Remote camera instance.
 *
 * @param secs  Maximum number of seconds to wait.
 *
 * @return The serial number of the "*kill*" command, 0 if the command cannot
 *         be sent before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_camera_kill(
    tao_remote_camera* cam,
    double secs);

/**
 * Wait the completion of a command sent to a remote camera.
 *
 * A client can call this function to wait for the completion of a command sent
 * to a remote camera.  A remote server execute commands asynchronously.  When
 * a command is sent, e.g. by tao_remote_camera_start(), the result, say `num`,
 * is one plus the actual value of the command counter.  When a command is
 * completed, the server increments the command counter by one.  The function
 * tao_remote_camera_wait_command() waits until the command counter becomes
 * greater of equal `num` but waiting no longer than `secs` seconds.
 *
 * @warning The caller must not have locked the remote camera.
 *
 * @param cam     Remote camera instance.
 *
 * @param num     Serial number of the command.
 *
 * @param secs    Maximum number of seconds to wait.
 *
 * @return @ref TAO_OK on success, @ref TAO_TIMEOUT if command completion does
 *         not occur before the time limit, and @ref TAO_ERROR on failure.
 */
extern tao_status tao_remote_camera_wait_command(
    tao_remote_camera* cam,
    tao_serial         num,
    double             secs);

/**
 * Wait for a given output image.
 *
 * This function behaves as @ref tao_remote_object_wait_output which to see.
 *
 * The caller must not have locked the remote camera.
 *
 * Typical usage to wait for the next image:
 *
 * ```{.c}
 * double maxsecs = 5.0; // max. number of seconds to wait
 * tao_remote_camera* cam = tao_remote_camera_attach(
 *     tao_config_read_shmid(camera_name));
 * tao_serial serial = tao_remote_camera_wait_output(cam, 0, maxsecs);
 * if (serial > 0) {
 *     tao_shmid shmid = tao_remote_camera_get_image_shmid(cam, serial);
 *     if (shmid != TAO_BAD_SHMID) {
 *         tao_shared_array *arr = tao_shared_array_attach(shmid);
 *         if (arr != NULL) {
 *             // Lock image to make sure its contents remains untouched.
 *             if (tao_shared_array_rdlock(arr) == TAO_OK) {
 *                 if (tao_shared_array_get_serial(arr) == serial) {
 *                     // Process image data and unlock it.
 *                     ....
 *                 }
 *                 tao_shared_array_unlock(arr);
 *             }
 *         }
 *     }
 * }
 * ```
 *
 * Other possibility to process, say 100, images with as few losses as
 * possible:
 *
 * ```{.c}
 * double maxsecs = 5.0; // max. number of seconds to wait
 * tao_remote_camera* cam = tao_remote_camera_attach(
 *     tao_config_read_shmid(camera_name));
 * tao_status status = TAO_OK;
 * tao_serial prev = 0; // serial number of previous image
 * long nimgs = 0; // number of processed images
 * while (status == TAO_OK && nimgs < 100) {
 *     // serial number of next image to wait
 *     tao_serial next = tao_remote_camera_get_serial(cam);
 *     if (next < prev + 1) {
 *         next = prev + 1;
 *     }
 *     tao_shmid shmid = tao_remote_camera_get_image_shmid(cam, next);
 *     if (shmid == TAO_BAD_SHMID) {
 *         status = TAO_ERROR;
 *         break;
 *     }
 *     tao_shared_array *arr = tao_shared_array_attach(shmid);
 *     if (arr == NULL) {
 *         status = TAO_ERROR;
 *         break;
 *     }
 *     // Lock image for reading (this will block until image has
 *     // been acquired).
 *     tao_status status = tao_shared_array_timed_rdlock(arr, maxsecs);
 *     if (status == TAO_OK) {
 *         // Check that image has not been overwritten.
 *         tao_serial serial = tao_shared_array_get_serial(arr);
 *         if (serial == next) {
 *             // Process image data.
 *             ....
 *             prev = serial;
 *             ++nimgs;
 *         }
 *         if (tao_shared_array_unlock(arr) != TAO_OK) {
 *             status = TAO_ERROR;
 *         }
 *     }
 *     if (tao_shared_array_detach(arr)
 *         status = TAO_ERROR;
 *     }
 * }
 * ```
 *
 * This version is more efficient because it attaches and locks the next image
 * **in advance** so that when the lock is acquired, the image can be
 * immediately processed (provided it has not been overwritten which is
 * asserted by its serial number).  Carefully look at the loop to realize that
 * lock-unlock and attach-detach are always done consistently.  Beware that
 * this *trick* only works for the **next image** (as computed in the example).
 * For previous images, you should be able to lock without waiting (however
 * remmeber that output images are in a cyclic list).
 *
 * @param cam     Pointer to a remote camera in caller's address space.
 *
 * @param serial  The serial number of the output image to wait for.  If less
 *                or equal zero, the next image is waited for.
 *
 * @param secs    Maximum amount of time to wait (in seconds).
 *
 * @return A strictly positive number which is the serial number of the
 *         requested image, `0` if the requested image is not available before
 *         the time limit (i.e. timeout), `-1` if the requested image is too
 *         old (it has been overwritten by some newer contents or it is beyond
 *         the last available image), `-2` if the server has been killed and
 *         the requested image is beyond the last available one, or `-3` in
 *         case of failure.  In the latter case, error details are reflected by
 *         the caller's last error.
 *
 * @see  tao_remote_object_wait_output, tao_remote_camera_get_image_shmid.
 */
extern tao_serial tao_remote_camera_wait_output(
    tao_remote_camera* cam,
    tao_serial         serial,
    double             secs);

/**
 * Get the shared memory identifier of a camera output image.
 *
 * This function yields the shared memory identifier of the image corresponding
 * to a given serial number in the cyclic list of output images of a remote
 * camera.  This shared memory identifier can be used to attach to the shared
 * array that stores the image data.
 *
 * @warning The actual serial number of the image may be different than @b serial
 *          because the requested image may have not yet been acquired or may
 *          have been overwritten by a newer one (output images are stored in a
 *          cyclic list).
 *
 * @param cam     Pointer to remote camera.
 *
 * @param serial  Serial number of buffer to retrieve.
 *
 * @return A shared memory identifier, @ref TAO_BAD_SHMID in case of error
 *         (i.e., the serial number is not strictly positive or no image is
 *         currently stored at the position corresponding to the serial number
 *         in the cyclic list of images).
 */
extern tao_shmid tao_remote_camera_get_image_shmid(
    tao_remote_camera* cam,
    tao_serial serial);

/**
 * @}
 */

#endif // TAO_REMOTE_CAMERAS_H_
