// tao-remote-sensors.h -
//
// Definitions for remote wavefront sensors in TAO.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2022, Éric Thiébaut.

#ifndef TAO_REMOTE_SENSORS_H_
#define TAO_REMOTE_SENSORS_H_ 1

#include <tao-basics.h>
#include <tao-shackhartmann.h>
#include <tao-remote-objects.h>

TAO_BEGIN_DECLS

/**
 * @defgroup RemoteSensors  Remote wavefront sensors
 *
 * @ingroup WavefrontSensors
 *
 * @brief Client/server interface for wavefront sensors.
 *
 * A remote wavefront sensor instance is a structure stored in shared memory
 * which is used to communicate with a wavefront sensor server.  The shared
 * structure contains the current settings of the wavefront sensor and an
 * history of the measures made by the wavefront sensor.
 *
 * @{
 */

/**
 * Remote wavefront sensor.
 *
 * A remote wavefront sensor is created by a server by calling
 * tao_remote_sensor_create(), clients call tao_remote_sensor_attach() to
 * connect to the remote wavefront sensor.  The server and the clients call
 * tao_remote_sensor_detach() when they no longer need access to the remote
 * sensor.  A remote wavefront sensor is a remote shared object with the
 * following additional components:
 *
 * - A 2-dimensional array of indices describing the layout of the sub-images
 *   which can be retrieved by calling tao_remote_sensor_get_layout().
 *
 * - An internal configuration storing, among others, the bounding boxes and
 *   the reference position of the sub-images.  These settings can be retrieved
 *   by calling tao_remote_sensor_get_configuration() and which is set by
 *   tao_remote_sensor_set_configuration().
 *
 * - An internal buffer storing the requested commands which can be retrieved
 *   by calling tao_remote_sensor_get_requested_commands() and which is set by
 *   tao_remote_sensor_send_commands().
 *
 * - An internal buffer storing the actual commands which can be retrieved by
 *   calling tao_remote_sensor_get_actual_commands() and which is set by
 *   tao_remote_sensor_send_commands() after the commands have been applied to
 *   the device.
 *
 * - A cyclic list of output buffers storing an history of the commands
 *   actually applied to the sensor.  Waiting for a given output buffer is done
 *   by calling tao_remote_sensor_wait_output() and retrieving the contents of
 *   an output buffer is done by calling tao_remote_sensor_fetch_data().
 *
 * Note that commands are relative to the reference values.
 */
typedef struct tao_remote_sensor tao_remote_sensor;

/**
 * Create a new instance of a remote wavefront sensor.
 *
 * This function creates the resources in shared memory to manage a remote
 * wavefront sensor and its telemetry.  This function shall be called by the
 * server in charge of a wavefront sensor device.  Clients shall call
 * tao_remote_sensor_attach() to connect to the remote wavefront sensor.  The
 * clients and the server are responsible of eventually calling
 * tao_remote_sensor_detach() to release the resources.
 *
 * @param owner      The name of the server.
 *
 * @param nbufs      The number of cyclic data-frame buffers.
 *
 * @param max_ninds  The maximum number of nodes in of the 2-dimensional layout
 *                   grid.
 *
 * @param max_nsubs  The maximum number of sub-images.
 *
 * @param flags      Permissions for clients and options.
 *
 * @return The address of the new remote wavefront sensor instance or `NULL` in
 *         case of errors.
 */
extern tao_remote_sensor* tao_remote_sensor_create(
    const char* owner,
    long        nbufs,
    long    max_ninds,
    long    max_nsubs,
    unsigned    flags);

/**
 * @brief Attach an existing remote wavefront sensor to the address space of
 * the caller.
 *
 * This function attaches an existing remote wavefront sensor to the address
 * space of the caller.  As a result, the number of attachments on the returned
 * sensor is incremented by one.  When the sensor is no longer used by the
 * caller, the caller is responsible of calling tao_remote_sensor_detach() to
 * detach the sensor from its address space, decrement its number of
 * attachments by one and eventually free the shared memory associated with the
 * sensor.
 *
 * In principle, the same process may attach a remote wavefront sensor more
 * than once but each attachment, due to tao_remote_sensor_attach() or to
 * tao_remote_sensor_create(), should be matched by a
 * tao_remote_sensor_detach() with the corresponding address in the caller's
 * address space.
 *
 * @param shmid  Shared memory identifier.
 *
 * @return The address of the remote wavefront sensor in the address space of
 *         the caller; `NULL` in case of failure.  Even tough the arguments are
 *         correct, an error may arise if the sensor has been destroyed before
 *         attachment completes.
 *
 * @see tao_remote_sensor_detach().
 */
extern tao_remote_sensor* tao_remote_sensor_attach(
    tao_shmid shmid);

/**
 * @brief Detach a remote wavefront sensor from the address space of the
 * caller.
 *
 * This function detaches a remote wavefront sensor from the address space of
 * the caller and decrements the number of attachments of the remote wavefront
 * sensor.  If the number of attachments reaches zero, the shared memory
 * segment backing the storage of the sensor is destroyed (unless bit @ref
 * TAO_PERSISTENT was set at sensor creation).
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_sensor_attach().
 */
extern tao_status tao_remote_sensor_detach(
    tao_remote_sensor* wfs);

/**
 * @brief Get the size of a remote wavefront sensor.
 *
 * This function yields the number of bytes of shared memory occupied by the
 * remote wavefront sensor.  The size is constant for the life of the sensor,
 * it is thus not necessary to have locked the sensor to retrieve its
 * identifier.
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @return The number of bytes of the shared memory segment backing the storage
 *         of the remote wavefront sensor, `0` if @a wfs is `NULL`.  Whatever
 *         the result, this getter function leaves the caller's last error
 *         unchanged.
 */
extern size_t tao_remote_sensor_get_size(
    const tao_remote_sensor* wfs);

/**
 * @brief Get the type identifier of a remote wavefront sensor.
 *
 * This function yields the identifier of the type of the remote wavefront
 * sensor.  The type identifier is constant for the life of the sensor, it is
 * thus not necessary to have locked the sensor to retrieve its identifier.
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @return The type identifier of the remote wavefront sensor, `0` if @a wfs is
 *         `NULL`. Whatever the result, this getter function leaves the
 *         caller's last error unchanged.
 */
extern uint32_t tao_remote_sensor_get_type(
    const tao_remote_sensor* wfs);

/**
 * @brief Get the shared memory identifier of a remote wavefront sensor.
 *
 * This function yields the shared memory identifier of the remote wavefront
 * sensor. This value can be used by another process to attach to its address
 * space the remote wavefront sensor.  The shared memory identifier is constant
 * for the life of the sensor, it is thus not necessary to have locked the
 * sensor to retrieve its identifier.
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @return The identifier of the remote wavefront sensor data, `TAO_BAD_SHMID`
 *         if @a wfs is `NULL`.  Whatever the result, this getter function
 *         leaves the caller's last error unchanged.
 *
 * @see tao_remote_sensor_attach.
 */
extern tao_shmid tao_remote_sensor_get_shmid(
    const tao_remote_sensor* wfs);

/**
 * Lock a remote wavefront sensor for exclusive access.
 *
 * This function locks a remote wavefront sensor for exclusive (read and write)
 * access. The sensor must be attached to the address space of the caller.  In
 * case of success, the caller is responsible for calling
 * tao_unlock_shared_sensor() to eventually release the lock.
 *
 * @warning The same thread/process must not attempt to lock the same sensor
 * more than once and should unlock it as soon as possible.
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_remote_sensor_lock(
    tao_remote_sensor* wfs);

/**
 * Unlock a remote wavefront sensor.
 *
 * This function unlocks a remote wavefront sensor that has been successfully
 * locked by the caller.
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_remote_sensor_unlock(
    tao_remote_sensor* wfs);

/**
 * Attempt to immediately lock a remote wavefront sensor for exclusive access.
 *
 * This function attempts to lock a remote wavefront sensor for exclusive (read
 * and write) access without blocking.  The caller is responsible for
 * eventually releasing the lock with tao_remote_sensor_unlock().
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @return @ref TAO_OK on success, @ref TAO_TIMEOUT if the lock cannot be
 *         immediately acquired, or @ref TAO_ERROR on failure.
 */
extern tao_status tao_remote_sensor_try_lock(
    tao_remote_sensor* wfs);

/**
 * Attempt to lock a remote wavefront sensor for exclusive access with an
 * absolute time limit.
 *
 * This function attempts to lock a remote wavefront sensor for exclusive (read
 * and write) access without blocking beyond a given time limit.  The caller is
 * responsible for eventually releasing the lock with
 * tao_remote_sensor_unlock().
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @param lim    Absolute time limit.
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_sensor_abstimed_lock(
    tao_remote_sensor* wfs,
    const tao_time* lim);

/**
 * Attempt to lock a remote wavefront sensor for exclusive access with a
 * relative time limit.
 *
 * This function attempts to lock a remote wavefront sensor for exclusive (read
 * and write) access without blocking more than a given duration.  The caller
 * is responsible for eventually releasing the lock with
 * tao_remote_sensor_unlock().
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @param secs   Maximum time to wait (in seconds).  If this amount of time is
 *               very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling tao_remote_sensor_lock().  If
 *               this amount of time is very short, the effect is the same as
 *               calling tao_remote_sensor_try_lock().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_sensor_timed_lock(
    tao_remote_sensor* wfs,
    double secs);

/**
 * Signal a condition variable to at most one thread waiting on a remote
 * wavefront sensor.
 *
 * This function restarts one of the threads that are waiting on the condition
 * variable of the sensor.  Nothing happens, if no threads are waiting on the
 * condition variable.
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_sensor_broadcast_condition(),
 *      tao_remote_sensor_wait_condition().
 */
extern tao_status tao_remote_sensor_signal_condition(
    tao_remote_sensor* wfs);

/**
 * Signal a condition to all threads waiting on a remote wavefront sensor.
 *
 * This function behaves like tao_remote_sensor_signal_condition() except that
 * all threads waiting on the condition variable of the sensor are restarted.
 * Nothing happens, if no threads are waiting on the condition variable.
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_sensor_signal_condition(),
 *      tao_remote_sensor_wait_condition().
 */
extern tao_status tao_remote_sensor_broadcast_condition(
    tao_remote_sensor* wfs);

/**
 * Wait for a condition to be signaled for a remote wavefront sensor.
 *
 * This function atomically unlocks the exclusive lock associated with the
 * remote wavefront sensor and waits for its associated condition variable to
 * be signaled.  The thread execution is suspended and does not consume any CPU
 * time until the condition variable is signaled.  The mutex of the sensor must
 * have been locked (e.g., with tao_remote_sensor_lock()) by the calling thread
 * on entrance to this function.  Before returning to the calling thread, this
 * function re-acquires the mutex.
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_sensor_lock(), tao_remote_sensor_signal_condition().
 */
extern tao_status tao_remote_sensor_wait_condition(
    tao_remote_sensor* wfs);

/**
 * Wait for a condition to be signaled for a remote wavefront sensor without
 * blocking longer than an absolute time limit.
 *
 * This function behaves like tao_remote_sensor_wait_condition() but blocks no
 * longer than a given duration.
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @param lim    Absolute time limit with the same conventions as
 *               tao_get_current_time().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_sensor_abstimed_wait_condition(
    tao_remote_sensor* wfs,
    const tao_time* lim);

/**
 * Wait for a condition to be signaled for a remote wavefront sensor without
 * blocking longer than a relative time limit.
 *
 * This function behaves like tao_remote_sensor_wait_condition() but blocks no
 * longer than a given duration.
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller.
 *
 * @param secs   Maximum amount of time (in seconds).  If this amount of time
 *               is very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling
 *               tao_remote_sensor_wait_condition().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_sensor_timed_wait_condition(
    tao_remote_sensor* wfs,
    double secs);

/**
 * Get the name of the owner of a remote wavefront sensor.
 *
 * This function yields the name of the owner of the remote wavefront sensor.
 * This information is immutable and the sensor needs not be locked by the
 * caller.
 *
 * @param wfs     Pointer to a remote wavefront sensor attached to the address
 *                space of the caller.
 *
 * @return The name of the remote wavefront sensor owner or an empty string
 *         `""` for a `NULL` sensor pointer.  Whatever the result, this getter
 *         function leaves the caller's last error unchanged.
 */
extern const char* tao_remote_sensor_get_owner(
    const tao_remote_sensor* wfs);

/**
 * Get the number of output data-frames of a remote wavefront sensor.
 *
 * This function yields the length of the cyclic list of data-frames memorized
 * by the owner of a remote wavefront sensor.  This information is immutable
 * and the sensor needs not be locked by the caller.
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller and locked by the caller.
 *
 * @return The length of the list of shared arrays memorized by the owner of
 *         the remote wavefront sensor, `0` if @a wfs is `NULL`.  Whatever the
 *         result, this getter function leaves the caller's last error
 *         unchanged.
 *
 * @see tao_remote_sensor_lock.
 */
extern long tao_remote_sensor_get_nbufs(
    const tao_remote_sensor* wfs);

/**
 * Get the serial number of the last available data-frame for a remote
 * wavefront sensor.
 *
 * This function yields the serial number of the last data-frame available from
 * a remote wavefront sensor.  This is also the number of data-frames posted so
 * far by the server owning the remote wavefront sensor.
 *
 * The serial number of last data-frame may change (i.e., when acquisition is
 * running), but serial number is stored in an *atomic* variable, so the caller
 * needs not lock the remote wavefront sensor.
 *
 * @param wfs    Pointer to a remote wavefront sensor attached to the address
 *               space of the caller and locked by the caller.
 *
 * @return A nonnegative integer.  A strictly positive value which is the
 *         serial number of the last available image if any, `0` if image
 *         acquisition has not yet started of if `wfs` is `NULL`.  Whatever the
 *         result, this getter function leaves the caller's last error
 *         unchanged.
 *
 * @see tao_remote_sensor_lock.
 */
extern tao_serial tao_remote_sensor_get_serial(
    const tao_remote_sensor* wfs);

/**
 * Get the number of commands processed by the server owning a remote wavefront
 * sensor.
 *
 * This function yields the the number of commands processed so far by the
 * owner of the remote wavefront sensor.
 *
 * The number of processed commands is stored in an *atomic* variable, so the
 * caller needs not lock the remote camera.
 *
 * @param wfs     Pointer to a remote wavefront sensor attached to the address
 *                space of the caller.
 *
 * @return The number processed commands, a nonnegative integer which may be
 *         `0` if no commands have been ever processed or if `wfs` is `NULL`.
 *         Whatever the result, this getter function leaves the caller's last
 *         error unchanged.
 */
extern tao_serial tao_remote_sensor_get_ncmds(
    const tao_remote_sensor* wfs);

/**
 * Get the current state of the server owning a remote wavefront sensor.
 *
 * This function yields the current state of the server owning the remote
 * sensor.
 *
 * The server state is stored in an *atomic* variable, so the caller needs not
 * lock the remote wavefront sensor.
 *
 * @param wfs     Pointer to a remote wavefront sensor attached to the address
 *                space of the caller.
 *
 * @return The state of the remote server, @ref TAO_STATE_UNREACHABLE if `wfs`
 *         is `NULL`.  Whatever the result, this getter function leaves the
 *         caller's last error unchanged.
 */
extern tao_state tao_remote_sensor_get_state(
    const tao_remote_sensor* wfs);

/**
 * Check whether the server owning a remote wavefront sensor is alive.
 *
 * This function uses the current state of the server owning the remote
 * wavefront sensor to determine whether the server is alive.
 *
 * The server state is stored in an *atomic* variable, so the caller needs not
 * lock the remote wavefront sensor.
 *
 * @param wfs     Pointer to a remote wavefront sensor attached to the address
 *                space of the caller.
 *
 * @return A boolean result; `false` if `wfs` is `NULL`.  Whatever the result,
 *         this getter function leaves the caller's last error unchanged.
 */
extern int tao_remote_sensor_is_alive(
    const tao_remote_sensor* wfs);

// Getters.

extern long tao_remote_sensor_get_ninds(
    const tao_remote_sensor* wfs);

const long* tao_remote_sensor_get_dims(
    const tao_remote_sensor* wfs);

extern long tao_remote_sensor_get_nsubs(
    const tao_remote_sensor* wfs);

extern long tao_remote_sensor_get_max_ninds(
    const tao_remote_sensor* wfs);

extern long tao_remote_sensor_get_max_nsubs(
    const tao_remote_sensor* wfs);

extern const char* tao_remote_sensor_get_camera_owner(
    const tao_remote_sensor* wfs);

extern long tao_remote_sensor_get_camera_width(
    const tao_remote_sensor* wfs);

extern long tao_remote_sensor_get_camera_height(
    const tao_remote_sensor* wfs);

extern tao_shmid tao_remote_sensor_get_camera_shmid(
    const tao_remote_sensor* wfs);

extern const long* tao_remote_sensor_get_inds(
    const tao_remote_sensor* wfs,
    long* dims);

extern const tao_subimage* tao_remote_sensor_get_subs(
    const tao_remote_sensor* wfs, long* nsubs);

extern tao_status tao_remote_sensor_fetch_data(
    const tao_remote_sensor* obj,
    tao_serial               serial,
    tao_shackhartmann_data*  data,
    long                     ndata,
    tao_dataframe_info*      info);

/**
 * Get the camera of a remote wavefront sensor.
 *
 * @param wfs     Pointer to a remote wavefront sensor attached to the address
 *                space of the caller.
 *
 * @param shmid   Address where to store the shared memory identifier of
 *                the camera.  Can be `NULL` to not retrieve this information.
 *
 * @param dims    Address where to store the dimensions (width and height) of
 *                the camera.  Can be `NULL` to not retrieve this information.
 *
 * @return The name of the server owning the camera of the remote wavefront
 *         sensor; an empty string if `wfs` is `NULL` or currently has no
 *         associated remote camera.  Whatever the result, this getter function
 *         leaves the caller's last error unchanged.
 */
const char* tao_remote_sensor_get_camera(
    const tao_remote_sensor* wfs,
    tao_shmid* shmid,
    long* dims);

/**
 * Tune the run-time parameters of a remote wavefront sensor.
 *
 * This function tunes the run-time parameters of a remote wavefront sensor.
 *
 * The caller must have locked the remote wavefront sensor.  Once the lock is
 * released, the new parameters will be used by the server for subsequent
 * iterations.  Calling this function automatically signal any changes.  Only
 * the run-time parameters of this configuration can be different from those of
 * the current server configuration.  So a typical usage is:
 *
 * @code {.c}
 * tao_remote_sensor_lock(wfs);
 * tao_remote_sensor_get_config(wfs, &cfg, NULL, NULL, NULL, NULL, 0, NULL, 0);
 * cfg.forgetting_factor = new_forgetting_factor_value;
 * tao_remote_sensor_tune_config(wfs, &cfg);
 * tao_remote_sensor_unlock(wfs);
 * @endcode
 *
 * @param wfs     The destination remote wavefront sensor.
 *
 * @param cfg     The configuration with the new settings.  Must not be `NULL`.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_remote_sensor_tune_config(
    tao_remote_sensor* wfs,
    const tao_shackhartmann_config* cfg);

/**
 * Get the current configuration of a remote wavefront sensor.
 *
 * This function retrieves the current settings of a remote wavefront sensor.
 *
 * The caller must have locked the remote wavefront sensor for exclusive access.
 *
 * @param wfs           The source remote wavefront sensor.
 *
 * @param cfg           The destination configuration.  Must not be `NULL`.
 *
 * @param camera_owner  A buffer of at least @ref TAO_OWNER_SIZE characters
 *                      to store the name of the server owning the wavefront
 *                      sensor camera.  Not used if `NULL`.
 *
 * @param camera_shmid  The address to store the shared memory identifier of
 *                      the server owning the wavefront sensor camera.  Not
 *                      used if `NULL`.
 *
 * @param camera_dims   A buffer of at least 2 entries to store the dimensions
 *                      of the wavefront sensor camera.  Not used if `NULL`.
 *
 * @param inds          A buffer to store the grid of the indices of the
 *                      wavefront sensor sub-images.  Not used if `NULL`.
 *
 * @param max_ninds     The maximum number of entries in `inds`.
 *
 * @param subs          A buffer to store the descriptors of the wavefront
 *                      sensor sub-images.  Not used if `NULL`.
 *
 * @param max_nsubs     The maximum number of entries in `subs`.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_remote_sensor_get_config(
    const tao_remote_sensor* wfs,
    tao_shackhartmann_config* cfg,
    char* camera_owner,
    tao_shmid* camera_shmid,
    long* camera_dims,
    long* inds,
    long max_ninds,
    tao_subimage* subs,
    long max_nsubs);

/**
 * Check a configuration for a remote wavefront sensor.
 *
 * This function checks the settings of a remote wavefront sensor.  The
 * arguments are the same as those of tao_remote_sensor_configure.
 *
 * The caller must have locked the remote wavefront sensor for exclusive
 * access.
 *
 * @param wfs           The target remote wavefront sensor.
 *
 * @param cfg           The configuration.  Must not be `NULL`.
 *
 * @param camera_owner  The name of the server owning the wavefront sensor
 *                      camera.  Not used if `NULL`.
 *
 * @param camera_shmid  The shared memory identifier of the server owning the
 *                      wavefront sensor camera.  Not used if @ref
 *                      TAO_BAD_SHMID.
 *
 * @param inds          The 2-dimensional array of indices of the wavefront
 *                      sensor sub-images.  Assumed unchanged if `NULL`;
 *                      otherwise, must have `cfg->dims[0]*cfg->dims[1]`
 *                      entries.
 *
 * @param subs          The descriptors of the wavefront sensor sub-images.
 *                      Assumed unchanged if `NULL`; otherwise, must have
 *                      `cfg->nsubs` entries.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_remote_sensor_check_config(
    const tao_remote_sensor* wfs,
    const tao_shackhartmann_config* cfg,
    const char* camera_owner,
    tao_shmid camera_shmid,
    const long* inds,
    const tao_subimage* subs);

/**
 * Change the configuration of a remote wavefront sensor.
 *
 * This function send a command to change the settings of a remote wavefront
 * sensor.  The arguments are the same as those of
 * tao_remote_sensor_check_config.
 *
 * The command is executed asynchronously.  The returned value is the serial
 * number of the command so that the caller can call
 * tao_remote_sensor_wait_command() to make sure that the command has been
 * completed.
 *
 * The caller must not have locked the remote wavefront sensor.
 *
 * @param wfs           The target remote wavefront sensor.
 *
 * @param cfg           The configuration.  Must not be `NULL`.
 *
 * @param camera_owner  The name of the server owning the wavefront sensor
 *                      camera.  Not used if `NULL`.
 *
 * @param camera_shmid  The shared memory identifier of the server owning the
 *                      wavefront sensor camera.  Not used if @ref
 *                      TAO_BAD_SHMID.
 *
 * @param inds          The 2-dimensional array of indices of the wavefront
 *                      sensor sub-images.  Assumed unchanged if `NULL`;
 *                      otherwise, must have `cfg->dims[0]*cfg->dims[1]`
 *                      entries.
 *
 * @param subs          The descriptors of the wavefront sensor sub-images.
 *                      Assumed unchanged if `NULL`; otherwise, must have
 *                      `cfg->nsubs` entries.
 *
 * @param secs          The maximum number of seconds to wait.
 *
 * @return The serial number of the "*start*" command, 0 if the command cannot
 *         be sent before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_sensor_configure(
    tao_remote_sensor* wfs,
    const tao_shackhartmann_config* cfg,
    const char* camera_owner,
    tao_shmid camera_shmid,
    const long* inds,
    const tao_subimage* subs,
    double secs);

/**
 * Start processing by a remote wavefront sensor.
 *
 * A client can call this function to start processing by a remote wavefront
 * sensor.
 *
 * The command is executed asynchronously.  The returned value is the serial
 * number of the command so that the caller can call
 * tao_remote_sensor_wait_command() to make sure that the command has been
 * completed.
 *
 * The caller must not have locked the remote wavefront sensor.
 *
 * @param cam   remote wavefront sensor instance.
 *
 * @param secs  Maximum number of seconds to wait.
 *
 * @return The serial number of the "*start*" command, 0 if the command cannot
 *         be sent before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_sensor_start(
    tao_remote_sensor* cam,
    double secs);

/**
 * Stop processing by a remote wavefront sensor.
 *
 * A client can call this function to stop processing by a remote wavefront
 * sensor.  The command is executed asynchronously.  The returned value is the
 * serial number of the command so that the caller can call
 * tao_remote_sensor_wait_command() to make sure that the command has been
 * completed.
 *
 * The caller must not have locked the remote wavefront sensor.
 *
 * @param cam   remote wavefront sensor instance.
 *
 * @param secs  Maximum number of seconds to wait.
 *
 * @return The serial number of the "*stop*" command, 0 if the command cannot
 *         be sent before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_sensor_stop(
    tao_remote_sensor* cam,
    double secs);

/**
 * Kill a remote wavefront sensor server.
 *
 * A client can call this function to kill the server owning a remote wavefront
 * sensor.
 *
 * The command is executed asynchronously.  The returned value is the serial
 * number of the command so that the caller can call
 * tao_remote_sensor_wait_command() to make sure that the command has been
 * completed.
 *
 * The caller must not have locked the remote wavefront sensor.
 *
 * @param cam   Remote wavefront sensor instance.
 *
 * @param secs  Maximum number of seconds to wait.
 *
 * @return The serial number of the "*kill*" command, 0 if the command cannot
 *         be sent before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_sensor_kill(
    tao_remote_sensor* wfs,
    double secs);

/**
 * Wait for a given command to have been processed.
 *
 * This function waits for a specific command sent to the server owning a
 * remote wavefront sensor to have been processed.
 *
 * @warning The caller must not have locked the remote wavefront sensor.
 *
 * @param wfs     Pointer to a remote wavefront sensor attached to the address
 *                space of the caller.
 *
 * @param num     The serial number of the command to wait for.
 *
 * @param secs    Maximum amount of time to wait (in seconds).
 *
 * @return @ref TAO_OK on success, @ref TAO_TIMEOUT if the command has not been
 *         processed before the time limit, and @ref TAO_ERROR on failure.
 */
extern tao_status tao_remote_sensor_wait_command(
    tao_remote_sensor* wfs,
    tao_serial         num,
    double             secs);

/**
 * Wait for a given wavefront sensor data-frame.
 *
 * This function waits for a specific data-frame to be available.  Upon
 * success, the contents of the data-frame should be copied as soon as possible
 * with tao_remote_sensor_fetch_data().
 *
 * The caller must not have locked the wavefront sensor.
 *
 * Typical usage:
 *
 * ~~~~~{.c}
 * tao_shmid shmid = tao_config_read_shmid(sensor_name);
 * tao_remote_sensor* wfs = tao_remote_sensor_attach(shmid);
 * FIXME: long nsubs = tao_remote_sensor_get_nsubs(wfs);
 * FIXME: size_t size = data->nsubs*sizeof(double);
 * FIXME: double* refs = malloc(size);
 * FIXME: double* cmds = malloc(size);
 * tao_serial serial = tao_remote_sensor_wait_output(wfs, 0, 3.2);
 * if (serial > 0) {
 *     tao_dataframe_info info;
 *     tao_status status = tao_remote_sensor_fetch_data(
 *         wfs, serial, refs, cmds, data->nsubs, &info);
 *     if (status == TAO_OK) {
 *        // Process data-frame.
 *        ....
 *     }
 * } else {
 *     // Deal with exception.
 *     ...;
 * }
 * ~~~~~
 *
 * @param wfs      Pointer to remote wavefront sensor in caller's address
 * space.
 *
 * @param serial   The serial number of the frame to wait for.  If less or
 *                 equal zero, the next frame is waited for.
 *
 * @param secs     Maximum number of seconds to wait.
 *
 * @return A strictly positive number which is the serial number of the
 *         requested frame, `0` if the requested frame is not available before
 *         the time limit (i.e. timeout), `-1` if the requested frame is too
 *         old (it has been overwritten by some newer frames or it is beyond
 *         the last available frame), `-2` if the server has been killed and
 *         the requested frame is beyond the last available one, or `-3` in
 *         case of failure.  In the latter case, error details are reflected by
 *         the caller's last error.
 */
extern tao_serial tao_remote_sensor_wait_output(
    tao_remote_sensor* wfs,
    tao_serial         serial,
    double             secs);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_REMOTE_SENSORS_H_
