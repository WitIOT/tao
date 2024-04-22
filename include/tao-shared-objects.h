// tao-shared-objects.h -
//
// Definitions for shared objects in TAO library.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_SHARED_OBJECTS_H_
#define TAO_SHARED_OBJECTS_H_ 1

#include <tao-basics.h>
#include <tao-utils.h>
#include <tao-threads.h>
#include <tao-shared-memory.h>

TAO_BEGIN_DECLS

/**
 * @defgroup SharedObjects Shared objects
 *
 * @ingroup ParallelProgramming
 *
 * @brief Basic process-shared objects.
 *
 * Header file @ref tao-shared-objects.h provides definitions for basic
 * shared objects in TAO library.  These objects are the base of all other
 * object stored in shared memory.
 *
 * @{
 */

/**
 * Shared object structure.
 *
 * Structure @ref tao_shared_object defines the common part of all shared
 * object types and it is assumed that a pointer to a shared object of any type
 * derived from @ref tao_shared_object can be safely cast as a
 * `tao_shared_object*`.  Shared objects are stored in a single segment of
 * shared memory and have process-shared exclusive lock and condition variable.
 * Memory beyond `sizeof(tao_shared_object)` is used to store other members and
 * data (with suitable alignment) of descendant types.
 *
 * The structure defining a specific shared object type derived from
 * `tao_shared_object` should be defined by something like:
 *
 * ~~~~~{.c}
 * struct some_shared_object_type {
 *     tao_shared_object base; // Base of any TAO shared object
 *     some_type some_member;    // First member specific to the derived type
 *     some_other_type some_other_member; // etc.
 * }
 * ~~~~~
 *
 * In shared memory, the object is typically stored as follows:
 *
 * 1. Basic object members.
 * 2. Specific members.
 * 3. Padding (any amount of unused bytes required for proper
 *    alignment of the following *data* part).
 * 4. Object data.
 */
typedef struct tao_shared_object tao_shared_object;

/**
 * @def TAO_SHARED_MAGIC
 *
 * To avoid confusion, object types bitwise or'ed with @ref TAO_SHARED_MASK
 * shall be equal to @ref TAO_SHARED_MAGIC.
 */
#define TAO_SHARED_MAGIC 0x9bb04e00

// Above magic number generated with Yorick command (8-th random number):
//
//     write, format="0x%08x\n", lround(random()*(1<<24))<<8;
//

/**
 * @def TAO_SHARED_MASK
 *
 * To avoid confusion, object types bitwise or'ed with @ref TAO_SHARED_MASK
 * shall be equal to @ref TAO_SHARED_MAGIC.
 */
#define TAO_SHARED_MASK 0xffffff00

/**
 * @def TAO_SHARED_SUPERTYPE_MASK
 *
 * Mask to keep only the super-type part of a TAO object type.
 */
#define TAO_SHARED_SUPERTYPE_MASK 0xffffffe0
// FIXME: There should be an inlined function to get the super-type.  See
//        "Inline functions", p. 206 of "Modern C" by Jens Gustedt.

// * Type identifiers of shared objects.
typedef enum tao_object_type {
    // 1st generation types:
    TAO_SHARED_OBJECT   =  TAO_SHARED_MAGIC,           ///< Basic shared object.
    // 2nd generation types:
    TAO_RWLOCKED_OBJECT = (TAO_SHARED_OBJECT  |(1<<5)),///< Basic r/w locked object.
    TAO_REMOTE_OBJECT   = (TAO_SHARED_OBJECT  |(2<<5)),///< Basic remote object.
    // 3rd generation types:
    TAO_SHARED_ARRAY    = (TAO_RWLOCKED_OBJECT|     1),///< Shared multi-dimensional array.
    TAO_REMOTE_CAMERA   = (TAO_REMOTE_OBJECT  |     2),///< Remote camera.
    TAO_REMOTE_MIRROR   = (TAO_REMOTE_OBJECT  |     3),///< Remote deformable mirror.
    TAO_REMOTE_SENSOR   = (TAO_REMOTE_OBJECT  |     4),///< Remote wavefront sensor.
} tao_object_type;

/**
 * @def TAO_PERSISTENT
 *
 * The value of this macro can be combined (bitwise or'ed) with the permission
 * bits in tao_shared_object_create() to indicate that persistent shared object
 * is requested.  Otherwise, the shared object is destroyed after the last
 * detach.
 */
#define TAO_PERSISTENT  (1U << 20)

/**
 * Create a new shared object.
 *
 * This function creates a new shared object of given type and size.  The
 * object is stored in shared memory so that it can be accessible by other
 * processes (calling tao_shared_object_attach() or equivalent).  The new
 * object is initially attached to the address space of the caller.  Hence, the
 * returned object has a single attachment.  When the object is no longer used
 * by the caller, the caller is responsible of calling
 * tao_shared_object_detach() (or equivalent) to detach the object from its
 * address space and decrement its number of attachments by one.
 *
 * The remaining bytes after the basic object information are all set to zero.
 *
 * @param type   Type identifier of the object.
 *
 * @param size   Total number of bytes to allocate.
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
extern tao_shared_object* tao_shared_object_create(
    uint32_t    type,
    size_t      size,
    unsigned    flags);

/**
 * @brief Attach an existing shared object to the address space of the caller.
 *
 * This function attaches an existing shared object to the address space of the
 * caller.  As a result, the number of attachments on the returned object is
 * incremented by one.  When the object is no longer used by the caller, the
 * caller is responsible of calling tao_shared_object_detach() to detach the
 * object from its address space, decrement its number of attachments by one
 * and eventually free the shared memory associated with the object.
 *
 * In principle, the same process may attach a shared object more than once but
 * each attachment, due to tao_shared_object_attach() or to
 * tao_shared_object_create(), should be matched by a
 * tao_shared_object_detach() with the corresponding address in the caller's
 * address space.
 *
 * @param shmid  Shared memory identifier.
 *
 * @return The address of the shared object in the address space of the caller;
 *         `NULL` in case of failure.  Even tough the arguments are correct, an
 *         error may arise if the object has been destroyed before attachment
 *         completes.
 *
 * @see tao_shared_object_detach().
 */
extern tao_shared_object* tao_shared_object_attach(
    tao_shmid shmid);

/**
 * @brief Detach a shared object from the address space of the caller.
 *
 * This function detaches a shared object from the address space of the caller
 * and decrements the number of attachments of the shared object.  If the
 * number of attachements reaches zero, the shared memory segment backing the
 * storage of the object is destroyed (unless bit @ref TAO_PERSISTENT was set
 * at object creation).
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_shared_object_attach().
 */
extern tao_status tao_shared_object_detach(
    tao_shared_object* obj);

/**
 * @brief Get the size of a shared object.
 *
 * This function yields the number of bytes of shared memory occupied by the
 * shared object.  The size is constant for the life of the object, it is thus
 * not necessary to have locked the object to retrieve its identifier.
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @return The number of bytes of the shared memory segment backing the storage
 *         of the shared object, `0` if @a obj is `NULL`.  Whatever the result,
 *         this getter function leaves the caller's last error unchanged.
 */
extern size_t tao_shared_object_get_size(
    const tao_shared_object* obj);

/**
 * @brief Get the type identifier of a shared object.
 *
 * This function yields the identifier of the type of the shared object.  The
 * type identifier is constant for the life of the object, it is thus not
 * necessary to have locked the object to retrieve its identifier.
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @return The type identifier of the shared object, `0` if @a obj is `NULL`.
 *         Whatever the result, this getter function leaves the caller's last
 *         error unchanged.
 */
extern uint32_t tao_shared_object_get_type(
    const tao_shared_object* obj);

/**
 * @brief Get the shared memory identifier of a shared object.
 *
 * This function yields the shared memory identifier of the shared object.
 * This value can be used by another process to attach to its address space the
 * shared object.  The shared memory identifier is constant for the life of the
 * object, it is thus not necessary to have locked the object to retrieve its
 * identifier.
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @return The shared memory identifier of the shared object, `TAO_BAD_SHMID`
 *         if @a obj is `NULL`.  Whatever the result, this getter function
 *         leaves the caller's last error unchanged.
 *
 * @see tao_shared_object_attach.
 */
extern tao_shmid tao_shared_object_get_shmid(
    const tao_shared_object* obj);

/**
 * Lock a shared object for exclusive access.
 *
 * This function locks a shared object for exclusive (read and write) access.
 * The object must be attached to the address space of the caller.  In case of
 * success, the caller is responsible for calling tao_unlock_shared_object()
 * to eventually release the lock.
 *
 * @warning The same thread/process must not attempt to lock the same object
 * more than once and should unlock it as soon as possible.
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_shared_object_lock(
    tao_shared_object* obj);

/**
 * Unlock a shared object.
 *
 * This function unlocks a shared object that has been successfully locked by
 * the caller.
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_shared_object_unlock(
    tao_shared_object* obj);

/**
 * Attempt to immediately lock a shared object for exclusive access.
 *
 * This function attempts to lock a shared object for exclusive (read and
 * write) access without blocking.  The caller is responsible for eventually
 * releasing the lock with tao_shared_object_unlock().
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success, @ref TAO_TIMEOUT if the lock cannot be
 *         immediately acquired, or @ref TAO_ERROR on failure.
 */
extern tao_status tao_shared_object_try_lock(
    tao_shared_object* obj);

/**
 * Attempt to lock a shared object for exclusive access with an absolute time
 * limit.
 *
 * This function attempts to lock a shared object for exclusive (read and
 * write) access without blocking beyond a given time limit.  The caller is
 * responsible for eventually releasing the lock with
 * tao_shared_object_unlock().
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @param lim    Absolute time limit.
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_shared_object_abstimed_lock(
    tao_shared_object* obj,
    const tao_time* lim);

/**
 * Attempt to lock a shared object for exclusive access with a relative time
 * limit.
 *
 * This function attempts to lock a shared object for exclusive (read and
 * write) access without blocking more than a given duration.  The caller is
 * responsible for eventually releasing the lock with
 * tao_shared_object_unlock().
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @param secs   Maximum time to wait (in seconds).  If this amount of time is
 *               very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling tao_shared_object_lock().  If
 *               this amount of time is very short, the effect is the same as
 *               calling tao_shared_object_try_lock().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred
 *         before or @ref TAO_ERROR in case of error.
 */
extern tao_status tao_shared_object_timed_lock(
    tao_shared_object* obj,
    double secs);

/**
 * Signal a condition variable to at most one thread waiting on a shared object.
 *
 * This function restarts one of the threads that are waiting on the condition
 * variable of the object.  Nothing happens, if no threads are waiting on the
 * condition variable.
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 *
 * @see tao_shared_object_broadcast_condition(),
 *      tao_shared_object_wait_condition().
 */
extern tao_status tao_shared_object_signal_condition(
    tao_shared_object* obj);

/**
 * Signal a condition to all threads waiting on a shared object.
 *
 * This function behaves like tao_shared_object_signal_condition() except that
 * all threads waiting on the condition variable of the object are restarted.
 * Nothing happens, if no threads are waiting on the condition variable.
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 *
 * @see tao_shared_object_signal_condition(),
 *      tao_shared_object_wait_condition().
 */
extern tao_status tao_shared_object_broadcast_condition(
    tao_shared_object* obj);

/**
 * Wait for a condition to be signaled for a shared object.
 *
 * This function atomically unlocks the exclusive lock associated with the
 * shared object and waits for its associated condition variable to be
 * signaled.  The thread execution is suspended and does not consume any CPU
 * time until the condition variable is signaled.  The mutex of the object must
 * have been locked (e.g., with tao_shared_object_lock()) by the calling thread
 * on entrance to this function.  Before returning to the calling thread, this
 * function re-acquires the mutex.
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 *
 * @see tao_shared_object_lock(),
 *      tao_shared_object_signal_condition().
 */
extern tao_status tao_shared_object_wait_condition(
    tao_shared_object* obj);

/**
 * Wait for a condition to be signaled for a shared object without blocking
 * longer than an absolute time limit.
 *
 * This function behaves like tao_shared_object_wait_condition() but blocks no
 * longer than a given duration.
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @param lim    Absolute time limit with the same conventions as
 *               tao_get_current_time().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_shared_object_abstimed_wait_condition(
    tao_shared_object* obj,
    const tao_time* lim);

/**
 * Wait for a condition to be signaled for a shared object without blocking
 * longer than a relative time limit.
 *
 * This function behaves like tao_shared_object_wait_condition() but blocks no
 * longer than a given duration.
 *
 * @param obj    Pointer to a shared object attached to the address space of
 *               the caller.
 *
 * @param secs   Maximum amount of time (in seconds).  If this amount of time
 *               is very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling
 *               tao_shared_object_wait_condition().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_shared_object_timed_wait_condition(
    tao_shared_object* obj,
    double secs);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_SHARED_OBJECTS_H_
