// tao-rwlocked-objects.h -
//
// Definitions for shared objects in TAO library.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_RWLOCKED_OBJECTS_H_
#define TAO_RWLOCKED_OBJECTS_H_ 1

#include <tao-basics.h>
#include <tao-utils.h>
#include <tao-shared-memory.h>
#include <tao-shared-objects.h>

TAO_BEGIN_DECLS

/**
 * @defgroup RWLockedObjects  Read/write locked objects
 *
 * @ingroup ParallelProgramming
 *
 * @brief Basic process-shared objects with read/write access control.
 *
 * Header file @ref tao-rwlocked-objects.h provides definitions for basic
 * read/write locked objects in TAO library.  Type @ref tao_rwlocked_object is
 * the base of objects in shared memory with controlled read-only and
 * read-write access.
 *
 * @{
 */

/**
 * @brief Opaque structure for a read/write locked object.
 *
 * Read/write locked objects are shared objects (see @ref tao_shared_object)
 * implementing a read/write lock.  Such objects are shared objects whose
 * resources are controlled for read-only or read-write access.  At any time,
 * there can be any number of readers with read-only access and no writers, or
 * a single writer with read-write access and no readers.
 *
 * @see tao_rwlocked_object_.
 */
typedef struct tao_rwlocked_object tao_rwlocked_object;

/**
 * @brief Create a new read/write locked object.
 *
 * This function creates a new read/write locked object stored in shared
 * memory.  The new object is initially attached to the address space of the
 * caller, other processes may access the object by calling
 * tao_rwlocked_object_attach().  When the object is no longer used by the
 * caller, the caller is responsible of calling tao_rwlocked_object_detach() to
 * detach the object from its address space.
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
 *
 * @see tao_shared_object_create(), tao_rwlocked_object_attach().
 */
extern tao_rwlocked_object* tao_rwlocked_object_create(
    uint32_t type,
    size_t   size,
    unsigned flags);

/**
 * @brief Attach an existing read/write locked object to the address space of
 * the caller.
 *
 * This function attaches an existing read/write locked object to the address
 * space of the caller.  As a result, the number of attachments on the returned
 * object is incremented by one.  When the object is no longer used by the
 * caller, the caller is responsible of calling tao_rwlocked_object_detach() to
 * detach the object from its address space, decrement its number of
 * attachments by one and eventually free the shared memory associated with the
 * object.
 *
 * In principle, the same process may attach a read/write locked object more
 * than once but each attachment, due to tao_rwlocked_object_attach() or to
 * tao_rwlocked_object_create(), should be matched by a
 * tao_rwlocked_object_detach() with the corresponding address in the caller's
 * address space.
 *
 * @param shmid  Shared memory identifier.
 *
 * @return The address of the read/write locked object in the address space of
 *         the caller; `NULL` in case of failure.  Even tough the arguments are
 *         correct, an error may arise if the object has been destroyed before
 *         attachment completes.
 *
 * @see tao_rwlocked_object_detach().
 */
extern tao_rwlocked_object* tao_rwlocked_object_attach(
    tao_shmid shmid);

/**
 * @brief Detach a read/write locked object from the address space of the
 * caller.
 *
 * This function detaches a read/write locked object from the address space of
 * the caller and decrements the number of attachments of the read/write locked
 * object.  If the number of attachements reaches zero, the shared memory
 * segment backing the storage of the object is destroyed (unless bit @ref
 * TAO_PERSISTENT was set at object creation).
 *
 * @param obj    Pointer to a read/write locked object attached to the address
 *               space of the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_rwlocked_object_attach().
 */
extern tao_status tao_rwlocked_object_detach(
    tao_rwlocked_object* obj);

/**
 * @brief Get the size of a read/write locked object.
 *
 * This function yields the number of bytes of shared memory occupied by the
 * read/write locked object.  The size is constant for the life of the object,
 * it is thus not necessary to have locked the object to retrieve its
 * identifier.
 *
 * @param obj    Pointer to a read/write locked object attached to the address
 *               space of the caller.
 *
 * @return The number of bytes of the shared memory segment backing the storage
 *         of the read/write locked object, `0` if @a obj is `NULL`.  Whatever
 *         the result, this getter function leaves the caller's last error
 *         unchanged.
 */
extern size_t tao_rwlocked_object_get_size(
    const tao_rwlocked_object* obj);

/**
 * @brief Get the type identifier of a read/write locked object.
 *
 * This function yields the identifier of the type of the read/write locked
 * object.  The type identifier is constant for the life of the object, it is
 * thus not necessary to have locked the object to retrieve its identifier.
 *
 * @param obj    Pointer to a read/write locked object attached to the address
 *               space of the caller.
 *
 * @return The type identifier of the read/write locked object, `0` if @a obj
 *         is `NULL`.  Whatever the result, this getter function leaves the
 *         caller's last error unchanged.
 */
extern uint32_t tao_rwlocked_object_get_type(
    const tao_rwlocked_object* obj);

/**
 * @brief Get the shared memory identifier of a read/write locked object.
 *
 * This function yields the shared memory identifier of the read/write locked
 * object.  This value can be used by another process to attach to its address
 * space the read/write locked object.  The shared memory identifier is
 * constant for the life of the object, it is thus not necessary to have locked
 * the object to retrieve its identifier.
 *
 * @param obj    Pointer to a read/write locked object attached to the address
 *               space of the caller.
 *
 * @return The identifier of the read/write locked object data, `TAO_BAD_SHMID`
 *         if @a obj is `NULL`.  Whatever the result, this getter function
 *         leaves the caller's last error unchanged.
 *
 * @see tao_rwlocked_object_attach.
 */
extern tao_shmid tao_rwlocked_object_get_shmid(
    const tao_rwlocked_object* obj);

/**
 * @brief Unlock a read/write locked object.
 *
 * This function unlocks a read/write locked object that has been successfully
 * locked by the caller.
 *
 * @param obj    Pointer to a read/write locked object attached to the address
 *               space of the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlocked_object_unlock(
    tao_rwlocked_object* obj);

/**
 * @brief Lock a read/write locked object for read-only access.
 *
 * This function is similar to @ref tao_rwlock_rdlock except that it applies to
 * a read/write locked object.  The object must be attached to the address
 * space of the caller.  In case of success, the caller is responsible for
 * calling tao_rwlocked_object_unlock() to eventually release the lock.
 *
 * @warning The same thread/process must not attempt to lock the same object
 * more than once and should unlock it as soon as possible.
 *
 * @param obj    Pointer to a read/write locked object attached to the address
 *               space of the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlocked_object_rdlock(
    tao_rwlocked_object* obj);

/**
 * @brief Lock a read/write locked object for read-write access.
 *
 * This function is similar to @ref tao_rwlock_wrlock except that it applies to
 * a read/write locked object.  The object must be attached to the address space of the
 * caller.  In case of success, the caller is responsible for calling
 * tao_rwlocked_object_unlock() to eventually release the lock.
 *
 * @warning The same thread/process must not attempt to lock the same object
 * more than once and should unlock it as soon as possible.
 *
 * @param obj    Pointer to a read/write locked object attached to the address
 *               space of the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlocked_object_wrlock(
    tao_rwlocked_object* obj);

/**
 * @brief Attempt to lock a read/write locked object for read-only access
 * without blocking.
 *
 * This function attempts to lock a read/write locked object for read-only
 * access.  This function is similar to @ref tao_rwlock_try_rdlock except that
 * it applies to a read/write locked object.  The object must be attached to
 * the address space of the caller.  In case of success, the caller is
 * responsible for calling tao_rwlocked_object_unlock() to eventually release
 * the lock.
 *
 * @warning The same thread/process must not attempt to lock the same object
 * more than once and should unlock it as soon as possible.
 *
 * @param obj    Pointer to a read/write locked object attached to the address
 *               space of the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_TIMEOUT if the object cannot be
 * immediately locked; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlocked_object_try_rdlock(
    tao_rwlocked_object* obj);

/**
 * @brief Attempt to lock a read/write locked object for read-write access
 * without blocking.
 *
 * This function attempts to lock a read/write locked object for read-write
 * access.  This function is similar to @ref tao_rwlock_try_wrlock except that
 * it applies to a read/write locked object.  The object must be attached to
 * the address space of the caller.  In case of success, the caller is
 * responsible for calling tao_rwlocked_object_unlock() to eventually release
 * the lock.
 *
 * @warning The same thread/process must not attempt to lock the same object
 * more than once and should unlock it as soon as possible.
 *
 * @param obj    Pointer to a read/write locked object attached to the address
 *               space of the caller.
 *
 * @return @ref TAO_OK on success; @ref TAO_TIMEOUT if the object cannot be
 * immediately locked; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlocked_object_try_wrlock(
    tao_rwlocked_object* obj);

/**
 * @brief Attempt to lock a read/write lock for read-only access without
 * blocking more than a given duration.
 *
 * This function behaves like @ref tao_rwlocked_object_rdlock but blocks no
 * longer than a given number of seconds.
 *
 * @param obj    Pointer to a read/write locked object attached to the address
 *               space of the caller.
 *
 * @param secs   Maximum time to wait (in seconds).  If this amount of time is
 *               very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling tao_rwlocked_object_rdlock().  If
 *               this amount of time is very short, the effect is the same as
 *               calling tao_rwlocked_object_try_rdlock().
 *
 * @return @ref TAO_OK on success; @ref TAO_TIMEOUT if the lock cannot acquired
 * before the time limit; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlocked_object_timed_rdlock(
    tao_rwlocked_object* obj,
    double secs);

/**
 * @brief Attempt to lock a read/write lock for read-write access without
 * blocking more than a given duration.
 *
 * This function behaves like @ref tao_rwlocked_object_wrlock but blocks no
 * longer than a given number of seconds.
 *
 * @param obj    Pointer to a read/write locked object attached to the address
 *               space of the caller.
 *
 * @param secs   Maximum time to wait (in seconds).  If this amount of time is
 *               very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling tao_rwlocked_object_wrlock().  If
 *               this amount of time is very short, the effect is the same as
 *               calling tao_rwlocked_object_try_wrlock().
 *
 * @return @ref TAO_OK on success; @ref TAO_TIMEOUT if the lock cannot acquired
 * before the time limit; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlocked_object_timed_wrlock(
    tao_rwlocked_object* obj,
    double secs);

/**
 * @brief Attempt to lock a read/write lock for read-only access without
 * blocking longer than a time limit.
 *
 * This function behaves like @ref tao_rwlocked_object_rdlock but blocks no
 * longer than a given time limit.
 *
 * @param obj    Pointer to a read/write locked object attached to the address
 *               space of the caller.
 *
 * @param abstime  Time limit (using `CLOCK_REALTIME` clock).
 *
 * @return @ref TAO_OK on success; @ref TAO_TIMEOUT if the lock cannot acquired
 * before the time limit; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlocked_object_abstimed_rdlock(
    tao_rwlocked_object* obj,
    const tao_time* abstime);

/**
 * @brief Attempt to lock a read/write lock for read-write access without
 * blocking longer than a time limit.
 *
 * This function behaves like @ref tao_rwlocked_object_wrlock but blocks no
 * longer than a given time limit.
 *
 * @param obj    Pointer to a read/write locked object attached to the address
 *               space of the caller.
 *
 * @param abstime  Time limit (using `CLOCK_REALTIME` clock).
 *
 * @return @ref TAO_OK on success; @ref TAO_TIMEOUT if the lock cannot acquired
 * before the time limit; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlocked_object_abstimed_wrlock(
    tao_rwlocked_object* obj,
    const tao_time* abstime);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_RWLOCKED_OBJECTS_H_
