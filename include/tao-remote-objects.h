// tao-remote-objects.h -
//
// Definitions for remote objects in TAO library.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_REMOTE_OBJECTS_H_
#define TAO_REMOTE_OBJECTS_H_ 1

#include <tao-threads.h>
#include <tao-shared-memory.h>

TAO_BEGIN_DECLS

/**
 * @defgroup RemoteObjects  Remote objects
 *
 * @ingroup ParallelProgramming
 *
 * @brief Basic process-shared objects to communicate with servers.
 *
 * Header file @ref tao-remote-objects.h provides definitions for basic remote
 * objects in TAO library.  Remote objects are shared objects (@ref
 * tao_shared_object) which are used for communication with a server running on
 * the same machine.  These objects are the base of all other remote object
 * types.
 *
 * @{
 */

/**
 * Enumeration of possible commands for remote objects.
 */
typedef enum tao_command {
    TAO_COMMAND_NONE   = 0,///< No pending command.
    TAO_COMMAND_RESET  = 1,///< Reset correction or configuration.
    TAO_COMMAND_SEND   = 2,///< Send actuators commands.
    TAO_COMMAND_CONFIG = 3,///< Change configuration.
    TAO_COMMAND_START  = 4,///< Start work.
    TAO_COMMAND_STOP   = 5,///< Stop work.
    TAO_COMMAND_ABORT  = 6,///< Abort work.
    TAO_COMMAND_KILL   = 7,///< Require remote server to quit.
} tao_command;

/**
 * Yield the literal name of a server command.
 *
 * @param cmd   Server command value.
 *
 * @return The literal name of the server command or `"unknown"`.
 */
extern const char* tao_command_get_name(
    tao_command cmd);

/**
 * Enumeration of possible state for remote servers.
 *
 * Checking that a server is running and ready to accept commands
 * amounts to checking that its state is strictly positive.
 */
typedef enum tao_state {
    // The values must be in a logical ascending order.
    TAO_STATE_INITIALIZING =  0,///< Server is not yet ready.
    TAO_STATE_WAITING      =  1,///< Server is waiting for commands.
    TAO_STATE_CONFIGURING  =  2,///< Server is configuring the settings.
    TAO_STATE_STARTING     =  3,///< Server is starting its work.
    TAO_STATE_WORKING      =  4,///< Server is working.
    TAO_STATE_STOPPING     =  5,///< Server is stopping its work.
    TAO_STATE_ABORTING     =  6,///< Server is aborting its work.
    TAO_STATE_ERROR        =  7,///< Server is in recoverable error state.
    TAO_STATE_RESETTING    =  8,///< Server is attempting a reset.
    TAO_STATE_QUITTING     =  9,///< Server is about to quit.
    TAO_STATE_UNREACHABLE  = 10,///< Server is unreachable.
} tao_state;

/**
 * Yield the literal name of a server state.
 *
 * @param state   Server state value.
 *
 * @return The literal name of the server state or `"unknown"`.
 */
extern const char* tao_state_get_name(
    tao_state state);

/**
 * @def TAO_OWNER_SIZE
 *
 * The number of bytes (including the final null) for the name of the owner of
 * a shared object.
 */
#define TAO_OWNER_SIZE 64

/**
 * @brief Opaque structure for a remote shared object.
 *
 * Remote shared objects are shared objects (see @ref tao_shared_object) used
 * for communication between a server and its clients.  They provide command
 * queue and a cyclic list of shared output buffers.
 *
 * @see tao_remote_object_.
 */
typedef struct tao_remote_object tao_remote_object;

/**
 * @typedef tao_dataframe_info
 *
 * @brief Data-frame descriptor as retrieved by clients.
 *
 * This structure is used to retrieve informations about a given data-frame
 * from a remote object.  The only difference with @ref tao_dataframe_header is
 * that its members have no specific (like `atomic` or `volatile`) storage
 * qualifiers.
 */
typedef struct tao_dataframe_info {
    tao_serial serial; ///< Serial number.
    tao_serial   mark; ///< User-defined mark.
    tao_time     time; ///< Time-stamp.
} tao_dataframe_info;

/**
 * @brief Create a new remote object.
 *
 * This function creates a new remote object.  This function is similar to
 * tao_shared_object_create().
 *
 * A remote object is a shared object intended to implement the communication
 * between a server and its clients.  The remote object has a cyclic list of
 * output buffers.  The contents of the `i`-th output buffer (with `i ≥ 1`) is
 * assumed to be stored in the object itself at offset (in bytes):
 *
 * ~~~~~{.c}
 * offset + ((i - 1)%nbufs)*stride
 * ~~~~~
 *
 * hence argument `stride` shall be equal to the size of an output buffer
 * rounded-up to a multiple of a suitable number of bytes (the cache line size
 * for instance).
 *
 * @param owner  Short string identifying the creator of the shared resource.
 *               Cannot be `NULL` nor an empty string and maximal size
 *               including the final null character is @ref TAO_OWNER_SIZE.
 *
 * @param type   Type identifier of the object.
 *
 * @param nbufs  Number of output buffers.
 *
 * @param offset Offset (in bytes) to the first output buffer.  Must be at least
 *               `sizeof(tao_remote_object)`.
 *
 * @param sride  Rounded-up size of a single output buffer.
 *
 * @param size   Total number of bytes to allocate.  Must be at least
 *               `offset + nbufs*stride`.
 *
 * @param flags  Permissions granted to the group and to the others.  At least,
 *               read and write access (that is bits `S_IRUSR` and `S_IWUSR`)
 *               are granted for the caller.  Unless bit @ref TAO_PERSISTENT is
 *               set in `flags`, the shared memory backing the storage of the
 *               shared data will be destroyed upon last detach.
 *
 * @return The address of the new object in the address space of the caller;
 *         `NULL` in case of failure.
 */
extern tao_remote_object* tao_remote_object_create(
    const char* owner,
    uint32_t    type,
    long        nbufs,
    long        offset,
    long        stride,
    size_t      size,
    unsigned    flags);

/**
 * @brief Attach an existing remote object to the address space of the caller.
 *
 * This function attaches an existing remote object to the address space of the
 * caller.  As a result, the number of attachments on the returned object is
 * incremented by one.  When the object is no longer used by the caller, the
 * caller is responsible of calling tao_remote_object_detach() to detach the
 * object from its address space, decrement its number of attachments by one
 * and eventually free the shared memory associated with the object.
 *
 * In principle, the same process may attach a remote object more than once but
 * each attachment, due to tao_remote_object_attach() or to
 * tao_remote_object_create(), should be matched by a
 * tao_remote_object_detach() with the corresponding address in the caller's
 * address space.
 *
 * @param shmid  Shared memory identifier.
 *
 * @return The address of the remote object in the address space of the caller;
 *         `NULL` in case of failure.  Even tough the arguments are correct, an
 *         error may arise if the object has been destroyed before attachment
 *         completes.
 *
 * @see tao_remote_object_detach().
 */
extern tao_remote_object* tao_remote_object_attach(
    tao_shmid shmid);

/**
 * @brief Detach a remote object from the address space of the caller.
 *
 * This function detaches a remote object from the address space of the caller
 * and decrements the number of attachments of the remote object.  If the
 * number of attachements reaches zero, the shared memory segment backing the
 * storage of the object is destroyed (unless bit @ref TAO_PERSISTENT was set
 * at object creation).
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_object_attach().
 */
extern tao_status tao_remote_object_detach(
    tao_remote_object* obj);

/**
 * @brief Get the size of a remote object.
 *
 * This function yields the number of bytes of shared memory occupied by the
 * remote object.  The size is constant for the life of the object, it is thus
 * not necessary to have locked the object to retrieve its identifier.
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @return The number of bytes of the shared memory segment backing the storage
 *         of the remote object, `0` if @a obj is `NULL`.  Whatever the result,
 *         this getter function leaves the caller's last error unchanged.
 */
extern size_t tao_remote_object_get_size(
    const tao_remote_object* obj);

/**
 * @brief Get the type identifier of a remote object.
 *
 * This function yields the identifier of the type of the remote object.  The
 * type identifier is constant for the life of the object, it is thus not
 * necessary to have locked the object to retrieve its identifier.
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @return The type identifier of the remote object, `0` if @a obj is `NULL`.
 *         Whatever the result, this getter function leaves the caller's last
 *         error unchanged.
 */
extern uint32_t tao_remote_object_get_type(
    const tao_remote_object* obj);

/**
 * @brief Get the shared memory identifier of a remote object.
 *
 * This function yields the shared memory identifier of the remote object.
 * This value can be used by another process to attach to its address space the
 * remote object.  The shared memory identifier is constant for the life of the
 * object, it is thus not necessary to have locked the object to retrieve its
 * identifier.
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @return The identifier of the remote object data, `TAO_BAD_SHMID` if @a obj
 *         is `NULL`.  Whatever the result, this getter function leaves the
 *         caller's last error unchanged.
 *
 * @see tao_remote_object_attach.
 */
extern tao_shmid tao_remote_object_get_shmid(
    const tao_remote_object* obj);

/**
 * Lock a remote object for exclusive access.
 *
 * This function locks a remote object for exclusive (read and write) access.
 * The object must be attached to the address space of the caller.  In case of
 * success, the caller is responsible for calling tao_unlock_remote_object()
 * to eventually release the lock.
 *
 * @warning The same thread/process must not attempt to lock the same object
 * more than once and should unlock it as soon as possible.
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_remote_object_lock(
    tao_remote_object* obj);

/**
 * Unlock a remote object.
 *
 * This function unlocks a remote object that has been successfully locked by
 * the caller.
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_remote_object_unlock(
    tao_remote_object* obj);

/**
 * Attempt to immediately lock a remote object for exclusive access.
 *
 * This function attempts to lock a remote object for exclusive (read and
 * write) access without blocking.  The caller is responsible for eventually
 * releasing the lock with tao_remote_object_unlock().
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success, @ref TAO_TIMEOUT if the lock cannot be
 *         immediately acquired, or @ref TAO_ERROR on failure.
 */
extern tao_status tao_remote_object_try_lock(
    tao_remote_object* obj);

/**
 * Attempt to lock a remote object for exclusive access with an absolute time
 * limit.
 *
 * This function attempts to lock a remote object for exclusive (read and
 * write) access without blocking beyond a given time limit.  The caller is
 * responsible for eventually releasing the lock with
 * tao_remote_object_unlock().
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @param lim    Absolute time limit.
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_object_abstimed_lock(
    tao_remote_object* obj,
    const tao_time* lim);

/**
 * Attempt to lock a remote object for exclusive access with a relative time
 * limit.
 *
 * This function attempts to lock a remote object for exclusive (read and
 * write) access without blocking more than a given duration.  The caller is
 * responsible for eventually releasing the lock with
 * tao_remote_object_unlock().
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @param secs   Maximum time to wait (in seconds).  If this amount of time is
 *               very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling tao_remote_object_lock().  If
 *               this amount of time is very short, the effect is the same as
 *               calling tao_remote_object_try_lock().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_object_timed_lock(
    tao_remote_object* obj,
    double secs);

/**
 * Signal a condition variable to at most one thread waiting on a remote object.
 *
 * This function restarts one of the threads that are waiting on the condition
 * variable of the object.  Nothing happens, if no threads are waiting on the
 * condition variable.
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_object_broadcast_condition(),
 *      tao_remote_object_wait_condition().
 */
extern tao_status tao_remote_object_signal_condition(
    tao_remote_object* obj);

/**
 * Signal a condition to all threads waiting on a remote object.
 *
 * This function behaves like tao_remote_object_signal_condition() except that
 * all threads waiting on the condition variable of the object are restarted.
 * Nothing happens, if no threads are waiting on the condition variable.
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_object_signal_condition(),
 *      tao_remote_object_wait_condition().
 */
extern tao_status tao_remote_object_broadcast_condition(
    tao_remote_object* obj);

/**
 * Wait for a condition to be signaled for a remote object.
 *
 * This function atomically unlocks the exclusive lock associated with the
 * remote object and waits for its associated condition variable to be
 * signaled.  The thread execution is suspended and does not consume any CPU
 * time until the condition variable is signaled.  The mutex of the object must
 * have been locked (e.g., with tao_remote_object_lock()) by the calling thread
 * on entrance to this function.  Before returning to the calling thread, this
 * function re-acquires the mutex.
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 *
 * @see tao_remote_object_lock(),
 *      tao_remote_object_signal_condition().
 */
extern tao_status tao_remote_object_wait_condition(
    tao_remote_object* obj);

/**
 * Wait for a condition to be signaled for a remote object without blocking
 * longer than an absolute time limit.
 *
 * This function behaves like tao_remote_object_wait_condition() but blocks no
 * longer than a given duration.
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @param lim    Absolute time limit with the same conventions as
 *               tao_get_current_time().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_object_abstimed_wait_condition(
    tao_remote_object* obj,
    const tao_time* lim);

/**
 * Wait for a condition to be signaled for a remote object without blocking
 * longer than a relative time limit.
 *
 * This function behaves like tao_remote_object_wait_condition() but blocks no
 * longer than a given duration.
 *
 * @param obj    Pointer to a remote object attached to the address space of
 *               the caller.
 *
 * @param secs   Maximum amount of time (in seconds).  If this amount of time
 *               is very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling
 *               tao_remote_object_wait_condition().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_remote_object_timed_wait_condition(
    tao_remote_object* obj,
    double secs);

/**
 * Get the name of the owner of a remote object.
 *
 * This function yields the name of the owner of the remote object.  This
 * information is immutable and the object needs not be locked by the caller.
 *
 * @param obj     Pointer to a remote object attached to the address space of
 *                the caller.
 *
 * @return The name of the remote object owner or an empty string `""` for a
 *         `NULL` object pointer.  Whatever the result, this getter function
 *         leaves the caller's last error unchanged.
 */
extern const char* tao_remote_object_get_owner(
    const tao_remote_object* obj);

/**
 * Get the number of output buffers of a remote object.
 *
 * This function yields the number of entries in the cyclic list of output
 * buffers allocated by the remote object.  This information is immutable and
 * the object needs not be locked by the caller.
 *
 * @param obj     Pointer to a remote object attached to the address space of
 *                the caller.
 *
 * @return The number of buffers or `0` for a `NULL` object pointer.  Whatever
 *         the result, this getter function leaves the caller's last error
 *         unchanged.
 */
extern long tao_remote_object_get_nbufs(
    const tao_remote_object* obj);

/**
 * Get the serial number of the last output buffer of a remote object.
 *
 * This function yields the serial number of the last output buffer available
 * from a remote object.  This is also the number of output buffers posted so
 * far by the owner of the remote object.
 *
 * The serial number is stored in an *atomic* variable, so the caller needs not
 * lock the remote object.
 *
 * @param obj     Pointer to a remote object attached to the address space of
 *                the caller.
 *
 * @return A nonnegative integer.  The serial number of the last available
 *         output buffer of the remote object, `0` if no output buffer are
 *         available yet or if `obj` is `NULL`.  Whatever the result, this
 *         getter function leaves the caller's last error unchanged.
 */
extern tao_serial tao_remote_object_get_serial(
    const tao_remote_object* obj);

/**
 * Get the number of commands processed by the server owning a remote object.
 *
 * This function yields the the number of commands processed so far by the
 * owner of the remote object.
 *
 * The number of processed commands is stored in an *atomic* variable, so the
 * caller needs not lock the remote camera.
 *
 * @param obj     Pointer to a remote object attached to the address space of
 *                the caller.
 *
 * @return The number processed commands, a nonnegative integer which may be
 *         `0` if no commands have been ever processed or if `obj` is `NULL`.
 *         Whatever the result, this getter function leaves the caller's last
 *         error unchanged.
 */
extern tao_serial tao_remote_object_get_ncmds(
    const tao_remote_object* obj);

/**
 * Get the current state of the server owning a remote object.
 *
 * This function yields the current state of the server owning the remote
 * object.
 *
 * The server state is stored in an *atomic* variable, so the caller needs not
 * lock the remote object.
 *
 * @param obj     Pointer to a remote object attached to the address space of
 *                the caller.
 *
 * @return The state of the remote server, @ref TAO_STATE_UNREACHABLE if `obj` is
 *         `NULL`.  Whatever the result, this getter function leaves the
 *         caller's last error unchanged.
 */
extern tao_state tao_remote_object_get_state(
    const tao_remote_object* obj);

/**
 * Check whether the server owning a remote object is alive.
 *
 * This function uses the current state of the server owning the remote object
 * to determine whether the server is alive.
 *
 * The server state is stored in an *atomic* variable, so the caller needs not
 * lock the remote object.
 *
 * @param obj     Pointer to a remote object attached to the address space of
 *                the caller.
 *
 * @return A boolean result; `false` if `obj` is `NULL`.  Whatever the result,
 *         this getter function leaves the caller's last error unchanged.
 */
extern int tao_remote_object_is_alive(
    const tao_remote_object* obj);

/**
 * Wait for a given output buffer to be available.
 *
 * This function waits for a specific output buffer to be available from a
 * remote server.  Output buffers are stored by remote servers in a cyclic list
 * which provide an history of limited length.  As a consequence, the contents
 * of retrieved output buffers should be copied as soon as possible to avoid
 * that the buffer be overwritten.
 *
 * @warning The caller must not have locked the object.
 *
 * @param obj     Pointer to a remote object attached to the address space of
 *                the caller.
 *
 * @param num     The serial number of the output buffer to wait for.  If less
 *                or equal zero, the next output buffer is waited for.
 *
 * @param secs    Maximum amount of time to wait (in seconds).
 *
 * @return A strictly positive number which is the serial number of the
 *         requested buffer, `0` if the requested buffer is not available
 *         before the time limit (i.e. timeout), `-1` if the requested buffer
 *         is too old (it has been overwritten or it is beyond the last
 *         available buffer), `-2` if the server has been killed and the
 *         requested buffer is beyond the last available one, or `-3` in case
 *         of failure.  In the latter case, error details are reflected by the
 *         caller's last error.
 */
extern tao_serial tao_remote_object_wait_output(
    tao_remote_object* obj,
    tao_serial         num,
    double             secs);

/**
 * Wait for a given command to have been processed.
 *
 * This function waits for a specific command sent to the server owning a
 * remote object to have been processed.
 *
 * @warning The caller must not have locked the object.
 *
 * @param obj     Pointer to a remote object attached to the address space of
 *                the caller.
 *
 * @param num     The serial number of the command to wait for.
 *
 * @param secs    Maximum amount of time to wait (in seconds).
 *
 * @return @ref TAO_OK on success, @ref TAO_TIMEOUT if the command has not been
 *         processed before the time limit, and @ref TAO_ERROR on failure.
 */
extern tao_status tao_remote_object_wait_command(
    tao_remote_object* obj,
    tao_serial         num,
    double             secs);

/**
 * Send a simple command to a remote server.
 *
 * This function attempts to send a simple command (i.e., that requires no
 * other arguments) to the remote server owning a shared object not waiting
 * longer than a given amount of time.
 *
 * @note Functions tao_remote_object_lock_for_command() and, eventually,
 *       tao_remote_object_unlock() can be used to implement more complex
 *       commands.
 *
 * @warning The caller must not have locked the object.
 *
 * @param obj     Pointer to a remote object attached to the address space of
 *                the caller.
 *
 * @param cmd     Command to send.
 *
 * @param secs    Maximum amount of time to wait (in seconds).
 *
 * @return The serial number of the command, 0 if the command cannot be sent
 *         before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_object_send_simple_command(
    tao_remote_object* obj,
    tao_command cmd,
    double secs);

/**
 * Send a kill command to a remote server.
 *
 * This function attempts to send a "*kill*" command to the remote server
 * owning a shared object not waiting longer than a given amount of time.
 *
 * @warning The object must not be locked.
 *
 * @param obj     Pointer to a remote object attached to the address space of
 *                the caller.
 *
 * @param secs    Maximum amount of time to wait (in seconds).
 *
 * @return The serial number of the command, 0 if the command cannot be sent
 *         before the time limit, -1 in case of error.
 */
extern tao_serial tao_remote_object_kill(
    tao_remote_object* obj,
    double secs);

/**
 * Wait for a remote server to be ready for a new command.
 *
 * This function attempts to lock a remote object and to wait for its server to
 * accept commands or to be killed.  Upon success, the next command to execute
 * is set to be @b cmd and, unless the command is @ref TAO_COMMAND_NONE, the
 * server is notified that a new command is pending.  The command will be
 * executed just after the object is unlocked.  This behavior is intended for
 * commands that require other arguments than just the the command itself.
 *
 * @warning The object must be initially unlocked and it is the caller's
 *          responsibility to eventually unlock the object if a strictly
 *          positive number is returned by this function.
 *
 * @note This is a low level function provided to implement sending of commands
 *       for various remote object types.
 *
 * @param obj     Pointer to a remote object attached to the address space of
 *                the caller.
 *
 * @param cmd     Command to send.
 *
 * @param secs    Maximum amount of time to wait (in seconds).
 *
 * @return The serial number of the command (a strictly positive integer), 0 if
 *         the command cannot be sent before the time limit, -1 in case of
 *         error.
 */
extern tao_serial tao_remote_object_lock_for_command(
    tao_remote_object* obj,
    tao_command cmd,
    double secs);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_REMOTE_OBJECTS_H_
