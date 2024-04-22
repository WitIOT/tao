// tao-shared-memory.h -
//
// Definitions for shared memory in TAO library.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_SHARED_MEMORY_H_
#define TAO_SHARED_MEMORY_H_ 1

#include <tao-basics.h>

TAO_BEGIN_DECLS

/**
 * @defgroup SharedMemory  Shared memory
 *
 * @ingroup ParallelProgramming
 *
 * @brief Operations on shared memory.
 *
 * Header file @ref tao-shared-memory.h provides definitions for basic
 * operations on shared memory.  For efficiency, System V shared memory is
 * used.
 *
 * @{
 */

/**
 * Type of shared memory identifier.
 */
typedef int32_t tao_shmid;

/**
 * @def TAO_BAD_SHMID
 *
 * This macro is the value used by tao_shared_memory_create() to indicate an
 * invalid shared memory identifier.
 */
#define TAO_BAD_SHMID   ((tao_shmid)-1)

/**
 * Create a new shared memory segment.
 *
 * This function creates a new shared memory segment and attaches it to the
 * address space of the caller.  The contents of the shared memory is initially
 * zero-filled.  The caller is responsible of calling
 * tao_shared_memory_detach() to detach the shared memory segment from its
 * address space.
 *
 * @param shmid_ptr If not `NULL`, the address where to store the identifier
 *                  of the new shared memory segment.  If creating the shared
 *                  memory segment fails, the value @ref TAO_BAD_SHMID is
 *                  stored at `shmid_ptr`.
 *
 * @param size      Total number of bytes to allocate.
 *
 * @param perms     A combination of bits specifying the permissions granted to
 *                  the owner, group, and others as for the system `open(2)`
 *                  function.
 *
 * @return The location of the shared memory segment in the caller address
 *         space or `NULL` in case of failure.
 *
 * @see TAO_BAD_SHMID, tao_shared_memory_attach, tao_shared_memory_detach,
 *      tao_shared_memory_destroy.
 */
extern void* tao_shared_memory_create(
    tao_shmid* shmid_ptr,
    size_t size,
    unsigned flags);

/**
 * Attach shared memory segment.
 *
 * This function attaches a shared memory segment to the address space of the
 * caller.  The caller is responsible of calling tao_shared_memory_detach() to
 * detach the shared memory segment from its address space.
 *
 * @param shmid     The shared memory identifier.
 *
 * @param sizeptr   If not `NULL`, the address where to store the total number
 *                  of bytes of the shared memory segment.
 *
 * @return The location of the shared memory segment in the caller address
 *         space or `NULL` in case of failure.
 *
 * @see tao_shared_memory_create, tao_shared_memory_detach,
 *      tao_shared_memory_destroy.
 */
extern void* tao_shared_memory_attach(
    tao_shmid shmid,
    size_t* sizeptr);

/**
 * Detach shared memory segment.
 *
 * This function detaches a shared memory segment from the address space of the
 * caller.
 *
 * @param addr      The location of the shared memory segment in the address
 *                  space of the caller as returned by
 *                  tao_shared_memory_create() or by
 *                  tao_shared_memory_attach().
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_shared_memory_create, tao_shared_memory_attach,
 *      tao_shared_memory_destroy.
 */
extern tao_status tao_shared_memory_detach(
    void* addr);

/**
 * Destroy shared memory segment.
 *
 * This function manages to have the shared memory identified by `shmid`.
 *
 * On Linux, this function may be called while the shared memory is attached,
 * destruction will ne effective on last detach.
 *
 * On MacOS, this function shall be called while the shared memory is not
 * attached by any process.
 *
 * @param shmid     The identifier of the shared memory segment.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_shared_memory_create, tao_shared_memory_attach,
 *      tao_shared_memory_detach.
 */
extern tao_status tao_shared_memory_destroy(
    tao_shmid shmid);

/**
 * Query shared memory information.
 *
 * This function yields the size and the number of attachments of a shared
 * memory segment given its identifier.  If the identifier is invalid or if the
 * shared memory segment has been destroyed, the size and number of attachments
 * are both assumed to be zero.  This function can be safely called to check
 * for the existence of a shared memory segment.
 *
 * @param shmid   Shared memory identifier.
 *
 * @param segsz   If non-`NULL`, the address where to store the size of the
 *                shared memory segment in bytes.
 *
 * @param nattch  If non-`NULL`, the address where to store the number of
 *                attachments of the shared memory segment.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR if the identifier is invalid
 *         or if the shared memory segment has been destroyed.
 */
extern tao_status tao_shared_memory_stat(
    tao_shmid shmid,
    size_t* segsz,
    int64_t* nattch);

/**
 * @}
 */

#endif // TAO_SHARED_MEMORY_H_
