// tao-threadpools.h -
//
// Definitions for thread-pools in TAO.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-20212, Éric Thiébaut.

#ifndef TAO_THREADPOOLS_H_
#define TAO_THREADPOOLS_H_ 1

#include <tao-basics.h>

TAO_BEGIN_DECLS

/**
 * @defgroup ThreadPools  Thread-pools
 *
 * @ingroup ParallelProgramming
 *
 * @brief Pools of worker threads.
 *
 * TAO Library provides minimalist thread-pools which are managed by
 * only 4 functions:
 *
 * - tao_threadpool_create() to create a new thread-pool,
 * - tao_threadpool_push_job() to push a new job to the end of the job queue
 *   of a thread-pool,
 * - tao_threadpool_wait() to wait for completion of all jobs,
 * - tao_threadpool_destroy() to eventually destroy a thread-pool.
 *
 * Example of use:
 *
 * ~~~~~{.c}
 * // Create a thread pool of 4 workers.
 * tao_threadpool* pool = tao_threadpool_create(4);
 *
 * // Push a bunch of jobs and then wait for all jobs to complete.
 * tao_threadpool_push_job(pool, func1, arg1);
 * tao_threadpool_push_job(pool, func2, arg2);
 * ...
 * tao_threadpool_push_job(pool, funcN, argN);
 * tao_threadpool_wait(pool);
 *
 * // Push another bunch of jobs and then wait for all jobs to complete.
 * tao_threadpool_push_job(pool, otherfunc1, otherarg1);
 * tao_threadpool_push_job(pool, otherfunc2, otherarg2);
 * ...
 * tao_threadpool_push_job(pool, otherfuncN, otherargN);
 * tao_threadpool_wait(pool);
 *
 * // Destroy the pool when no longer needed (unstarted jobs will be
 * // abandoned).
 * tao_threadpool_destroy(pool);
 * ~~~~~
 *
 * @{
 */

/**
 * Opaque structure to represent a simple thread pool.
 *
 * A very minimalist thread-pool is implemented.
 *
 */
typedef struct tao_threadpool tao_threadpool;

/**
 * Create a thread-pool.
 *
 * @param workers  The number of workers (will be at least 2).
 *
 * @return The address of the thread-pool, `NULL` in case of failure.  It is the
 *         caller's responsibility to call tao_threadpool_destroy() to
 *         eventually release resources associated with the thread-pool.
 */
extern tao_threadpool* tao_threadpool_create(
    int workers);

/**
 * Destroy a thread-pool.
 *
 * This function destroys a thread-pool releasing all associated resources.
 * Calling this function waits for running jobs to complete.  Running jobs are
 * completed but unstarted jobs are abandoned.
 *
 * @param pool  The address of the thread-pool to destroy.
 */
extern void tao_threadpool_destroy(
    tao_threadpool* pool);

/**
 * Push a new job to be executed by a thread-pool.
 *
 * This function pushes a new job in the queue of pending jobs of a
 * thread-pool.  Execution of the job is started as soon as possible but after
 * previouly pushed jobs have been completed.  It is the caller responsibility
 * to make sure that the resources used by the function specified to execute
 * the job will remain accessible when this function will be executed.
 *
 * @param pool  The address of the thread-pool.
 *
 * @param func  The function to call for executing the job.
 *
 * @param arg   Some argument to pass to `func`.
 *
 * @return @ref TAO_OK is the job has been successfully pushed; @ref TAO_ERROR
 *         in case of failure.
 */
extern tao_status tao_threadpool_push_job(
    tao_threadpool* pool,
    void (*func)(void*),
    void* arg);

/**
 * Wait for jobs to complete.
 *
 * This function waits for all pending jobs in a thread-pool to be
 * finished.
 *
 * @param pool  The address of the thread-pool to wait on.
 *
 * @return @ref TAO_OK is the job has been successfully pushed; @ref TAO_ERROR
 *         in case of failure.
 */
extern tao_status tao_threadpool_wait(
    tao_threadpool* pool);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_THREADPOOLS_H_
