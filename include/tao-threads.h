// tao-threads.h -
//
// Definitions for mutexes, condition variables, read/write locks, threads, and
// semaphores in TAO library.  Except for semaphores, functions defined in this
// file are mostly simple wrappers around POSIX Thread functions to homogenize
// handling of errors.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_THREADS_H_
#define TAO_THREADS_H_ 1

#include <tao-basics.h>
#include <tao-utils.h>

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

TAO_BEGIN_DECLS

/**
 * @defgroup Mutexes  Mutexes
 *
 * @ingroup ParallelProgramming
 *
 * @brief Exclusive locks that may be shared by processes.
 *
 * @{
 */

/**
 * Exclusive lock.
 *
 * This alias is to homogenize code style and simplify future changes.
 */
typedef pthread_mutex_t tao_mutex;

/**
 * Default initializer for a static exclusive lock (mutex).
 */
#define TAO_MUTEX_INITIALIZER  ((tao_mutex)PTHREAD_MUTEX_INITIALIZER)

/**
 * Indicator of a process-private or process-shared resource.
 *
 * The constant `TAO_PROCESS_PRIVATE` indicates that a given resource (lock,
 * condition variable, or semaphore) shall be initialized so as to be private
 * to the threads of a given process.
 *
 * The constant `TAO_PROCESS_SHARED` indicates that a given resource (lock,
 * condition variable, or semaphore) shall be initialized so as to be sharable
 * by other process.  Note that a process-shared lock or condition variable
 * must be stored in shared memory (@see SharedMemory).
 */
typedef enum tao_process_sharing {
    TAO_PROCESS_PRIVATE,
    TAO_PROCESS_SHARED
} tao_process_sharing;

/**
 * Initialize a non-static mutex.
 *
 * This functions initialize a mutex prior to its usage.  The function
 * tao_mutex_destroy() must be called when the mutex is no longer needed.
 *
 * @param mutex  Pointer to the mutex to initialize.
 *
 * @param share  If set to @ref TAO_PROCESS_SHARED, require that the mutex be
 *               accessible between processes; otherwise, must be @ref
 *               TAO_PROCESS_PRIVATE and the mutex will be *private* (that is,
 *               only accessible by threads in the same process as the caller).
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_mutex_initialize(
    tao_mutex* mutex,
    tao_process_sharing share);

/**
 * Lock a mutex.
 *
 * @param mutex  Pointer to the mutex to lock.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_mutex_lock(
    tao_mutex* mutex);

/**
 * Attempt to lock a mutex.
 *
 * @param mutex  Pointer to the mutex to lock.
 *
 * @return @ref TAO_OK if mutex has been locked by the caller, @ref TAO_TIMEOUT
 *         if the mutex is already locked by some other thread/process; @ref
 *         TAO_ERROR in case of failure.
 */
extern tao_status tao_mutex_try_lock(
    tao_mutex* mutex);

/**
 * Attempt to lock a mutex without blocking longer than an absolute time limit.
 *
 * This function behaves like tao_mutex_lock() but blocks no longer than a
 * given absolute time limit.
 *
 * @param mutex    Pointer to the mutex to lock.
 *
 * @param abstime  Absolute time limit for waiting.
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit; @ref TAO_TIMEOUT if timeout occurred before;
 *         @ref TAO_ERROR in case of failure.
 *
 * @see tao_mutex_lock(), tao_mutex_try_lock(), tao_mutex_timed_lock(),
 *      tao_status.
 */
extern tao_status tao_mutex_abstimed_lock(
    tao_mutex* mutex,
    const tao_time* abstime);

/**
 * Attempt to lock a mutex without blocking longer than a relative time limit.
 *
 * This function behaves like tao_mutex_lock() but blocks no longer than a
 * given number of seconds from now.
 *
 * @param mutex  Pointer to the mutex to lock.
 *
 * @param secs   Maximum time to wait (in seconds).  If this amount of time is
 *               very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling tao_mutex_lock().  If this
 *               amount of time is very short, the effect is the same as
 *               calling tao_mutex_try_lock().
 *
 * @return @ref TAO_OK if the read/write lock has been locked for reading by
 *         the caller before the specified time limit; @ref TAO_TIMEOUT if
 *         timeout occurred before; @ref TAO_ERROR in case of failure.
 *
 * @see tao_mutex_lock(), tao_mutex_try_lock(), tao_get_absolute_timeout(),
 *      tao_mutex_abstimed_lock(), tao_status.
 */
extern tao_status tao_mutex_timed_lock(
    tao_mutex* mutex,
    double secs);

/**
 * Unlock a mutex.
 *
 * @param mutex  Pointer to the mutex to unlock.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_mutex_unlock(
    tao_mutex* mutex);

/**
 * Destroy a mutex.
 *
 * This function destroys a mutex that has been initialized by
 * tao_mutex_initialize().
 *
 * @param mutex  Pointer to the mutex to destroy.
 *
 * @param wait   If the mutex is locked, this parameter specifies whether the
 *               function should block until the mutex is unlocked by its
 *               owner.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_mutex_destroy(
    tao_mutex* mutex,
    bool wait);

/**
 * @}
 */

/**
 * @defgroup ConditionVariables Condition variables
 *
 * @ingroup ParallelProgramming
 *
 * @brief Condition variables that may be shared by processes.
 *
 * Condition variables are used in association with mutexes to notify
 * conditions to other thread(s) or processe(s).
 *
 * @see Mutexes
 *
 * @{
 */

/**
 * Condition variable.
 *
 * This alias is to homogenize code style and simplify future changes.
 */
typedef pthread_cond_t tao_cond;

/**
 * Default initializer for a static condition variable.
 */
#define TAO_COND_INITIALIZER   ((tao_cond)PTHREAD_COND_INITIALIZER)

/**
 * Initialize a condition variable.
 *
 * This function initializes a non-static condition variable.  The caller is
 * responsible of calling tao_condition_destroy() to free the resources that
 * may be associated with the condition variable.
 *
 * @param cond   Pointer to the condition variable to initialize.
 *
 * @param share  If set to @ref TAO_PROCESS_SHARED, require that the condition
 *               variable be accessible between processes; otherwise, must be
 *               @ref TAO_PROCESS_PRIVATE and the condition variable will be
 *               *private* (that is, only accessible by threads in the same
 *               process as the caller).
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_condition_initialize(
    tao_cond* cond,
    tao_process_sharing share);

/**
 * Destroy a condition variable.
 *
 * This function destroys a condition variable that has been initialized by
 * tao_condition_initialize().
 *
 * @param cond   Pointer to the condition variable to destroy.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_condition_destroy(
    tao_cond* cond);

/**
 * Signal a condition variable to at most one thread.
 *
 * This function restarts one of the threads that are waiting on the condition
 * variable @b cond.  Nothing happens, if no threads are waiting on the
 * condition variable.
 *
 * @param cond   Pointer to the condition variable to signal.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_condition_signal(
    tao_cond* cond);

/**
 * Signal a condition variable to all waiting threads.
 *
 * This function behaves like tao_condition_signal() except that all threads
 * waiting on the condition variable @a cond are restarted.  Nothing happens,
 * if no threads are waiting on the condition variable.
 *
 * @param cond   Pointer to the condition variable to signal.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_condition_broadcast(
    tao_cond* cond);

/**
 * Wait for a condition to be signaled.
 *
 * This function atomically unlocks the mutex and waits for the condition
 * variable to be signaled.  The thread execution is suspended and does not
 * consume any CPU time until the condition variable is signaled. The mutex
 * must be locked by the calling thread on entrance to this function.  Before
 * returning to the calling thread, this function re-acquires the mutex.
 *
 * @param cond   Address of the condition variable to wait on.
 *
 * @param mutex  Address of the mutex associated with the condition variable.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 *
 * @see tao_condition_abstimed_wait(), tao_condition_timed_wait().
 */
extern tao_status tao_condition_wait(
    tao_cond* cond,
    tao_mutex* mutex);

/**
 * Wait for a condition to be signaled without blocking longer than an absolute
 * time limit.
 *
 * This function behaves like tao_condition_wait() but blocks no longer than
 * a given absolute time limit.
 *
 * @param cond   Address of the condition variable to wait on.
 *
 * @param mutex  Address of the mutex associated with the condition variable.
 *
 * @param lim    Absolute time limit with the same conventions as
 *               tao_get_current_time().
 *
 * @return @ref TAO_OK if the condition is signaled before the specified number
 *         of seconds; @ref TAO_TIMEOUT if timeout occurred before; @ref
 *         TAO_ERROR in case of failure.
 *
 * @see tao_condition_wait(), tao_condition_timed_wait(),
 *      tao_status.
 */
extern tao_status tao_condition_abstimed_wait(
    tao_cond* cond,
    tao_mutex* mutex,
    const tao_time* lim);

/**
 * Wait for a condition to be signaled without blocking longer than a relative
 * time limit.
 *
 * This function behaves like tao_condition_wait() but blocks no longer than
 * some given duration.
 *
 * @param cond   Address of the condition variable to wait on.
 *
 * @param mutex  Address of the mutex associated with the condition variable.
 *
 * @param secs   Maximum amount of time (in seconds).  If this amount of time
 *               is very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling tao_condition_wait().
 *
 * @return @ref TAO_OK if the condition is signaled before the specified number
 *         of seconds; @ref TAO_TIMEOUT if timeout occurred before; @ref
 *         TAO_ERROR in case of failure.
 *
 * @see tao_condition_wait(), tao_get_absolute_timeout(),
 *      tao_condition_abstimed_wait(), tao_status.
 */
extern tao_status tao_condition_timed_wait(
    tao_cond* cond,
    tao_mutex* mutex,
    double secs);

/**
 * @}
 */

/**
 * @defgroup ReadWriteLocks Read/write locks
 *
 * @ingroup ParallelProgramming
 *
 * @brief Read/write locks that may be shared by processes.
 *
 * Read/write locks are used to protect shared resources between threads or
 * processes.  A *writer* is a thread or a process which owns the read/write
 * lock for read-write access and a *reader* is a thread or a process which
 * owns the read/write lock for read-only access.  It is assumed that only a
 * *writer* can modify (write) the shared resources and that the resources
 * cannot be modified while one or more *readers* own the read/write lock for
 * reading.  At any given time, there can be either (i) no owners of the lock,
 * (ii) one writer owning the lock and no readers or (iii) no writers and one
 * of more readers owning the lock.
 *
 * TAO library provides helper functions for handling POSIX Threads read/write
 * locks with reporting of errors and shared data objects that are stored in
 * shared memory and have their own shared read/write lock.
 *
 * @{
 */

/**
 * Read/write lock.
 *
 * This alias is to homogenize code style and simplify future changes.
 *
 * The read/write locks implemented by TAO shared objects is a custom
 * implementation to serve different purposes.
 */
typedef pthread_rwlock_t tao_rwlock;

/**
 * Default initializer for a static read/write lock.
 */
#define TAO_RWLOCK_INITIALIZER  ((tao_rwlock)PTHREAD_RWLOCK_INITIALIZER)

/**
 * Initialize a read/write lock.
 *
 * This function initializes a non-static read/write lock.  The caller is
 * responsible of calling tao_rwlock_destroy() to free the resources that may
 * be associated with the read/write lock.
 *
 * @param lock   Pointer to the read/write lock to initialize.
 *
 * @param share  If set to @ref TAO_PROCESS_SHARED, require that the read/write
 *               lock be accessible between processes; otherwise, must be @ref
 *               TAO_PROCESS_PRIVATE and the read/write lock will be *private*
 *               (that is, only accessible by threads in the same process as
 *               the caller).
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlock_initialize(
    tao_rwlock* lock,
    tao_process_sharing share);

/**
 * Destroy a read/write lock.
 *
 * This function destroys a read/write lock that has been initialized by
 * tao_rwlock_initialize().
 *
 * @param lock   Pointer to the read/write lock to destroy.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlock_destroy(
    tao_rwlock* lock);

/**
 * @brief Lock a read/write lock for read-only access.
 *
 * This function locks a read/write lock for read-only access.  This function
 * blocks until the lock can be acquired.  In case of success, the caller is
 * responsible for calling tao_rwlock_unlock() to eventually release the lock.
 *
 * @param lock   Pointer to the read/write lock.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlock_rdlock(
    tao_rwlock* lock);

/**
 * @brief Lock a read/write lock for read-write access.
 *
 * This function locks a read/write lock for read-write access.  This function
 * blocks until the lock can be acquired.  In case of success, the caller is
 * responsible for calling tao_rwlock_unlock() to eventually release the lock.
 *
 * @param lock   Pointer to the read/write lock.
 *
 * @return @ref TAO_OK if successful; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlock_wrlock(
    tao_rwlock* lock);

/**
 * @brief Attempt to lock a read/write lock for read-only access without
 * blocking.
 *
 * This function attempts to lock a read/write for read-only access.  This
 * function never blocks.  In case of success, the caller is responsible for
 * calling tao_rwlock_unlock() to eventually release the lock.
 *
 * @param lock   Pointer to the read/write lock.
 *
 * @return @ref TAO_OK if successful; @ref TAO_TIMEOUT if the lock cannot
 *         be acquired immediately; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlock_try_rdlock(
    tao_rwlock* lock);

/**
 * @brief Attempt to lock a read/write lock for read-write access without
 * blocking.
 *
 * This function attempts to lock a read/write for read-write access.  This
 * function never blocks.  In case of success, the caller is responsible for
 * calling tao_rwlock_unlock() to eventually release the lock.
 *
 * @param lock   Pointer to the read/write lock.
 *
 * @return @ref TAO_OK if successful; @ref TAO_TIMEOUT if the lock cannot
 *         be acquired immediately; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_rwlock_try_wrlock(
    tao_rwlock* lock);

/**
 * @brief Attempt to lock a read/write lock for read-only access without
 * blocking longer than a time limit.
 *
 * @param lock     Pointer to the read/write lock.
 *
 * @param abstime  Time limit (using `CLOCK_REALTIME` clock).
 *
 * @return @ref TAO_OK if successful; @ref TAO_TIMEOUT if the lock cannot
 *         be acquirred before the time limit; @ref TAO_ERROR in case of
 *         error.
 */
extern tao_status tao_rwlock_abstimed_rdlock(
    tao_rwlock* lock,
    const tao_time* abstime);

/**
 * @brief Attempt to lock a read/write lock for read-write access without
 * blocking longer than a time limit.
 *
 * @param lock     Pointer to the read/write lock.
 *
 * @param abstime  Time limit (using `CLOCK_REALTIME` clock).
 *
 * @return @ref TAO_OK if successful; @ref TAO_TIMEOUT if the lock cannot
 *         be acquired before the time limit; @ref TAO_ERROR in case of
 *         error.
 */
extern tao_status tao_rwlock_abstimed_wrlock(
    tao_rwlock* lock,
    const tao_time* abstime);

/**
 * @brief Attempt to lock a read/write lock for read-only access without
 * blocking more than a given duration.
 *
 * @param lock     Pointer to the read/write lock.
 *
 * @param secs     Maximum duration to wait (in seconds).  If this amount is
 *                 very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *                 effect is the same as calling tao_rwlock_rdlock().  If this
 *                 amount is very short, the effect is the same as calling
 *                 tao_rwlock_try_rdlock().
 *
 * @return @ref TAO_OK if successful; @ref TAO_TIMEOUT if the lock cannot
 *         be acquired before the time limit; @ref TAO_ERROR in case of
 *         error.
 */
extern tao_status tao_rwlock_timed_rdlock(
    tao_rwlock* lock,
    double secs);

/**
 * @brief Attempt to lock a read/write lock for read-write access without
 * blocking more than a given duration.
 *
 * @param lock     Pointer to the read/write lock.
 *
 * @param secs     Maximum duration to wait (in seconds).  If this amount is
 *                 very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *                 effect is the same as calling tao_rwlock_wrlock().  If this
 *                 amount is very short, the effect is the same as calling
 *                 tao_rwlock_try_wrlock().
 *
 * @return @ref TAO_OK if successful; @ref TAO_TIMEOUT if the lock cannot
 *         be acquired before the time limit; @ref TAO_ERROR in case of
 *         error.
 */
extern tao_status tao_rwlock_timed_wrlock(
    tao_rwlock* lock,
    double secs);

/**
 * @brief Unlock a read/write lock.
 *
 * This function is called to release a lock for read-only or read-write
 * access.
 *
 * @param lock   Address of the read/write lock.  The caller must own a
 *               lock for reading or for writing on this lock.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 *
 * @see tao_rwlock_rdlock(), tao_rwlock_wrlock(),
 *      tao_rwlock_try_rdlock(), tao_rwlock_try_wrlock(),
 *      tao_rwlock_abstimed_rdlock(), tao_rwlock_abstimed_wrlock(),
 *      tao_rwlock_timed_rdlock(), tao_rwlock_timed_wrlock().
 */
extern tao_status tao_rwlock_unlock(
    tao_rwlock* lock);

/**
 * @}
 */

/**
 * @defgroup Threads  Threads
 *
 * @ingroup ParallelProgramming
 *
 * @brief Threads.
 *
 * @{
 */

/**
 * @brief Thread identifier.
 *
 * This alias is to homogenize code style and simplify future changes.
 */
typedef pthread_t tao_thread;

/**
 * @brief Thread attributes.
 *
 * This alias is to homogenize code style and simplify future changes.
 */
typedef pthread_attr_t tao_thread_attr;

/**
 * Retrieve the identifier of the calling thread.
 *
 * @return The identifier of the calling thread.
 */
inline tao_thread tao_thread_self(void)
{
    return pthread_self();
}

/**
 * Compare thread identifiers.
 *
 * @param a  A thread identifier.
 * @param b  Another thread identifier.
 *
 * @return Non-zero if `a` and `b` identify the same thread; zero, otherwise.
 */
inline int tao_thread_equal(
    tao_thread tao_arg1_,
    tao_thread tao_arg2_)
{
    return pthread_equal(tao_arg1_, tao_arg2_);
}

/**
 * @brief Create a new thread.
 *
 * @param id     Pointer to store the thread identifier.
 *
 * @param attr   Attributes for the thread (can be `NULL`).
 *
 * @param start  Function to execute to run the thread.
 *
 * @param arg    Argument for the thread function.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_thread_create(
    tao_thread* id,
    const tao_thread_attr *attr,
    void *(*start)(void *),
    void *arg);

/**
 * @brief Detach a thread.
 *
 * @param id     Thread identifier.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_thread_detach(
    tao_thread id);

/**
 * @brief Send a cancellation request to a thread.
 *
 * @param id     Thread identifier.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_thread_cancel(
    tao_thread id);

/**
 * @brief Join with a terminated thread.
 *
 * @param id      Thread identifier.
 *
 * @param retval  If not `NULL`, address where to store the exit status et by the thread.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_thread_join(
    tao_thread id,
    void** retval);

/**
 * @}
 */

/**
 * @defgroup Semaphores  Semaphores
 *
 * @ingroup ParallelProgramming
 *
 * @brief Named and anonymous semaphores.
 *
 * Semaphores come in two flavors: named semaphores and anonymous semaphores.
 *
 * @{
 */

/**
 * @brief Initialize an anonymous semaphore.
 *
 * This function must be called to initialize an anonymous semaphore (stored in
 * shared memory).  The caller is responsible of eventually calling
 * tao_semaphore_destroy() when the semaphore is no longer needed.
 *
 * @param sem    Address of an anonymous semaphore to initialize.
 *
 * @param share  If set to @ref TAO_PROCESS_SHARED, require that the semaphore
 *               be accessible between processes; otherwise, must be @ref
 *               TAO_PROCESS_PRIVATE and the semaphore will be *private* (that
 *               is, only accessible by threads in the same process as the
 *               caller).
 *
 * @param value  Initial value of the anonymous semaphore.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_semaphore_initialize(
    sem_t* sem,
    tao_process_sharing share,
    unsigned int value);

/**
 * Destroy an anonymous semaphore.
 *
 * This function must be called to release the resources associated with an
 * anonymous semaphore initialized by tao_semaphore_initialize().
 *
 * @param sem    Address of anonymous semaphore to destroy.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_semaphore_destroy(
    sem_t* sem);

/**
 * Create a named semaphore.
 *
 * This function is called by the owner of a named semaphore to create it.  A
 * named semaphore with the same name must not already exists.  Call
 * tao_semaphore_open() to open an existing named semaphore.  Call
 * tao_semaphore_close() to close the access to the named semaphore for the
 * caller.  Call tao_semaphore_unlink() to remove the named semaphore.
 *
 * @param name   The name of the named semaphore.
 *
 * @param perms  Access permissions.
 *
 * @param value  Initial value of the named semaphore.
 *
 * @return The address of a new named semaphore; `NULL` in case of failure.
 */
extern sem_t* tao_semaphore_create(
    const char* name,
    int perms,
    unsigned int value);

/**
 * Open an existing named semaphore.
 *
 * This function is open an existing named semaphore.  The caller must call
 * tao_semaphore_close() to close the access to the named semaphore.
 *
 * @param name   The name of the named semaphore.
 *
 * @return The address of a new named semaphore; `NULL` in case of failure.
 */
extern sem_t* tao_semaphore_open(
    const char* name);

/**
 * Close a named semaphore.
 *
 * @param sem    Address of named semaphore to close.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_semaphore_close(
    sem_t* sem);

/**
 * Remove a named semaphore.
 *
 * @param name   Name of named semaphore to remove.
 *
 * @param force  Indicate whether the named semaphore may not exist.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_semaphore_unlink(
    const char* name,
    bool force);

/**
 * Get the current value of a semaphore.
 *
 * @param sem    Address of semaphore.
 *
 * @param val    Address of variable to store the semaphore value.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_semaphore_get_value(
    sem_t* sem,
    int* val);

/**
 * Increment the value of a semaphore.
 *
 * This function increments the value of a semaphore by one, thus unblocking
 * one of the processes or threads waiting to be able to decrement the
 * semaphore.
 *
 * @param sem    Address of semaphore.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_semaphore_post(
    sem_t* sem);

/**
 * Decrement the value of a semaphore.
 *
 * This function blocks until the value of the semaphore becomes strictly
 * positive and is decremented by one.
 *
 * @param sem    Address of semaphore.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_semaphore_wait(
    sem_t* sem);

/**
 * Attempt to decrement the value of a semaphore immediately.
 *
 * This function is similar to tao_semaphore_wait() but it returns immediately
 * and reports whether the value of the semaphore was decremented.
 *
 * @param sem    Address of semaphore.
 *
 * @return @ref TAO_OK if the value of the semaphore was successfully
 *         decremented; @ref TAO_TIMEOUT if the value of a semaphore was zero;
 *         @ref TAO_ERROR in case of another failure.
 */
extern tao_status tao_semaphore_try_wait(
    sem_t* sem);

/**
 * Attempt to decrement the value of a semaphore with a time limit.
 *
 * This function is similar to tao_semaphore_wait() but it waits no longer than
 * a given absolute time limit.
 *
 * @param sem      Address of semaphore.
 *
 * @param abstime  Absolute time limit for waiting.
 *
 * @return @ref TAO_OK if the was successfully decremented before the specified
 *         number of seconds; @ref TAO_TIMEOUT if timeout occurred before; @ref
 *         TAO_ERROR in case of another failure.
 */
extern tao_status tao_semaphore_abstimed_wait(
    sem_t* sem,
    const tao_time* abstime);

/**
 * Attempt to decrement the value of a semaphore with a time limit.
 *
 * This function is similar to tao_semaphore_wait() but it waits no longer than
 * a given number of seconds from now.
 *
 * @param sem    Address of semaphore.
 *
 * @param secs   Maximum time to wait (in seconds).  If this amount of time is
 *               very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling tao_semaphore_wait().  If this
 *               amount of time is very small, the effect is the same as
 *               calling tao_semaphore_try_wait().
 *
 * @return @ref TAO_OK if the was successfully decremented before the specified
 *         number of seconds; @ref TAO_TIMEOUT if timeout occurred before; @ref
 *         TAO_ERROR in case of another failure.
 */
extern tao_status tao_semaphore_timed_wait(
    sem_t* sem,
    double secs);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_THREADS_H_
