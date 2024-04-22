// tao-remote-mirrors.h -
//
// Definitions for remote deformable mirrors in TAO.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_REMOTE_MIRRORS_H_
#define TAO_REMOTE_MIRRORS_H_ 1

#include <tao-basics.h>
#include <tao-remote-objects.h>

TAO_BEGIN_DECLS

/**
 * @defgroup RemoteMirrors  Remote deformable mirrors
 *
 * @ingroup DeformableMirrors
 *
 * @brief Client/server interface for deformable mirrors.
 *
 * A remote mirror instance is a structure stored in shared memory which is
 * used to communicate with a deformable mirror server.  The shared structure
 * contains the current settings of the deformable mirror and an history of the
 * commands sent to the device.
 *
 * @{
 */

/**
 * Remote deformable mirror.
 *
 * A remote deformable mirror is created by a server by calling
 * tao_remote_mirror_create(), clients call tao_remote_mirror_attach() to
 * connect to the remote mirror.  The server and the clients call
 * tao_remote_mirror_detach() when they no longer need access to the remote
 * mirror.  A remote deformable mirror is a remote shared object with the
 * following additional components:
 *
 * - A 2-dimensional array of indices describing the layout of the actuators
 *   which can be retrieved by calling tao_remote_mirror_get_layout().
 *
 * - An internal buffer storing the reference commands which can be retrieved
 *   by calling tao_remote_mirror_get_reference() and which is set by
 *   tao_remote_mirror_set_reference().
 *
 * - An internal buffer storing perturbations of the actuators commands and
 *   which is set by tao_remote_mirror_set_perturbation().
 *
 * - An internal buffer storing the requested commands and which is set by
 *   tao_remote_mirror_send_commands().
 *
 * - A cyclic list of output buffers storing an history of the commands
 *   actually applied to the mirror.  Waiting for a given output buffer is done
 *   by calling tao_remote_mirror_wait_output() and retrieving the contents of
 *   an output buffer is done by calling tao_remote_mirror_fetch_data().
 *
 * Commands sent to the deformable mirror device are the sum of the requested
 * comands, of the reference commands, and of the perturbations of the
 * commands.  Since the deformable mirror may has limitations on the possible
 * actuators commands, the actual commands applied to the deformable mirror may
 * be different than the requested ones.  All this 4 sets of actuators commands
 * can be retrieved in the output buffers published by the server owning the
 * deformable mirror.
 */
typedef struct tao_remote_mirror tao_remote_mirror;

/**
 * Create a new instance of a remote deformable mirror.
 *
 * This function creates the resources in shared memory to manage a remote
 * deformable mirror and its telemetry.  This function shall be called by the
 * server in charge of a deformable mirror device.  Clients shall call
 * tao_remote_mirror_attach() to connect to the remote deformable
 * mirror.  The clients and the server are responsible of eventually calling
 * tao_remote_mirror_detach() to release the resources.
 *
 * In the returned instance, the reference commands are set to the mean of
 * `cmin` and `cmax`, the perturbation and actuators commands are set to zero.
 *
 * @param owner   The name of the server.
 *
 * @param nbufs   The number of cyclic data-frame buffers.
 *
 * @param inds    The layout of the actuators.
 *
 * @param dim1    The first dimension of the grid of actuators.
 *
 * @param dim2    The second dimension of the grid of actuators.
 *
 * @param cmin    The minimal value for an actuator command.
 *
 * @param cmax    The maximal value for an actuator command.
 *
 * @param flags   Permissions for clients and options.
 *
 *
 * @return The address of the new remote deformable mirror instance or `NULL`
 *         in case of errors.
 */
tao_remote_mirror* tao_remote_mirror_create(
    const char* owner,
    long        nbufs,
    const long* inds,
    long        dim1,
    long        dim2,
    double      cmin,
    double      cmax,
    unsigned    flags);

/**
 * @brief Attach an existing remote mirror to the address space of the caller.
 *
 * This function attaches an existing remote mirror to the address space of the
 * caller.  As a result, the number of attachments on the returned mirror is
 * incremented by one.  When the mirror is no longer used by the caller, the
 * caller is responsible of calling tao_remote_mirror_detach() to detach the
 * mirror from its address space, decrement its number of attachments by one
 * and eventually free the shared memory associated with the mirror.
 *
 * In principle, the same process may attach a remote mirror more than once but
 * each attachment, due to tao_remote_mirror_attach() or to
 * tao_remote_mirror_create(), should be matched by a
 * tao_remote_mirror_detach() with the corresponding address in the caller's
 * address space.
 *
 * @param shmid  Shared memory identifier.
 *
 * @return The address of the remote mirror in the address space of the caller;
 *         `NULL` in case of failure.  Even tough the arguments are correct, an
 *         error may arise if the mirror has been destroyed before attachment
 *         completes.
 *
 * @see tao_remote_mirror_detach().
 */
extern tao_remote_mirror* tao_remote_mirror_attach(
    tao_shmid shmid);

/**
 * @brief Detach a remote mirror from the address space of the caller.
 *
 * This function detaches a remote mirror from the address space of the caller
 * and decrements the number of attachments of the remote mirror.  If the
 * number of attachements reaches zero, the shared memory segment backing the
 * storage of the mirror is destroyed (unless bit @ref TAO_PERSISTENT was set
 * at mirror creation).
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_mirror_attach().
 */
extern tao_status tao_remote_mirror_detach(
    tao_remote_mirror* obj);

/**
 * @brief Get the size of a remote mirror.
 *
 * This function yields the number of bytes of shared memory occupied by the
 * remote mirror.  The size is constant for the life of the mirror, it is thus
 * not necessary to have locked the mirror to retrieve its identifier.
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @return The number of bytes of the shared memory segment backing the storage
 *         of the remote mirror, `0` if @a obj is `NULL`.  Whatever the result,
 *         this getter function leaves the caller's last error unchanged.
 */
extern size_t tao_remote_mirror_get_size(
    const tao_remote_mirror* obj);

/**
 * @brief Get the type identifier of a remote mirror.
 *
 * This function yields the identifier of the type of the remote mirror.  The
 * type identifier is constant for the life of the mirror, it is thus not
 * necessary to have locked the mirror to retrieve its identifier.
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @return The type identifier of the remote mirror, `0` if @a obj is `NULL`.
 *         Whatever the result, this getter function leaves the caller's last
 *         error unchanged.
 */
extern uint32_t tao_remote_mirror_get_type(
    const tao_remote_mirror* obj);

/**
 * @brief Get the shared memory identifier of a remote mirror.
 *
 * This function yields the shared memory identifier of the remote mirror.
 * This value can be used by another process to attach to its address space the
 * remote mirror.  The shared memory identifier is constant for the life of the
 * mirror, it is thus not necessary to have locked the mirror to retrieve its
 * identifier.
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @return The identifier of the remote mirror data, `TAO_BAD_SHMID` if @a obj
 *         is `NULL`.  Whatever the result, this getter function leaves the
 *         caller's last error unchanged.
 *
 * @see tao_remote_mirror_attach.
 */
extern tao_shmid tao_remote_mirror_get_shmid(
    const tao_remote_mirror* obj);

/**
 * Lock a remote mirror for exclusive access.
 *
 * This function locks a remote mirror for exclusive (read and write) access.
 * The mirror must be attached to the address space of the caller.  In case of
 * success, the caller is responsible for calling tao_unlock_shared_mirror()
 * to eventually release the lock.
 *
 * @warning The same thread/process must not attempt to lock the same mirror
 * more than once and should unlock it as soon as possible.
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_remote_mirror_lock(
    tao_remote_mirror* obj);

/**
 * Unlock a remote mirror.
 *
 * This function unlocks a remote mirror that has been successfully locked by
 * the caller.
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_remote_mirror_unlock(
    tao_remote_mirror* obj);

/**
 * Attempt to immediately lock a remote mirror for exclusive access.
 *
 * This function attempts to lock a remote mirror for exclusive (read and
 * write) access without blocking.  The caller is responsible for eventually
 * releasing the lock with tao_remote_mirror_unlock().
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success, @ref TAO_TIMEOUT if the lock cannot be
 *         immediately acquired, or @ref TAO_ERROR on failure.
 */
extern tao_status tao_remote_mirror_try_lock(
    tao_remote_mirror* obj);

/**
 * Attempt to lock a remote mirror for exclusive access with an absolute time
 * limit.
 *
 * This function attempts to lock a remote mirror for exclusive (read and
 * write) access without blocking beyond a given time limit.  The caller is
 * responsible for eventually releasing the lock with
 * tao_remote_mirror_unlock().
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @param lim    Absolute time limit.
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_mirror_abstimed_lock(
    tao_remote_mirror* obj,
    const tao_time* lim);

/**
 * Attempt to lock a remote mirror for exclusive access with a relative time
 * limit.
 *
 * This function attempts to lock a remote mirror for exclusive (read and
 * write) access without blocking more than a given duration.  The caller is
 * responsible for eventually releasing the lock with
 * tao_remote_mirror_unlock().
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @param secs   Maximum time to wait (in seconds).  If this amount of time is
 *               very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling tao_remote_mirror_lock().  If
 *               this amount of time is very short, the effect is the same as
 *               calling tao_remote_mirror_try_lock().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_mirror_timed_lock(
    tao_remote_mirror* obj,
    double secs);

/**
 * Signal a condition variable to at most one thread waiting on a remote mirror.
 *
 * This function restarts one of the threads that are waiting on the condition
 * variable of the mirror.  Nothing happens, if no threads are waiting on the
 * condition variable.
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_mirror_broadcast_condition(),
 *      tao_remote_mirror_wait_condition().
 */
extern tao_status tao_remote_mirror_signal_condition(
    tao_remote_mirror* obj);

/**
 * Signal a condition to all threads waiting on a remote mirror.
 *
 * This function behaves like tao_remote_mirror_signal_condition() except that
 * all threads waiting on the condition variable of the mirror are restarted.
 * Nothing happens, if no threads are waiting on the condition variable.
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_mirror_signal_condition(),
 *      tao_remote_mirror_wait_condition().
 */
extern tao_status tao_remote_mirror_broadcast_condition(
    tao_remote_mirror* obj);

/**
 * Wait for a condition to be signaled for a remote mirror.
 *
 * This function atomically unlocks the exclusive lock associated with the
 * remote mirror and waits for its associated condition variable to be
 * signaled.  The thread execution is suspended and does not consume any CPU
 * time until the condition variable is signaled.  The mutex of the mirror must
 * have been locked (e.g., with tao_remote_mirror_lock()) by the calling thread
 * on entrance to this function.  Before returning to the calling thread, this
 * function re-acquires the mutex.
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_mirror_lock(),
 *      tao_remote_mirror_signal_condition().
 */
extern tao_status tao_remote_mirror_wait_condition(
    tao_remote_mirror* obj);

/**
 * Wait for a condition to be signaled for a remote mirror without blocking
 * longer than an absolute time limit.
 *
 * This function behaves like tao_remote_mirror_wait_condition() but blocks no
 * longer than a given duration.
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @param lim    Absolute time limit with the same conventions as
 *               tao_get_current_time().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_mirror_abstimed_wait_condition(
    tao_remote_mirror* obj,
    const tao_time* lim);

/**
 * Wait for a condition to be signaled for a remote mirror without blocking
 * longer than a relative time limit.
 *
 * This function behaves like tao_remote_mirror_wait_condition() but blocks no
 * longer than a given duration.
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller.
 *
 * @param secs   Maximum amount of time (in seconds).  If this amount of time
 *               is very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling
 *               tao_remote_mirror_wait_condition().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_mirror_timed_wait_condition(
    tao_remote_mirror* obj,
    double secs);

/**
 * Get the name of the owner of a remote mirror.
 *
 * This function yields the name of the owner of the remote mirror.  This
 * information is immutable and the mirror needs not be locked by the caller.
 *
 * @param obj     Pointer to a remote mirror attached to the address space of
 *                the caller.
 *
 * @return The name of the remote mirror owner or an empty string `""` for a
 *         `NULL` mirror pointer.  Whatever the result, this getter function
 *         leaves the caller's last error unchanged.
 */
extern const char* tao_remote_mirror_get_owner(
    const tao_remote_mirror* obj);

/**
 * Get the number of output data-frames of a remote mirror.

 * This function yields the length of the cyclic list of data-frames memorized
 * by the owner of a remote mirror.  This information is immutable and the
 * mirror needs not be locked by the caller.
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller and locked by the caller.
 *
 * @return The length of the list of shared arrays memorized by the owner of
 *         the remote mirror, `0` if @a obj is `NULL`.  Whatever the result,
 *         this getter function leaves the caller's last error unchanged.
 *
 * @see tao_remote_mirror_lock.
 */
extern long tao_remote_mirror_get_nbufs(
    const tao_remote_mirror* obj);

/**
 * Get the serial number of the last available data-frame for a remote mirror.
 *
 * This function yields the serial number of the last data-frame available from
 * a remote mirror.  This is also the number of data-frames posted so far by
 * the server owning the remote mirror.
 *
 * The serial number of last data-frame may change (i.e., when acquisition is
 * running), but serial number is stored in an *atomic* variable, so the caller
 * needs not lock the remote mirror.
 *
 * @param obj    Pointer to a remote mirror attached to the address space of
 *               the caller and locked by the caller.
 *
 * @return A nonnegative integer.  A strictly positive value which is the
 *         serial number of the last available image if any, `0` if image
 *         acquisition has not yet started of if `obj` is `NULL`.  Whatever the
 *         result, this getter function leaves the caller's last error
 *         unchanged.
 *
 * @see tao_remote_mirror_lock.
 */
extern tao_serial tao_remote_mirror_get_serial(
    const tao_remote_mirror* obj);

/**
 * Get the number of commands processed by the server owning a remote mirror.
 *
 * This function yields the the number of commands processed so far by the
 * owner of the remote mirror.
 *
 * The number of processed commands is stored in an *atomic* variable, so the
 * caller needs not lock the remote camera.
 *
 * @param obj     Pointer to a remote mirror attached to the address space of
 *                the caller.
 *
 * @return The number processed commands, a nonnegative integer which may be
 *         `0` if no commands have been ever processed or if `obj` is `NULL`.
 *         Whatever the result, this getter function leaves the caller's last
 *         error unchanged.
 */
extern tao_serial tao_remote_mirror_get_ncmds(
    const tao_remote_mirror* obj);

/**
 * Get the current state of the server owning a remote mirror.
 *
 * This function yields the current state of the server owning the remote
 * mirror.
 *
 * The server state is stored in an *atomic* variable, so the caller needs not
 * lock the remote mirror.
 *
 * @param obj     Pointer to a remote mirror attached to the address space of
 *                the caller.
 *
 * @return The state of the remote server, @ref TAO_STATE_UNREACHABLE if `obj` is
 *         `NULL`.  Whatever the result, this getter function leaves the
 *         caller's last error unchanged.
 */
extern tao_state tao_remote_mirror_get_state(
    const tao_remote_mirror* obj);

/**
 * Check whether the server owning a remote mirror is alive.
 *
 * This function uses the current state of the server owning the remote mirror
 * to determine whether the server is alive.
 *
 * The server state is stored in an *atomic* variable, so the caller needs not
 * lock the remote mirror.
 *
 * @param obj     Pointer to a remote mirror attached to the address space of
 *                the caller.
 *
 * @return A boolean result; `false` if `obj` is `NULL`.  Whatever the result,
 *         this getter function leaves the caller's last error unchanged.
 */
extern int tao_remote_mirror_is_alive(
    const tao_remote_mirror* obj);

/**
 * Get the mark set for the next deformable mirror frame.
 *
 * The caller shall own the lock on the remote deformable mirror.
 *
 * @param obj   Pointer to remote deformable mirror in caller's address space
 *             (can be `NULL`).
 *
 * @return The mark set for the next deformable mirror frame, 0 if `obj` is
 *         `NULL` or no mark has been applied yet.  Whatever the result, this
 *         getter function leaves the caller's last error unchanged.
 */
extern tao_serial tao_remote_mirror_get_mark(
    const tao_remote_mirror* obj);

/**
 * Get the number of actuators of a remote deformable mirror.
 *
 * The number of actuators is a constant.  It is not needed to lock the remote
 * deformable mirror before calling this function.
 *
 * @param obj  Address of remote deformable mirror instance (can be `NULL`).
 *
 * @return The number of actuators, 0 if `obj` is `NULL`.  Whatever the result,
 *         this getter function leaves the caller's last error unchanged.
 */
extern long tao_remote_mirror_get_nacts(
    const tao_remote_mirror* obj);

/**
 * Get the dimensions of the actuator grid of a remote deformable mirror.
 *
 * The dimensions of the actuator grid are constant.  It is not needed to lock
 * the remote deformable mirror before calling this function.
 *
 * @param obj  Address of remote deformable mirror instance (can be `NULL`).
 *
 * @return An array of integers storing the 2 dimensions of the actuator grid,
 *         `NULL` if `obj` is `NULL`.  Whatever the result, this getter
 *         function leaves the caller's last error unchanged.
 */
extern const long* tao_remote_mirror_get_dims(
    const tao_remote_mirror* obj);

/**
 * Get the layout of the actuators of a remote deformable mirror.
 *
 * The layout of the actuator grid of a remote deformable mirror is a `dims[0]`
 * by `dims[1]` array of indices stored in column-major order.  A negative
 * index indicates that there are no actuators at the corresponding position;
 * otherwise, the value is the 0-based index in the vector of actuator
 * commands.
 *
 * The layout of the actuator grid is constant.  It is not needed to lock the
 * remote deformable mirror before calling this function.
 *
 * @param obj   Address of remote deformable mirror instance (can be `NULL`).
 *
 * @param dims  If non-`NULL`, array of 2 integers to retrieve the dimensions
 *              of the actuator grid.
 *
 * @return An array of integers storing the indices of the actuators, `NULL` if
 *         `obj` is `NULL`.  Whatever the result, this getter function leaves
 *         the caller's last error unchanged.
 */
extern const long* tao_remote_mirror_get_layout(
    const tao_remote_mirror* obj,
    long dims[2]);

/**
 * Get the minimal value for an actuator commands.
 *
 * The minimal value is constant.  It is not needed to lock the
 * remote deformable mirror before calling this function.
 *
 * @param obj   Address of remote deformable mirror instance (can be `NULL`).
 *
 * @return A minimal value, `NaN` if if `obj` is `NULL`.  Whatever the result,
 *         this getter function leaves the caller's last error unchanged.
 */
extern double tao_remote_mirror_get_cmin(
    const tao_remote_mirror* obj);

/**
 * Get the maximal value for an actuator commands.
 *
 * The maximal value is constant.  It is not needed to lock the
 * remote deformable mirror before calling this function.
 *
 * @param obj   Address of remote deformable mirror instance (can be `NULL`).
 *
 * @return A maximal value, `NaN` if if `obj` is `NULL`.  Whatever the result,
 *         this getter function leaves the caller's last error unchanged.
 */
extern double tao_remote_mirror_get_cmax(
    const tao_remote_mirror* obj);

/**
 * Obtain reference actuators commands of a remote deformable mirror.
 *
 * The actuator reference commands should be considered as read-only.  Call
 * tao_remote_mirror_set_reference() to change the reference
 * commands (and use them for further commands to the device).
 *
 * The caller shall own the lock on the remote deformable mirror to make sure
 * that the reference commands do not change.
 *
 * @param obj   Address of remote deformable mirror instance (can be `NULL`).
 *
 * @return An array of actuator reference commands, `NULL` if `obj` is `NULL`.
 *         Whatever the result, this getter function leaves the caller's last
 *         error unchanged.
 */
extern double *tao_remote_mirror_get_reference(
    const tao_remote_mirror* obj);

/**
 * Kill server owning a remote deformable mirror.
 *
 * This function manages to send a "*kill*" command to the server which owns a
 * remote deformable mirror.  The remote deformable mirror instance must not
 * have been locked by the caller.
 *
 * @param obj   Pointer to remote deformable mirror in caller's address space.
 *
 * @param secs  Maximum amount of time to wait (in seconds).
 *
 * @return The serial number of the "*kill*" command, 0 if the command cannot
 *         be sent before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_mirror_kill(
    tao_remote_mirror* obj,
    double secs);

/**
 * Set the reference for the subsequent actuators commands applied to a
 * deformable mirror.
 *
 * This function waits for the remote deformable mirror `obj` to become ready
 * for a new command and then sets the reference values for the actuators
 * commands.  These values will be used for the subsequent actuators commands
 * applied with tao_remote_mirror_send_commands() until another call to
 * tao_remote_mirror_set_reference() by any connected client.
 *
 * The remote deformable mirror must not have been locked by the caller.
 *
 * @param obj     Pointer to remote deformable mirror in caller's address
 *                space.
 *
 * @param vals    The reference command values.
 *
 * @param nvals   The number of values in `vals`, must be equal to the number
 *                of actuators.
 *
 * @param secs    Maximum amount of time to wait (in seconds).
 *
 * @param datnum  Address to store the serial number of the data-frame in the
 *                deformable mirror output telemetry where the command will be
 *                effective (not used if `NULL`).  The caller may wait on
 *                this value with tao_remote_mirror_wait_output() to wait for
 *                the first data-frame affected by the values in `vals`.
 *
 * @return The serial number of the "*set reference*" command, 0 if the command
 *         cannot be sent before the time limit, -1 in case of error.  If a
 *         positive value is returned, the caller may wait on this value with
 *         tao_remote_mirror_wait_command() to ensure that the new reference
 *         has been taken into account.
 */
extern tao_serial tao_remote_mirror_set_reference(
    tao_remote_mirror* obj,
    const double*      vals,
    long               nvals,
    double             secs,
    tao_serial*        datnum);

/**
 * Set a perturbation for the next actuators commands applied to a
 * deformable mirror.
 *
 * This function waits for the remote deformable mirror `obj` to become ready
 * for a new command and then sets the values of a perturbation of the
 * actuators commands.  These values will be used for the next actuators
 * commands applied with tao_remote_mirror_send_commands() and cleared when
 * this latter command is excuted.
 *
 * If another perturbation has been set before but not yet applied, the new
 * perturbation values replace the old ones.
 *
 * The remote deformable mirror must not have been locked by the caller.
 *
 * @param obj     Pointer to remote deformable mirror in caller's address
 *                space.
 *
 * @param vals    The values of the perturbation of the actuators commands.
 *
 * @param nvals   The number of values in `vals`, must be equal to the number
 *                of actuators.
 *
 * @param secs    Maximum amount of time to wait (in seconds).
 *
 * @param datnum  Address to store the serial number of the data-frame in the
 *                deformable mirror output telemetry where the effects of this
 *                command will be effective (not used if `NULL`).  The caller
 *                may wait on this value with tao_remote_mirror_wait_output()
 *                to wait for the data-frame affected by the values in `vals`.
 *
 * @return The serial number of the "*set perturbation*" command, 0 if the
 *         command cannot be sent before the time limit, -1 in case of error.
 *         If a positive value is returned, the caller may wait on this value
 *         with tao_remote_mirror_wait_command() to ensure that the new
 *         reference has been taken into account.
 */
extern tao_serial tao_remote_mirror_set_perturbation(
    tao_remote_mirror* obj,
    const double*      vals,
    long               nvals,
    double             secs,
    tao_serial*        datnum);

/**
 * Reset remote deformable mirror.
 *
 * This function manages to send a "*reset*" command to a remote deformable
 * mirror.  The remote deformable mirror must not have been locked by the
 * caller.  Calling this function is rigorously equivalent to calling
 * tao_remote_mirror_send_commands() with all requested actuators commands set
 * to zero.
 *
 * @param obj     Pointer to remote deformable mirror in caller's address
 *                space.
 *
 * @param mark    The number associated with the resulting data-frame in the
 *                deformable mirror telemetry.
 *
 * @param secs    Maximum amount of time to wait (in seconds).
 *
 * @param datnum  Address to store the serial number of the data-frame in the
 *                deformable mirror output telemetry where the command will be
 *                effective (not used if `NULL`).
 *
 * @return The serial number of the "*reset*" command, 0 if the command cannot
 *         be sent before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_mirror_reset(
    tao_remote_mirror* obj,
    tao_serial mark,
    double secs,
    tao_serial* datnum);

/**
 * Set the actuators of a remote deformable mirror.
 *
 * This function waits for the remote deformable mirror `obj` to become idle and
 * then requires that it applies the given actuator commands.  These values are
 * relative to the reference values set by the last call to
 * tao_remote_mirror_set_reference().
 *
 * @warning The remote deformable mirror must not have been locked by the
 *          caller.
 *
 * @param obj     Pointer to remote deformable mirror in caller's address
 *                space.
 *
 * @param vals    The actuator command values.
 *
 * @param nvals   The number of values in `vals`, must be equal to the number
 *                of actuators.
 *
 * @param mark    The number associated with the resulting data-frame in the
 *                deformable mirror telemetry.
 *
 * @param secs    Maximum amount of time to wait (in seconds).
 *
 * @param datnum  Address to store the serial number of the data-frame in the
 *                deformable mirror output telemetry where the command will be
 *                effective (not used if `NULL`).
 *
 * @return The serial number of the "*send*" command, 0 if the command cannot
 *         be sent before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_mirror_send_commands(
    tao_remote_mirror* obj,
    const double*      vals,
    long               nvals,
    tao_serial         mark,
    double             secs,
    tao_serial*        datnum);

/**
 * Wait for a given command to have been processed.
 *
 * This function waits for a specific command sent to the server owning a
 * remote mirror to have been processed.
 *
 * @warning The caller must not have locked the remote mirror.
 *
 * @param obj     Pointer to a remote mirror attached to the address space of
 *                the caller.
 *
 * @param cmdnum  The serial number of the command to wait for.
 *
 * @param secs    Maximum amount of time to wait (in seconds).
 *
 * @return @ref TAO_OK on success, @ref TAO_TIMEOUT if the command has not been
 *         processed before the time limit, and @ref TAO_ERROR on failure.
 */
extern tao_status tao_remote_mirror_wait_command(
    tao_remote_mirror* obj,
    tao_serial         cmdnum,
    double             secs);

/**
 * Wait for a given deformable mirror data-frame.
 *
 * This function waits for a specific data-frame to be available.  Upon
 * success, the contents of the data-frame should be copied as soon as possible
 * with tao_remote_mirror_fetch_data().
 *
 * The caller must not have locked the object.
 *
 * Typical usage:
 *
 * ~~~~~{.c}
 * tao_shmid shmid = tao_config_read_shmid(mirror_name);
 * tao_remote_mirror* dm = tao_remote_mirror_attach(shmid);
 * long nacts = tao_remote_mirror_get_nacts(dm);
 * size_t size = data->nacts*sizeof(double);
 * double* refs = malloc(size);
 * double* cmds = malloc(size);
 * tao_serial datnum = tao_remote_mirror_wait_output(dm, 0, 3.2);
 * if (serial > 0) {
 *     tao_dataframe_info info;
 *     tao_status status = tao_remote_mirror_fetch_data(
 *         dm, datnum, refs, cmds, data->nacts, &info);
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
 * @param obj      Pointer to remote mirror in caller's address space.
 *
 * @param datnum   The serial number of the data-frame to wait for.  If less or
 *                 equal zero, the next frame is waited for.
 *
 * @param secs     Maximum number of seconds to wait.
 *
 * @return A strictly positive number which is the serial number of the
 *         requested data-frame, `0` if the requested frame is not available
 *         before the time limit (i.e. timeout), `-1` if the requested frame is
 *         too old (it has been overwritten by some newer frames or it is
 *         beyond the last available frame), `-2` if the server has been killed
 *         and the requested frame is beyond the last available one, or `-3` in
 *         case of failure.  In the latter case, error details are reflected by
 *         the caller's last error.
 */
extern tao_serial tao_remote_mirror_wait_output(
    tao_remote_mirror* obj,
    tao_serial         datnum,
    double             secs);

/**
 * Fetch deformable mirror data-frame.
 *
 * This function shall be called to copy the contents of a deformable mirror
 * data-frame from shared memory as quickly as possible before it get
 * overwritten.
 *
 * Deformable mirror data-frames are stored in a cyclic list of buffers, so
 * their contents must be retrieved before they get overwritten.  This is
 * indicated by this function returning @ref TAO_TIMEOUT.
 *
 * The shared data shall not be locked by the caller.
 *
 * @param obj      Pointer to remote mirror in caller's address space.
 *
 * @param datnum   Serial number of the data-frame to fetch.  This value is
 *                 used to assert that the data-frame has not been overwritten
 *                 in the mean time.  Typically, this value has been obtained
 *                 by calling tao_remote_mirror_wait_output().
 *
 * @param refcmds  Buffer to store the reference commands, not used if `NULL`.
 *
 * @param perturb  Buffer to store the perturbations of the commands, not used
 *                 if `NULL`.
 *
 * @param reqcmds  Buffer to store the requested commands, not used if `NULL`.
 *
 * @param effcmds  Buffer to store the effective commands, not used if `NULL`.
 *
 * @param nvals    Number of elements to copy in the buffers `refs` and `cmds`
 *                 If any buffers is not `NULL`, `nvals` must be equal to the
 *                 number of actuators of the deformable mirror and the
 *                 non-`NULL` buffers must have at least that number of
 *                 elements.
 *
 * @param info     Pointer to retrieve the data-frame information, not used if
 *                 `NULL`.
 *
 * @return @ref TAO_OK on success, @ref TAO_TIMEOUT if the data-frame gets
 *         overwritten before its contents is copied, or @ref TAO_ERROR in case
 *         of failure.  If @ref TAO_ERROR is returned, no output buffer is
 *         modified.  If @ref TAO_TIMEOUT is returned, all output buffers are
 *         zero-filled and, if `info` is not `NULL`, `info->serial` is set to 0
 *         to indicate that the requested frame is too new (true timeout) and
 *         to -1 to indicate that the requested frame is too old (its contents
 *         has been overwritten by a newer one).  It is recommeneded to always
 *         specify a non-`NULL` valid address for `info` and check that
 *         `info->serial = serial` on return.
 */
extern tao_status tao_remote_mirror_fetch_data(
    const tao_remote_mirror* obj,
    tao_serial               num,
    double*                  refcmds,
    double*                  perturb,
    double*                  reqcmds,
    double*                  effcmds,
    long                     nvals,
    tao_dataframe_info*      info);

typedef struct tao_remote_mirror_operations tao_remote_mirror_operations;

/**
 * Table of operations for managing a deformable mirror.
 *
 * The `on_send` callback is called to send commands `vals` to the device.  On
 * entry, the values in `vals` are set to the requested commands to apply and
 * clamped in the interval `[cmin,cmax]`; on return, the callbak may modify the
 * values in `vals` to account for other constraints of the device for the
 * commands.
 */
struct tao_remote_mirror_operations {
    tao_status (*on_send)(tao_remote_mirror* obj, void* ctx, double* vals);
    const char* name;
    volatile bool debug;
};

/**
 * Run the event loop for a remote mirror server/
 *
 * @param obj      Pointer to remote mirror in caller's address space.
 *
 * @param ops      Table of callback functions.
 *
 * @param ctx      Additional context to pass to the callback functions.
 *
 * @return @ref TAO_OK on success or @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_remote_mirror_run_loop(
    tao_remote_mirror* obj,
    tao_remote_mirror_operations* ops,
    void* ctx);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_REMOTE_MIRRORS_H_
