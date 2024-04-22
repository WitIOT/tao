// tao-cameras.h -
//
// Definitions and API for cameras in TAO.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_CAMERAS_H_
#define TAO_CAMERAS_H_ 1

#include <tao-basics.h>
#include <tao-utils.h>
#include <tao-options.h>
#include <tao-encodings.h>
#include <tao-shared-arrays.h>
#include <tao-remote-objects.h>

TAO_BEGIN_DECLS

//-----------------------------------------------------------------------------
// COMMON TYPES AND METHODS

/**
 * @addtogroup CamerasTools Camera tools
 *
 * @ingroup Cameras
 *
 * @brief Common types and methods for cameras.
 *
 * @{
 */

/**
 * Attribute type.
 *
 * - `TAO_ATTR_BOOLEAN` is a boolean stored as `attr->val.i = 0` for `false`
 *   and `attr->val.i = 1` for `true`.
 *
 * - `TAO_ATTR_INTEGER` is an integer value stored as `attr->val.i`.
 *
 * - `TAO_ATTR_FLOAT` is a floating-point value stored as `attr->val.f`.
 *
 * - `TAO_ATTR_STRING` is a textual value stored as `attr->val.s`, its length
 *   (including the final null) must be at most @ref TAO_ATTR_VAL_LEN.
 *
 */
typedef enum tao_attr_type {
    TAO_ATTR_BOOLEAN = 1,
    TAO_ATTR_INTEGER = 2,
    TAO_ATTR_FLOAT   = 3,
    TAO_ATTR_STRING  = 4
} tao_attr_type;

/**
 * @def TAO_ATTR_VARIABLE
 *
 * Macro `TAO_ATTR_VARIABLE` can be bitwise or'ed with attribute type to
 * indicate that the attribute may vary spontaneously (e.g., the temperature).
 */
#define TAO_ATTR_VARIABLE  (1 << 5)

/**
 * @def TAO_ATTR_READABLE
 *
 * Macro `TAO_ATTR_READABLE` can be bitwise or'ed with attribute type to
 * indicate that the attribute is readable.
 */
#define TAO_ATTR_READABLE  (1 << 6)

/**
 * @def TAO_ATTR_WRITABLE
 *
 * Macro `TAO_ATTR_WRITABLE` can be bitwise or'ed with attribute type to
 * indicate that the attribute is writable.
 */
#define TAO_ATTR_WRITABLE  (1 << 7)

/**
 * @def TAO_ATTR_KEY_LEN
 *
 * Macro `TAO_ATTR_KEY_LEN` is the maximum length (including the final null) of
 * the name of an attribute.
 */
#define TAO_ATTR_KEY_LEN 31

/**
 * @def TAO_ATTR_VAL_LEN
 *
 * Macro `TAO_ATTR_VAL_LEN` is the maximum length (including the final null) of
 * the string value of an attribute.
 */
#define TAO_ATTR_VAL_LEN 32

/**
 * Named attribute.
 *
 * Named atributes associate a key (the name of the attribute) and a value
 * in a small fixed size object that can be stored in shared memory.
 */
typedef struct tao_attr {
    char key[TAO_ATTR_KEY_LEN];
    uint8_t bits;
    union {
        int64_t i;
        double  f;
        char    s[TAO_ATTR_VAL_LEN];
    } val;
} tao_attr;

#define TAO_ATTR_TYPE(attr)   ((attr)->bits & 0x1f)

#define TAO_ATTR_ACCESS(attr) \
    ((attr)->bits & (TAO_ATTR_READABLE|TAO_ATTR_WRITABLE))

#define TAO_ATTR_IS_VARIABLE(attr) \
    (((attr)->bits & TAO_ATTR_VARIABLE) == TAO_ATTR_VARIABLE)

#define TAO_ATTR_IS_READABLE(attr) \
    (((attr)->bits & TAO_ATTR_READABLE) == TAO_ATTR_READABLE)

#define TAO_ATTR_IS_WRITABLE(attr) \
    (((attr)->bits & TAO_ATTR_WRITABLE) == TAO_ATTR_WRITABLE)

/**
 * Search an attribute in a list.
 *
 * This function searches an attribute with a given name in an array of
 * attributes.  The attribute names are case sensitive.  It is assumed that an
 * empty attribute name in the list indicates the end of the list whatever the
 * value of `len`.
 *
 * @param key   The name of the attribute to search.
 *
 * @param attr  An array of attributes.
 *
 * @param len   The maximum length of the array of attributes.  It is assumed
 *              that an empty attribute name in the list (at index less than
 *              `len`) indicates the end of the list.
 *
 * @return An index in the range `[0,len-1]` if the attribute is found; `-1` if
 *         the attribute is not found (whatever the reason).
 */
extern long tao_attr_search(
    const char* key,
    const tao_attr attr[],
    long len);

/**
 * Maximum number of camera attributes.
 */
#define TAO_CAMERA_CONFIG_ATTR_LEN 50

/**
 * @brief Level of image pre-processing.
 */
typedef enum tao_preprocessing {
    TAO_PREPROCESSING_NONE   = 0,///< Just convert pixel values.
    TAO_PREPROCESSING_AFFINE = 1,///< Apply affine correction.
    TAO_PREPROCESSING_FULL   = 2 ///< Apply affine correction and compute
                                 ///  weights.
} tao_preprocessing;

/**
 * Region of interest on a camera.
 */
typedef struct tao_camera_roi {
    long   xbin;///< Horizontal binning (in physical pixels).
    long   ybin;///< Vertical binning (in physical pixels).
    long   xoff;///< Horizontal offset of the acquired images with respect to
                ///  the left border of the detector (in physical pixels).
    long   yoff;///< Vertical offset of the acquired images with respect to the
                ///  bottom border of the detector (in physical pixels).
    long  width;///< Number of pixels per line of the acquired images (in
                ///  macro-pixels).
    long height;///< Number of lines of pixels in the acquired images (in
                ///  macro-pixels).
} tao_camera_roi;

/**
 * Copy the settings of a region of interest.
 *
 * This function copies the settings of the source @b src into the destination
 * @b dest and returns @b dest.
 *
 * @warning For efficiency, this function does not check its arguments.
 *
 * @param dest   Address of destination.
 * @param  src   Address of source.
 *
 * @return The destination @b dest.
 *
 * @see tao_camera_roi, tao_camera_roi_check(),
 * tao_camera_roi_define().
 */
extern tao_camera_roi* tao_camera_roi_copy(
    tao_camera_roi* dest,
    const tao_camera_roi* src);

/**
 * Define a region of interest.
 *
 * This function defines the members of @b roi and returns @b roi.
 *
 * @warning For efficiency, this function does not check its arguments.
 *
 * @param   dest   Address of region of interest.
 * @param   xbin   Horizontal binning (in physical pixels).
 * @param   ybin   Vertical binning (in physical pixels).
 * @param   xoff   Horizontal offset of ROI (in physical pixels).
 * @param   yoff   Vertical offset of ROI (in physical pixels).
 * @param  width   Horizontal size of ROI (in macro-pixels).
 * @param height   Vertical size of ROI (in macro-pixels).
 *
 * @return The region of interest @b roi.
 *
 * @see tao_camera_roi, tao_camera_roi_copy(),
 * tao_camera_roi_check().
 */
extern tao_camera_roi* tao_camera_roi_define(
    tao_camera_roi* dest,
    long xbin,
    long ybin,
    long xoff,
    long yoff,
    long width,
    long height);

/**
 * Check a region of interest.
 *
 * This function checks whether the region of interest settings in @b roi are
 * valid and compatible with the sensor dimensions given by @b sensorwidth and
 * @b sensorheight.
 *
 * @param          roi   Address of the region of interest to check.
 * @param  sensorwidth   Horizontal size of detector (in pixels).
 * @param sensorheight   Vertical size of detector (in pixels).
 *
 * @return @ref TAO_OK if the ROI is valid, @ref TAO_ERROR otherwise.
 *
 * @see tao_camera_roi, tao_camera_roi_copy(),
 * tao_camera_roi_define().
 */
extern tao_status tao_camera_roi_check(
    const tao_camera_roi* roi,
    long sensorwidth,
    long sensorheight);

/**
 * Pending events for a camera.
 *
 * Pending events are given as a combination of bits.  The following events
 * are implemented:
 *
 * - @b TAO_EVENT_START: Start acquisition.
 * - @b TAO_EVENT_FRAME: A new frame is available.
 * - @b TAO_EVENT_ERROR: Some (recoverable) error occurred.
 * - @b TAO_EVENT_STOP: Stop acquisition.
 * - @b TAO_EVENT_ABORT: Abort acquisition.
 * - @b TAO_EVENT_QUIT: Acquisition thread must quit.
 */
typedef unsigned int tao_event;

#define TAO_EVENT_COMMAND (((tao_event)1) << 0) // Command sent
#define TAO_EVENT_FRAME   (((tao_event)1) << 1) // New frame available
#define TAO_EVENT_ERROR   (((tao_event)1) << 2) // Some error occurred

/**
 * Common camera configuration.
 *
 * This structure contains the parameters of a camera.  There are 3 groups
 * of parameters:
 *
 * - **Common non-configurable parameters** include the sensor size, various
 *   counters, the origin of time, etc.  These parameters are only modified by
 *   the owner of the camera.
 *
 * - **Common configurable parameters** include the region of interest, the
 *   pixel type, the buffer and the sensor encoding, etc.  These parameters may
 *   be modified by a client of a remote camera.
 *
 * - **Camera specific attributes** are pairs of key (the name of the
 *   parameter) and value which may be readable and/or writable by a client of
 *   a remote camera.  To facilitate the management of these attributes, their
 *   order may not be modified.  The maximum number of such attributes is
 *   given by the value of the macro @ref TAO_CAMERA_CONFIG_ATTR_LEN.
 */
typedef struct tao_camera_config {
    // Common non-configurable parameters.
    const long   sensorwidth;///< Number of physical pixels per row of the
                             ///  detector.
    const long  sensorheight;///< Number of physical pixels per column of the
                             ///  detector.
    tao_time          origin;///< Origin of time.
    tao_serial        frames;///< Number of frames acquired so far.
    tao_serial droppedframes;///< Number of dropped frames.
    tao_serial      overruns;///< Number of frames lost because of overruns.
    tao_serial    lostframes;///< Number of lost frames.
    tao_serial     overflows;///< Number of overflows.
    tao_serial     lostsyncs;///< Number of synchronization losts so far.
    tao_serial      timeouts;///< Number of timeouts so far.

    // Common configurable parameters.
    tao_camera_roi              roi;///< Region of interest on the detector.
    double                framerate;///< Acquisition rate in frames per second.
    double             exposuretime;///< Exposure time in seconds.
    long                    buffers;///< Number of acquisition buffers.
    tao_eltype            pixeltype;///< Pixel type in pre-processed images.
    tao_encoding     sensorencoding;///< Pixel encoding in images acquired by
                                    ///  the sensor.
    tao_encoding     bufferencoding;///< Pixel encoding for acquisition
                                    ///  buffers.
    tao_preprocessing preprocessing;///< Level of image pre-processing.

    // Specific attributes (may be readable and/or writable).
    tao_attr attributes[
        TAO_CAMERA_CONFIG_ATTR_LEN];///< List of named attributes.

} tao_camera_config;

/**
 * Initialize camera configuration.
 *
 * This function initializes the contents of a given camera information
 * structure.  The contents is consistent but is not meant to represent
 * the actual configuration of a camera.
 *
 * @param cfg    Address of camera configuration structure.  Nothing is done
 *               if `NULL`.
 *
 * @see tao_camera_config_print().
 */
extern void tao_camera_config_initialize(
    tao_camera_config* cfg);

/**
 * Copy camera configuration.
 *
 * @param dst    Address of the destination camera configuration structure.
 *               Nothing is done if `NULL`.
 *
 * @param src    Address of the source camera configuration structure.
 *               Nothing is done if `NULL`.
 *
 * @see tao_camera_config_initialize().
 */
extern void tao_camera_config_copy(
    tao_camera_config* dst,
    const tao_camera_config* src);

/**
 * Print camera configuration.
 *
 * @param out   The output file stream where to print.  The standard output
 *              stream is assumed if @a out is `NULL`.
 *
 * @param cfg   Address of camera configuration structure.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_camera_config_print().
 */
extern tao_status tao_camera_config_print(
    FILE* out,
    const tao_camera_config* cfg);

/**
 * @brief Get fast pixel type for processed images.
 *
 * @param proc   Image pre-processing method.
 *
 * @param enc    Raw pixel encoding.
 *
 * @return A suitable @ref tao_eltype for storing processed images,
 *         -1 in case of error.
 */
extern tao_eltype tao_fast_pixel_type(
    tao_preprocessing proc,
    tao_encoding enc);

/**
 * @}
 */

//-----------------------------------------------------------------------------
// UNIFIED INTERFACE FOR CAMERA DEVICES

/**
 * @addtogroup UnifiedCameras  Unified cameras
 *
 * @ingroup Cameras
 *
 * @brief Unified API for cameras.
 *
 * TAO provides generic structures and functions to operate real cameras in a
 * unified and yet flexible way.
 *
 * The following functions implement the API:
 *
 * - tao_camera_create() creates a new camera instance.
 *
 * - tao_camera_destroy() destroys a camera instance.
 *
 * - tao_camera_lock() and tao_camera_try_lock() locks a camera instance for
 *   exclusive (read-write) access.
 *
 * - tao_camera_unlock() unlocks a camera instance.
 *
 * - tao_camera_signal() and tao_camera_boeadcast() notify other threads that
 *   something may have changed in the camera instance.
 *
 * - tao_camera_wait(), tao_camera_abstimed_wait(), and tao_camera_timed_wait()
 *   wait for some changes(s) to occur in the camera instance.
 *
 * - tao_camera_start_acquisition() starts image acquisition by a camera
 *   instance.
 *
 * - tao_camera_stop_acquisition() stops image acquisition by a camera
 *   instance.
 *
 * - tao_camera_wait_acquisition_buffer() waits for a new image to be available
 *   from a camera instance.
 *
 * - tao_camera_reset() attempts to reset a camera to an idle state.
 *
 * - tao_camera_get_state() get the state of a camera instance.
 *
 * - tao_camera_check_configuration(), tao_camera_get_configuration(),
 *   tao_camera_set_configuration(), and tao_camera_reflect_configuration()
 *   deal with the configuration of a camera instance.
 *
 * - tao_camera_set_origin_of_time() and tao_camera_get_origin_of_time()
 *   respectively sets and retrieves the origin of time for a camera instance.
 *
 * - tao_camera_get_elapsed_seconds(), tao_camera_get_elapsed_milliseconds(),
 *   tao_camera_get_elapsed_microseconds(), and
 *   tao_camera_get_elapsed_nanoseconds() yields the time elapsed since the
 *   origin of time for a camera instance.
 *
 * @warning A camera can be shared between several threads (of the same
 *          process).  For instance, the acquisition callback of "Phoenix"
 *          cameras is run in another thread.  A mutex and a condition variable
 *          are embedded into a camera instance to protect its resources and
 *          synchronize its users; locking and unlocking the camera
 *          appropriately is critical.  Most functions operating on a camera
 *          assume that the caller have locked the camera.  The only exceptions
 *          are: tao_camera_destroy() and, of course, tao_camera_lock() and
 *          tao_camera_try_lock().  It is always the responsibility of the
 *          owner of a lock to eventually release it.
 *
 * @{
 */

/**
 * @brief Acquisition buffer information.
 *
 * This structure is used to store all needed information about an acquisition
 * buffer returned by tao_camera_wait().  The contents is considered as purely
 * informative by the high-level interface.  For instance the high-level
 * interface does not attempt to alloc or free the acquisition buffer data,
 * these tasks are performed by the low-level interface (e.g. by the "start"
 * and "finalize" virtual methods).
 *
 * A valid buffer counter should be equal to the corresponding frame counter
 * which is greater or equal 1.  When acquisition is started by
 * tao_camera_start_acquisition(), all buffer counters are set to 0.
 */
typedef struct tao_acquisition_buffer {
    void*            data;///< Address of buffer data.
    size_t           size;///< Number of bytes in buffer.
    long           offset;///< Offset (in bytes) of first pixel in ROI.
    long            width;///< Number of pixel per line in ROI.
    long           height;///< Number of lines in ROI.
    long           stride;///< Bytes per line in buffer (including padding).
    tao_encoding encoding;///< Pixel encoding in buffer.
    tao_serial     serial;///< Serial number of frame.
    tao_time  frame_start;///< Start time of frame.
    tao_time    frame_end;///< End time of frame.
    tao_time buffer_ready;///< Buffer ready time.
} tao_acquisition_buffer;

/**
 * @brief Opaque camera structure.
 */
typedef struct tao_camera tao_camera;

/**
 * @brief Opaque camera operations structure.
 */
typedef struct tao_camera_ops tao_camera_ops;

/**
 * @brief Create a new camera instance.
 *
 * This function create a new camera instance.  This function is typically
 * called by other "camera constructors".  Argument @b ops provides all the
 * methods needed to operate the specific model of camera.  Argument @b ctx is
 * some contextual data (stored as member `ctx` by this function but never
 * directly used otherwise).  Argument @b size specifies the number of bytes to
 * allocate for the camera instance (at least `sizeof(tao_camera)` bytes will
 * be allocated to store the @ref tao_camera structure.  See the code of
 * phnx_create_camera() for an example.
 *
 * @warning In case of failure of tao_camera_create(), the `finalize` method in
 *          @b ops is not called.
 *
 * @param ops   Virtual operations table.
 *
 * @param ctx   Address of contextual data.   This value is passed to the
 *              `initialize` method.
 *
 * @param size  Number of bytes to allocate.
 *
 * @return The address of the new (initialized) camera; `NULL` in case of
 *         failure.
 */
extern tao_camera* tao_camera_create(
    const tao_camera_ops* ops,
    void* ctx,
    size_t size);

/**
 * @brief Release the resources associated to a camera.
 *
 * This function aborts any acquisition with the associated camera and releases
 * all related resources.  The camera must not be locked when calling this
 * function.
 *
 * @warning The camera must not have been locked by the caller.
 *
 * @param camera      A pointer to the camera instance.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_camera_create().
 */
extern tao_status tao_camera_destroy(
    tao_camera* cam);

/**
 * @brief Lock a camera.
 *
 * This function locks the mutex of the camera @b cam.  The caller is
 * responsible of eventually calling tao_camera_unlock().
 *
 * Locking a camera is needed before changing anything in the camera instance
 * because the camera instance may be shared between the thread handling frame
 * grabber events and the other threads.  Make sure to lock and unlock a camera
 * before calling functions that may modify the camera structure.  Make sure
 * that the camera is not locked before calling tao_camera_destroy().
 *
 * @warning The camera must not have been locked by the caller.
 *
 * @param cam    Address of camera instance (must be non-`NULL`).
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_camera_lock(
    tao_camera* cam);

/**
 * @brief Try to lock a camera.
 *
 * This function attempts to lock the mutex of the camera @b cam without
 * blocking.  If the call is successful, the caller is responsible of
 * eventually calling tao_camera_unlock().
 *
 * @warning The camera must not have been locked by the caller.
 *
 * @param cam    Address of camera instance (must be non-`NULL`).
 *
 * @return @ref TAO_OK if camera has been locked by the caller; @ref TAO_ERROR
 *         otherwise (that is, if the camera is already locked by some other
 *         thread/process).
 */
extern tao_status tao_camera_try_lock(
    tao_camera* cam);

/**
 * @brief Unlock a camera.
 *
 * This function unlocks the mutex of the camera @b cam.  The caller must have
 * locked the camera @b cam with tao_camera_lock() or tao_camera_try_lock().
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance (must be non-`NULL`).
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_camera_unlock(
    tao_camera* cam);

/**
 * @brief Signal a change for a camera to one waiting threads.
 *
 * This function restarts one of the threads that are waiting on the condition
 * variable of the camera @b cam.  Nothing happens, if no threads are waiting
 * on the condition variable of @b cam.  The caller is assumed to have locked
 * the camera before calling this function and to unlock the camera soon after
 * calling this function to effectively trigger the notification to others.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance (must be non-`NULL`).
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_camera_wait(), tao_camera_timed_wait(), tao_camera_abstimed_wait(),
 *      tao_camera_broadcast().
 */
extern tao_status tao_camera_signal(
    tao_camera* cam);

/**
 * @brief Signal a change for a camera to all waiting threads.
 *
 * This function restarts all the threads that are waiting on the condition
 * variable of the camera @b cam.  Nothing happens, if no threads are waiting
 * on the condition variable of @b cam.  The caller is assumed to have locked
 * the camera before calling this function and to unlock the camera soon after
 * calling this function to effectively trigger the notification to others.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance (must be non-`NULL`).
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 *
 * @see tao_camera_wait(), tao_camera_timed_wait(), tao_camera_abstimed_wait(),
 * tao_camera_signal().
 */
extern tao_status tao_camera_broadcast(
    tao_camera* cam);

/**
 * @brief Wait for a condition to be signaled for a camera.
 *
 * This function atomically unlocks the exclusive lock associated with a
 * camera instance and waits for its associated condition variable to be
 * signaled.  The thread execution is suspended and does not consume any CPU
 * time until the condition variable is signaled.  The mutex of the camera must
 * have been locked (e.g., with tao_camera_lock()) by the calling thread
 * on entrance to this function.  Before returning to the calling thread, this
 * function re-acquires the mutex.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance (must be non-`NULL`).
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_camera_wait(
    tao_camera* cam);

/**
 * Wait for a condition to be signaled for a camera without blocking
 * longer than an absolute time limit.
 *
 * This function behaves like tao_camera_wait() but blocks no longer than a
 * given duration.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance (must be non-`NULL`).
 *
 * @param lim    Absolute time limit with the same conventions as
 *               tao_get_current_time().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_camera_abstimed_wait(
    tao_camera* cam,
    const tao_time* abstime);

/**
 * Wait for a condition to be signaled for a camera without blocking
 * longer than a a relative time limit.
 *
 * This function behaves like tao_camera_wait() but blocks no longer than a
 * given duration.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance (must be non-`NULL`).
 *
 * @param secs   Maximum amount of time (in seconds).  If this amount of time
 *               is very large, e.g. more than @ref TAO_MAX_TIME_SECONDS, the
 *               effect is the same as calling
 *               tao_camera_wait().
 *
 * @return @ref TAO_OK if the lock has been locked by the caller before the
 *         specified time limit, @ref TAO_TIMEOUT if timeout occurred before or
 *         @ref TAO_ERROR in case of error.
 */
extern tao_status tao_camera_timed_wait(
    tao_camera* cam,
    double secs);

/**
 * @brief Retrieve all the parameters of a camera.
 *
 * This function copies all camera information to the destination structure.
 * The caller may call tao_camera_update_configuration() before to make sure
 * that the camera parameters reflects the hardware settings.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param src   Address of source instance.
 *
 * @param dst   Address of destination structure.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of errors.
 */
extern tao_status tao_camera_get_information(
    const tao_camera* src,
    tao_camera_config* dst);

/**
 * Check pixel conversion and pre-processing settings.
 *
 * This functions check whether the `bufferencoding`, `pixeltype`, and
 * `preprocessing` configuration parameters are compatible.
 *
 * @param cfg   The address of the configuration to check.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of errors.
 */
extern tao_status tao_camera_configuration_check_preprocessing(
    const tao_camera_config* cfg);

/**
 * Check a camera configuration.
 *
 * This function checks whether a configuration is valid for a given camera.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance.
 * @param cfg    Address of configuration to check.
 *
 * @return @ref TAO_OK if the configuration is valid; @ref TAO_ERROR in case of
 *         errors.
 */
extern tao_status tao_camera_check_configuration(
    tao_camera* cam,
    const tao_camera_config* cfg);

/**
 * @brief Update camera configuration from current hardware settings.
 *
 * This function updates the camera internal configuration from the current
 * hardware settings.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of errors.
 */
extern tao_status tao_camera_update_configuration(
    tao_camera* cam);

/**
 * @brief Retrieve the configurable parameters of a camera.
 *
 * This function copies the camera internal configuration to the destination
 * structure.   The caller may call tao_camera_update_configuration() before
 * to make sure that the camera configuration reflects the hardware settings.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param src   Address of camera instance.
 *
 * @param dst   Address of destination configuration structure.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of errors.
 */
extern tao_status tao_camera_get_configuration(
    const tao_camera* src,
    tao_camera_config* dst);

/**
 * @brief Change camera settings.
 *
 * This function attempts to set the camera settings according to the source
 * configuration.  In case of errors, the caller may call
 * tao_camera_update_configuration() and then tao_camera_get_configuration() to
 * retrieve actual hardware settings.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam   Address of camera instance.
 *
 * @param cfg   Address of soruce configuration structure.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of errors.
 */
extern tao_status tao_camera_set_configuration(
    tao_camera* cam,
    const tao_camera_config* cfg);

extern void tao_camera_roi_option_show(
    FILE* file,
    const tao_option* opt);

extern bool tao_camera_roi_option_parse(
    const tao_option* opt,
    char* args[]);

/**
 * @brief Wait for an acquisition buffer to be available.
 *
 * This function waits for an acquisition buffer to be available from a camera
 * not blocking longer than a given duration.
 *
 * @warning The caller must own the lock on the camera when calling
 *          tao_camera_wait_acquisition_buffer().  Unlocking the camera during
 *          the processing of the acquisition buffer, as in the below example,
 *          is recommended if this operation is time consuming.
 *
 * @warning The returned acquisition buffer must be used before the next call
 *          to tao_camera_wait_acquisition_buffer() of
 *          tao_camera_stop_acquisition().  There are no guarantees that this
 *          contents may still exist after.
 *
 * Complete example of acquisition loop:
 *
 * ~~~~~{.c}
 * long timouts = 0, frames = 0, number = 100;
 * int drop = 0; // keep all images
 * double secs = 30.0; // max. number of seconds to wait for each buffer
 * tao_camera_lock(cam);
 * tao_status status = tao_camera_start_acquisition(cam);
 * while (status == TAO_OK && frames < number) {
 *     const tao_acquisition_buffer* buf;
 *     status = tao_camera_wait_acquisition_buffer(cam, &buf, secs, drop);
 *     if (status == TAO_OK) {
 *         // Process acquisition buffer and release buffer (unlock camera
 *         // during processing).
 *         tao_camera_unlock(cam);
 *         process(buf->data, buf->size, ...); // do the processing
 *         tao_camera_lock(cam);
 *         ++frames;
 *     } else if (status == TAO_TIMEOUT) {
 *         // Timeout occurred.
 *         if (++timeouts > 4) {
 *             fprintf(stderr, "Too many timouts, aborting acquisition...\");
 *         } else {
 *             status = TAO_OK;
 *         }
 *     }
 * }
 * tao_camera_stop_acquisition(cam);
 * tao_camera_unlock(cam);
 * if (tao_any_errors()) {
 *     tao_report_errors();
 * }
 * ~~~~~
 *
 * @param cam     Address of camera instance.
 *
 * @param buf     Address of structure to retrieve information about
 *                acquisition buffer.  In case of time-out or of error,
 *                `buf->data` is set to `NULL` on return.
 *
 * @param secs    Maximum number of seconds to wait.  Must be nonnegative.
 *                Wait forever if @b secs is too large, e.g. more than
 *                @ref TAO_MAX_TIME_SECONDS.
 *
 * @param drop    This parameter specifies how to deal with pending acquisition
 *                buffers if any.  If there are no pending acquisition buffers,
 *                the function waits for a new buffer to be acquired (but no
 *                longer than the given timeout).  Otherwise, if @b drop is
 *                less than 1, the first (and oldest) pending buffer is
 *                delivered; if @b drop is equal to 1, the last (and newest)
 *                pending buffer is delivered and all other pending buffers are
 *                released; if @b drop is greater than 1, all pending buffers
 *                are released and only a freshly acquired buffer can be
 *                returned.  For cameras which do not know the number of
 *                pending buffers, this argument is ignored.
 *
 * @return @ref TAO_OK, if a new acquisition buffer was available; @ref
 *         TAO_TIMEOUT, if no new acquisition buffer were available before
 *         `secs` seconds expired; @ref TAO_ERROR if some errors occurred.
 *
 * @see tao_camera_start_acquisition(), tao_camera_lock(), tao_camera_unlock(),
 * tao_camera_stop_acquisition(), tao_get_absolute_timeout().
 */
extern tao_status tao_camera_wait_acquisition_buffer(
    tao_camera* cam,
    tao_acquisition_buffer* buf,
    double secs,
    int drop);

/**
 * @brief Start image acquisition.
 *
 * This function starts image acquisition by a given camera.  If acquisition is
 * already running, nothing is done.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR on failure.
 */
extern tao_status tao_camera_start_acquisition(
    tao_camera* cam);

/**
 * @brief Stop image acquisition.
 *
 * This function stops image acquisition by a given camera.  If acquisition is
 * not running, nothing is done.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR on failure.
 */
extern tao_status tao_camera_stop_acquisition(
    tao_camera* cam);

/**
 * @brief Reset camera.
 *
 * This function attempts to reset a camera to idle state after an error.  If
 * acquistion is running, it is stopped.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR on failure.
 */
extern tao_status tao_camera_reset(
    tao_camera* cam);

/**
 * @brief Get the state of a camera.
 *
 * This function retrieves the state of a camera instance.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance.
 *
 * @return The camera state, @ref TAO_STATE_ERROR if the camera is in a
 *         recoverable error state, and @ref TAO_STATE_UNREACHABLE if the
 *         camera is in a fatal error state or if the argument is `NULL`.
 *         Whatever the result, this function leaves the caller's last error
 *         unchanged.
 */
extern tao_state tao_camera_get_state(
    const tao_camera* cam);

/**
 * @brief Set the origin of time for a camera.
 *
 * This function changes the origin of time of a camera instance.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance.
 *
 * @param orig   Address of structure with the origin of time.  If `NULL`,
 *               tao_get_monotonic_time() is called to set the origin of
 *               to the current time (using the monotonic clock if available).
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of errors.
 */
extern tao_status tao_camera_set_origin_of_time(
    tao_camera* cam,
    const tao_time* orig);

/**
 * @brief Get the origin of time of a camera.
 *
 * This function retrieves the origin of time of a camera instance.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param orig   Address of structure to store the origin of time of the
 *               camera.  Can be `NULL` to not store the origin of time.
 *
 * @param cam    Address of camera instance.
 *
 * @return A pointer to a structure with the origin of time of @b cam.
 */
extern const tao_time* tao_camera_get_origin_of_time(
    tao_time* orig,
    const tao_camera* cam);

/**
 * @brief Get the number of seconds since a camera origin of time.
 *
 * This function yields the elapsed time (in seconds) between the origin of
 * time registered in a camera instance and a given absolute time or the
 * current time.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance.
 *
 * @param t      Address of structure with the absolute time.  If `NULL`,
 *               tao_get_monotonic_time() is called to set the origin of
 *               to the current time (using the monotonic clock if available).
 *
 * @return A number of seconds, `NAN` is @a cam is `NULL`.
 *
 * @see tao_camera_set_origin_of_time(), tao_camera_get_elapsed_milliseconds(),
 * tao_camera_get_elapsed_microseconds(), tao_camera_get_elapsed_nanoseconds(),
 * tao_get_monotonic_time().
 */
extern double tao_camera_get_elapsed_seconds(
    const tao_camera* cam,
    const tao_time* t);

/**
 * @brief Get the number of milliseconds since a camera origin of time.
 *
 * This function yields the elapsed time (in milliseconds) between the origin
 * of time registered in a camera instance and a given absolute time or the
 * current time.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance.
 *
 * @param t      Address of structure with the absolute time.  If `NULL`,
 *               tao_get_monotonic_time() is called to set the origin of
 *               to the current time (using the monotonic clock if available).
 *
 * @return A number of milliseconds, `NAN` is @a cam is `NULL`.
 *
 * @see tao_camera_set_origin_of_time(), tao_camera_get_elapsed_seconds(),
 * tao_camera_get_elapsed_microseconds(), tao_camera_get_elapsed_nanoseconds(),
 * tao_get_monotonic_time().
 */
extern double tao_camera_get_elapsed_milliseconds(
    const tao_camera* cam,
    const tao_time* t);

/**
 * @brief Get the number of microseconds since a camera origin of time.
 *
 * This function yields the elapsed time (in microseconds) between the origin
 * of time registered in a camera instance and a given absolute time or the
 * current time.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance.
 *
 * @param t      Address of structure with the absolute time.  If `NULL`,
 *               tao_get_monotonic_time() is called to set the origin of
 *               to the current time (using the monotonic clock if available).
 *
 * @return A number of microseconds, `NAN` is @a cam is `NULL`.
 *
 * @see tao_camera_set_origin_of_time(), tao_camera_get_elapsed_seconds(),
 * tao_camera_get_elapsed_milliseconds(), tao_camera_get_elapsed_nanoseconds(),
 * tao_get_monotonic_time().
 */
extern double tao_camera_get_elapsed_microseconds(
    const tao_camera* cam,
    const tao_time* t);

/**
 * @brief Get the number of nanoseconds since a camera origin of time.
 *
 * This function yields the elapsed time (in nanoseconds) between the origin of
 * time registered in a camera instance and a given absolute time or the
 * current time.
 *
 * @warning The caller must own the lock on the camera.
 *
 * @param cam    Address of camera instance.
 *
 * @param t      Address of structure with the absolute time.  If `NULL`,
 *               tao_get_monotonic_time() is called to set the origin of
 *               to the current time (using the monotonic clock if available).
 *
 * @return A number of nanoseconds, `NAN` is @a cam is `NULL`.
 *
 * @see tao_camera_set_origin_of_time(), tao_camera_get_elapsed_seconds(),
 * tao_camera_get_elapsed_milliseconds(),
 * tao_camera_get_elapsed_microseconds(), tao_get_monotonic_time().
 */
extern double tao_camera_get_elapsed_nanoseconds(
    const tao_camera* cam,
    const tao_time* t);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_CAMERAS_H_
