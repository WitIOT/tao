/*
 * tao-rt.i --
 *
 * Yorick interface to TAO real-time software.  TAO is a library for Adaptive
 * Optics software
 *
 *-----------------------------------------------------------------------------
 *
 * This file is part of the TAO real-time software licensed under the MIT
 * license (https://git-cral.univ-lyon1.fr/tao/tao-rt-yorick).
 *
 * Copyright (C) 2018-2021, Éric Thiébaut.
 */

if (is_func(plug_in)) plug_in, "ytao";

local TAO_PERSISTENT, TAO_SHARED_MAGIC;
extern tao_attach_shared_object;
extern tao_create_shared_object;
extern tao_create_remote_object;
extern tao_create_rwlocked_object;
/* DOCUMENT obj = tao_attach_shared_object(shmid);
         or obj = tao_create_shared_object(type, size, flags);
         or obj = tao_create_rwlocked_object(type, size, flags);
         or obj = tao_create_remote_object(owner, type, nbufs, offset, stride,
                                           size, flags);

     The first function attaches the shared TAO object whose shared memory
     identifier is `shmid` to the address space of the Yorick process and
     returns it.

     The other functions create basic TAO shared objects of a given super-type.
     These functions are intended for debugging and test purposes, use `type ≥
     5` to avoid creating an object with an already existing type.

     Arguments are (`size` and `flags` may be omitted):

     - owner:  Short string identifying the creator of the shared resource.
     - type:   Type identifier of the object.
     - nbufs:  Number of output buffers.
     - offset: Offset (in bytes) to the first output buffer.
     - stride: Rounded-up size of an output buffer.
     - size:   Total number of bytes to allocate.
     - flags:  Options and permissions granted to the group and to the others.
               At least, read and write access are granted for the caller.
               Unless bit `TAO_PERSISTENT` is set in `flags`, the shared memory
               backing the storage of the shared data will be destroyed upon
               last detach.

     Shared objects and r/w locked objects have the following members:

         obj.flags __________ access permissions and other flags;
         obj.lock ___________ lock state;
         obj.nrefs __________ number of attachments;
         obj.shmid __________ shared memory identifier;
         obj.size ___________ size (in bytes) of object;
         obj.type ___________ type of object (i.e. `TAO_SHARED_ARRAY`);

     Remote objects have the following members:

         obj.alive __________ remote server is alive?
         obj.flags __________ access permissions and other flags;
         obj.lock ___________ lock state;
         obj.nbufs __________ number of output buffers;
         obj.ncmds __________ number of processed commands;
         obj.nrefs __________ number of attachments;
         obj.owner __________ name of server owning the remote object;
         obj.serial _________ number of published output buffers;
         obj.shmid __________ shared memory identifier;
         obj.size ___________ size (in bytes) of object;
         obj.state __________ server state;
         obj.type ___________ type of object (i.e. `TAO_SHARED_ARRAY`);

     The returned shared objects are automatically detached when no longer
     referenced by Yorick.

   SEE ALSO: tao_attach_shared_object, tao_is_shared_object,
             tao_create_shared_array.
 */

local TAO_SHARED_OBJECT, TAO_RWLOCKED_OBJECT, TAO_REMOTE_OBJECT,
    TAO_SHARED_ARRAY, TAO_REMOTE_CAMERA, TAO_REMOTE_MIRROR;
extern tao_is_shared_object;
/* DOCUMENT ans = tao_is_shared_object(obj);

     This function yields a non-zero value if `obj` is a TAO shared object.  If
     `obj` is not a TAO shared object, 0 is returned; otherwise, one of the
     following constants is returned:

     ---------------------------------------------------
     Constant             Description
     ---------------------------------------------------
     TAO_SHARED_OBJECT    Basic shared object
     TAO_RWLOCKED_OBJECT  Basic read/write locked object
     TAO_REMOTE_OBJECT    Basic remote object
     TAO_SHARED_ARRAY     Shared array
     TAO_REMOTE_CAMERA    Remote camera
     TAO_REMOTE_MIRROR    Remote deformable mirror
     ---------------------------------------------------

   SEE ALSO: tao_attach_shared_object.
 */

local TAO_STATE_INITIALIZING, TAO_STATE_WAITING, TAO_STATE_CONFIGURING,
    TAO_STATE_STARTING, TAO_STATE_WORKING, TAO_STATE_STOPPING,
    TAO_STATE_ABORTING, TAO_STATE_ERROR, TAO_STATE_RESETTING,
    TAO_STATE_QUITTING, TAO_STATE_UNREACHABLE;
extern tao_get_state_name;
/* DOCUMENT ans = tao_get_state_name(obj);
         or ans = tao_get_state_name(val);

     This function yields the name of the state of the server owning the remote
     object `obj` or corresponding the value `val`.  The following possible
     states exist:

     ----------------------------------------------------------------------
     Constant                   Value  Description
     ----------------------------------------------------------------------
     TAO_STATE_INITIALIZING (*)     0  Server is not yet ready
     TAO_STATE_WAITING              1  Server is waiting for commands
     TAO_STATE_CONFIGURING  (*)     2  Server is configuring the settings
     TAO_STATE_STARTING     (*)     3  Server is starting its work
     TAO_STATE_WORKING              4  Server is working
     TAO_STATE_STOPPING     (*)     5  Server is stopping its work
     TAO_STATE_ABORTING     (*)     6  Server is aborting its work
     TAO_STATE_ERROR                7  Server is in recoverable error state
     TAO_STATE_RESETTING    (*)     8  Server is attempting a reset
     TAO_STATE_QUITTING     (*)     9  Server is about to quit
     TAO_STATE_UNREACHABLE         10  Server is unreachable
     ---------------------------------------------------------------------

     The (*)'s indicate "transitory" states.
 */

local TAO_INT8, TAO_UINT8, TAO_INT16, TAO_UINT16, TAO_INT32, TAO_UINT32,
    TAO_INT64, TAO_UINT64, TAO_FLOAT, TAO_DOUBLE;
func tao_type(id)
/* DOCUMENT T = tao_type(id);

     This function yields the Yorick type matching TAO array element type `id`.
     Signedness of integers is ignored.

     The possible values of `id` are:

         -------------------------------------------------
         Constant   Value  Description
         -------------------------------------------------
         TAO_INT8      1   Signed 8-bit integer
         TAO_UINT8     2   Unsigned 8-bit integer
         TAO_INT16     3   Signed 16-bit integer
         TAO_UINT16    4   Unsigned 16-bit integer
         TAO_INT32     5   Signed 32-bit integer
         TAO_UINT32    6   Unsigned 32-bit integer
         TAO_INT64     7   Signed 64-bit integer
         TAO_UINT64    8   Unsigned 64-bit integer
         TAO_FLOAT     9   Single precision floating-point
         TAO_DOUBLE   10   Double precision floating-point
         -------------------------------------------------

   SEE ALSO: tao_create_shared_array.
 */
{
    if (id == TAO_INT8 || id == TAO_UINT8) {
        n = 1;
    } else if (id == TAO_INT16 || id == TAO_UINT16) {
        n = 2;
    } else if (id == TAO_INT32 || id == TAO_UINT32) {
        n = 4;
    } else if (id == TAO_INT64 || id == TAO_UINT64) {
        n = 8;
    } else if (id == TAO_FLOAT) {
        return float;
    } else if (id == TAO_DOUBLE) {
        return double;
    } else {
        error, "invalid type identifier";
    }
    if (sizeof(long) == n) {
        return long;
    } else if (sizeof(int) == n) {
        return int;
    } else if (sizeof(short) == n) {
        return short;
    } else  if (sizeof(char) == n) {
        return char;
    } else {
        error, swrite(format="no %d-bit integer type", 8*n);
    }
}

extern tao_attach_shared_array;
extern tao_create_shared_array;
/* DOCUMENT arr = tao_create_shared_array(eltype, dims..., perms=);
         or arr = tao_attach_shared_array(shmid);

     The first function creates a new shared array with element type `eltype`
     and dimensions `dims...`.  See `tao_type` for the possible values of the
     `eltype` argument.  Access permissions can be specified with keyword
     `perms`, by default user read and write permissions are granted.

     Examples:

         arr1 = tao_create_shared_array(float, 3,4,5);
         arr2 = tao_create_shared_array(long, 10, perms=0666);

     The second function attaches the shared array whose share memory
     identifier is `shmid` to the address space of the Yorick process and
     returns it.  The returned shared object `arr` is automatically detached
     when no longer referenced by Yorick.

     Shared arrays have the following members:

         arr.data ________ data (as a Yorick array);
         arr.dims ________ dimensions (as with `dimsof`) of shared array;
         arr.eltype ______ type of elements;
         arr.flags _______ access permissions and other flags;
         arr.lock ________ lock state;
         arr.ndims _______ number of dimensions;
         arr.nrefs _______ number of attachments;
         arr.serial ______ serial number;
         arr.shmid _______ shared memory identifier;
         arr.size ________ size (in bytes) of object;
         arr.timestamps __ vector of timestamps in seconds;
         arr.timestampN __ N-th timestamp in seconds;
         arr.type ________ type of object (i.e. `TAO_SHARED_ARRAY`);

     Shared arrays implement the following specific syntaxes:

         arr(i) __________ yields i-th array value as a fast scalar (int, long
                           or double) using Yorick's conventions that i <= 0
                           refers to the end of the range;
         arr() ___________ yields the full array, as arr.data;
         arr(*) __________ yields a the full array flattened as a vector;

   SEE ALSO: tao_attach_shared_object, tao_is_shared_object, tao_rdlock,
             tao_wrlock, tao_type.
 */

local TAO_PREPROCESSING_NONE, TAO_PREPROCESSING_AFFINE, TAO_PREPROCESSING_FULL;
extern tao_attach_remote_camera;
/* DOCUMENT cam = tao_attach_remote_camera(id);

     This function attaches the remote camera identified by `id` to the address
     space of the Yorick process and returns it.  The returned object is
     automatically detached when no longer referenced by Yorick.

     Remote cameras have the following members:

         cam.alive __________ remote server is alive?
         cam.bufferencoding _ encoding of pixels in the acquisition buffers;
         cam.buffers ________ number of acquisition buffers;
         cam.droppedframes __ number of dropped frames;
         cam.exposuretime ___ exposure duration (in seconds);
         cam.flags __________ access permissions and other flags;
         cam.framerate ______ number of frames per second;
         cam.frames _________ number of frames acquired so far;
         cam.height _________ number of lines in acquired images;
         cam.lock ___________ lock state;
         cam.lostframes _____ number of lost frames;
         cam.lostsyncs ______ number of synchronization losts;
         cam.nbufs __________ number of output buffers;
         cam.ncmds __________ number of processed commands;
         cam.nrefs __________ number of attachments;
         cam.overflows ______ number of overflows;
         cam.overruns _______ number of frames lost because of overruns
         cam.owner __________ name of server owning the remote object;
         cam.pixeltype ______ type of elements of processed images;
         cam.preprocessing __ level of preprocessing;
         cam.preprocshmids __ shared memory identifiers for preprocessing;
         cam.roi ____________ region of interest: `[xoff,yoff,width,height]`;
         cam.sensorencoding _ encoding of pixels in the raw captured images;
         cam.sensorheight ___ detector vertical size (in pixels);
         cam.sensorwidth ____ detector horizontal size (in pixels);
         cam.serial _________ number of acquired image;
         cam.shmid __________ shared memory identifier;
         cam.size ___________ size (in bytes) of object;
         cam.state __________ server state;
         cam.timeouts _______ number of timeouts;
         cam.type ___________ type of object (i.e. `TAO_REMOTE_CAMERA`);
         cam.width __________ length of lines in acquired images;
         cam.xbin ___________ horizontal binning (in pixels);
         cam.xoff ___________ horizontal offset of ROI (in pixels);
         cam.ybin ___________ vertical binning (in pixels);
         cam.yoff ___________ vertical offset of ROI (in pixels);

     The possible values of `pixeltype` are the same as those of `eltype` in
     shared arrays (see `tao_create_shared_array`).

     A remote camera instance can be called as function:

         cam() ______________ yields a list of names of named attributes;
         cam(key) ___________ yields the value of an attribute by name;

     See `tao_get_state_name` for the possible values of `state`.

     The possible values of `preprocessing` are:

         -------------------------------------------------------------------
         Constant                Value  Description
         -------------------------------------------------------------------
         TAO_PREPROCESSING_NONE      0  Just convert pixel values.
         TAO_PREPROCESSING_AFFINE    1  Apply affine correction.
         TAO_PREPROCESSING_FULL      2  Apply affine correction and compute
                                        weights.
         -------------------------------------------------------------------

   SEE ALSO: tao_create_shared_object, tao_create_shared_array,
             tao_get_data, tao_wait_image, tao_get_state_name.
 */

extern tao_attach_remote_mirror;
/* DOCUMENT dm = tao_attach_remote_mirror(id);

     Attach to remote mirror server identified by `id` (a shared memory
     identifier or the server name).

     Remote deformable mirrors have the following members:

         dm.alive __________ remote server is alive?
         dm.cmax ___________ maximum possible actuator command value
         dm.cmin ___________ minimum possible actuator command value
         dm.dims ___________ dimensions of actuators grid;
         dm.flags __________ access permissions and other flags;
         dm.layout _________ actuators layout;
         dm.lock ___________ lock state;
         dm.mark ___________ current mark;
         dm.nacts __________ number of actuators;
         dm.nbufs __________ number of output buffers;
         dm.ncmds __________ number of processed commands;
         dm.nrefs __________ number of attachments;
         dm.owner __________ name of server owning the remote object;
         dm.refcmds ________ current reference commands;
         dm.serial _________ serial number of last data frame;
         dm.shmid __________ shared memory identifier;
         dm.size ___________ shared memory size (in bytes);
         dm.state __________ server state;
         dm.type ___________ type of shared object (`TAO_REMOTE_MIRROR`);

     Remote deformable mirrors may be locked for exclusive access by
     `tao_lock`.  This is however not needed for most normal operations.

   SEE ALSO: tao_create_shared_object, tao_lock, tao_reset, tao_kill,
             tao_set_reference, tao_send_commands.
 */

extern tao_attach_remote_sensor;
/* DOCUMENT wfs = tao_attach_remote_sensor(id);

     Attach to remote sensor server identified by `id` (a shared memory
     identifier or the server name).

     Remote wavefront sensors have the following members:

         wfs.alive __________ remote server is alive?
         wfs.dims ___________ dimensions of sub-images grid;
         wfs.flags __________ access permissions and other flags;
         wfs.layout _________ sub-images layout;
         wfs.lock ___________ lock state;
         wfs.nsubs __________ number of sub-images;
         wfs.nbufs __________ number of output buffers;
         wfs.ncmds __________ number of processed commands;
         wfs.nrefs __________ number of attachments;
         wfs.owner __________ name of server owning the remote object;
         wfs.refs ___________ current reference positions;
         wfs.serial _________ serial number of last data frame;
         wfs.shmid __________ shared memory identifier;
         wfs.size ___________ shared memory size (in bytes);
         wfs.state __________ server state;
         wfs.type ___________ type of shared object (`TAO_REMOTE_SENSOR`);

     Remote wavefront sensors may be locked for exclusive access by
     `tao_lock`.  This is however not needed for most normal operations.

   SEE ALSO: tao_create_shared_object, tao_lock, tao_reset, tao_kill,
             tao_set_reference, tao_send_commands.
 */

func tao_get_members(obj, autolock=)
/* DOCUMENT cfg = tao_get_members(obj);

     This function yields a hash table with the members of the shared object
     `obj`.

     If keyword `autolock` is set true, the object is automatically locked (an
     unlocked) while the information is retrieved.

 */
{
    // Yeti is needed for hash-tables.
    if (! is_func(h_new)) include, "yeti.i", 1;

    // Check argument.
    type = tao_is_shared_object(obj);
    if (!type) error, "argument is not a shared object";
    autolock = (autolock && !obj.lock);

    // Extract members according to object type.
    if (type == TAO_REMOTE_CAMERA) {
        if (autolock) tao_lock, obj;
        tab = h_new(
            alive = obj.alive,
            bufferencoding = obj.bufferencoding,
            buffers = obj.buffers,
            droppedframes = obj.droppedframes,
            exposuretime = obj.exposuretime,
            flags = obj.flags,
            framerate = obj.framerate,
            frames = obj.frames,
            height = obj.height,
            lock = obj.lock,
            lostframes = obj.lostframes,
            lostsyncs = obj.lostsyncs,
            nbufs = obj.nbufs,
            ncmds = obj.ncmds,
            nrefs = obj.nrefs,
            overflows = obj.overflows,
            overruns = obj.overruns,
            owner = obj.owner,
            pixeltype = obj.pixeltype,
            preprocessing = obj.preprocessing,
            preprocshmids = obj.preprocshmids,
            roi = obj.roi,
            sensorencoding = obj.sensorencoding,
            sensorheight = obj.sensorheight,
            sensorwidth = obj.sensorwidth,
            serial = obj.serial,
            shmid = obj.shmid,
            size = obj.size,
            state = obj.state,
            timeouts = obj.timeouts,
            type = obj.type,
            width = obj.width,
            xbin = obj.xbin,
            xoff = obj.xoff,
            ybin = obj.ybin,
            yoff = obj.yoff);
        keys = obj();
        nkeys = numberof(keys);
        for (i = 1; i <= nkeys; ++i) {
            key = keys(i);
            h_set, tab, key, obj(key);
        }
        if (autolock) tao_unlock, obj;
    } else  if (type == TAO_REMOTE_MIRROR) {
        if (autolock) tao_lock, obj;
        tab = h_new(
            alive = obj.alive,
            cmax = obj.cmax,
            cmin = obj.cmin,
            dims = obj.dims,
            flags = obj.flags,
            layout = obj.layout,
            lock = obj.lock,
            mark = obj.mark,
            nacts = obj.nacts,
            nbufs = obj.nbufs,
            ncmds = obj.ncmds,
            nrefs = obj.nrefs,
            owner = obj.owner,
            refcmds = obj.refcmds,
            serial = obj.serial,
            shmid = obj.shmid,
            size = obj.size,
            state = obj.state,
            type = obj.type);
        if (autolock) tao_unlock, obj;
    } else  if (type == TAO_SHARED_ARRAY) {
        if (autolock) tao_rdlock, obj;
        tab = h_new(
            data = obj.data,
            dims = obj.dims,
            eltype = obj.eltype,
            flags = obj.flags,
            lock = obj.lock,
            ndims = obj.ndims,
            nrefs = obj.nrefs,
            serial = obj.serial,
            shmid = obj.shmid,
            size = obj.size,
            timestamps = obj.timestamps,
            type = obj.type);
        if (autolock) tao_unlock, obj;
    } else  if (type == TAO_SHARED_OBJECT) {
        if (autolock) tao_lock, obj;
        tab = h_new(
            flags = obj.flags,
            lock = obj.lock,
            nrefs = obj.nrefs,
            shmid = obj.shmid,
            size = obj.size,
            type = obj.type);
        if (autolock) tao_unlock, obj;
    } else  if (type == TAO_RWLOCKED_OBJECT) {
        if (autolock) tao_rdlock, obj;
        tab = h_new(
            flags = obj.flags,
            lock = obj.lock,
            nrefs = obj.nrefs,
            shmid = obj.shmid,
            size = obj.size,
            type = obj.type);
        if (autolock) tao_unlock, obj;
    } else  if (type == TAO_REMOTE_OBJECT) {
        if (autolock) tao_lock, obj;
        tab = h_new(
            alive = obj.alive,
            flags = obj.flags,
            lock = obj.lock,
            nbufs = obj.nbufs,
            ncmds = obj.ncmds,
            nrefs = obj.nrefs,
            owner = obj.owner,
            serial = obj.serial,
            shmid = obj.shmid,
            size = obj.size,
            state = obj.state,
            type = obj.type);
        if (autolock) tao_unlock, obj;
    } else {
        error, "unsupported object type";
    }
    return tab;
}

extern tao_wait_frame;
/* DOCUMENT frm = tao_wait_frame(src, num[, secs]);

     Wait for a frame with serial number `num` to be available from source
     `src` waiting no longer than `secs` seconds.  If `num < 1`, the next frame
     is waited for.  If `secs` is not specified, the call may wait until the
     server quit.

     When called as a function, the returned value is an object storing frame
     data.  The contents of the returned object depend on the type of the
     source `src`.  For a deformable mirror:

         frm.effcmds _____ effective commands;
         frm.mark ________ mark set for the frame;
         frm.nacts _______ number of actuators;
         frm.perturb _____ perturbating commands;
         frm.refcmds _____ reference values;
         frm.reqcmds _____ requested commands;
         frm.serial ______ serial number of the frame;
         frm.time ________ timestamp of the frame;

     The value of `frm.serial` should be checked according to:

     * `frm.serial ≥ 1`, if the requested frame has been retrieved;

     * `frm.serial = 0`, if a timeout occurred;

     * `frm.serial = -1`, if the requested frame is too old (its contents has
       been overwritten or it is beyond the last available frame);

     * `frm.serial = -2`, if the server is no longer running and the requested
       frame is beyond the last available one.

     The requested and effective commands, `frm.reqcmds` and `frm.effcmds`, are
     both relative to the reference and perturbating commands, `frm.refcmds`
     and `frm.perturb`.  The commands applied to the deformable mirror device
     are:

         devcmds = psi(reqcmds + refcmds + perturb);

     where the mapping `psi` models the modifications (clamping, etc.) due to
     the deformable mirror limitations.  To reflect these modifications, the
     actuators effective commands are computed as follows:

         effcmds = devcmds - (refcmds + perturb);

     which relies on the mirror model implemented by `psi`.

     When called as subroutine, no frame object is created and any error
     (including timeouts) is raised.

     The caller must not have locked the source object `src`.

     Typical usage:

         mirror = tao_attach_remote_mirror(mirror_name);
         serial = tao_send_commands(mirror, cmds, secs);
         frame = tao_wait_frame(mirror, serial, secs);
         if (frame.serial < 1) {
             error, (frame.serial == 0 ? "timeout" = -1 :
                     (frame.serial == -1 ? "contents has been overwritten" :
                      "server no longer running"));
         }

   SEE ALSO: tao_attach_remote_mirror, tao_set_reference.
 */

local TAO_MAX_TIME_SECONDS, TAO_WAIT_FOREVER;
/* DOCUMENT TAO_MAX_TIME_SECONDS;
         or TAO_WAIT_FOREVER;

     `TAO_MAX_TIME_SECONDS` is the maximum number of seconds for TAO high
     precision time-stamps.

     `TAO_WAIT_FOREVER` is a number of seconds large enough to make any
     operation with a time limit to effectively wait forever.
*/

local TAO_BAD_SHMID;
extern tao_config_read_shmid;
/* DOCUMENT id = tao_config_read_shmid(name);

     Yield the shared memory identifier of the remote TAO object named `name`.
     The value `TAO_BAD_SHMID` is returned in case of failure (normaly this
     means that no such server exists).

   SEE ALSO: tao_attach_remote_mirror.
*/

extern tao_get_data;
extern tao_set_data;
/* DOCUMENT arr = tao_get_data(obj);
         or tao_set_data, obj, arr;

     The call `tao_get_data(obj)` yields the contents of a TAO shared array
     object `obj` as a Yorick array; this is equivalent to:

         obj.data

     The subroutine `tao_set_data` copies the elements of Yorick array `arr`
     into TAO shared array `obj`.  The two arrays must have the same dimensions
     but may have different element types (they are automatically converted).
     If called as a function, `tao_set_data` returns its first argument (the
     shared object).

   SEE ALSO: tao_create_shared_array, tao_attach_shared_object.
 */

extern tao_get_serial;
extern tao_set_serial;
/* DOCUMENT num = tao_get_serial(obj);
         or tao_set_serial, obj, num;

     The call `tao_get_serial(obj)` yields the value of the serial number of
     the TAO shared object `obj`.  This is equivalent to:

         obj.serial

     Note that the serial number is an atomic variable, it is thus not
     necessary to lock the object to retrieve it.  For an object connected to a
     remote server, the serial number is also the number of data-frames posted
     so far by the server owning the object.

     The subroutine `tao_set_serial` sets the serial number of the TAO shared
     object `obj` to be `num`.  If called as a function, `tao_set_serial` yields
     its first argument (the shared object).  Not all objects implement this
     function.

   SEE ALSO: tao_wait_image, tao_get_data, tao_get_timestamp.
 */

extern tao_get_timestamp;
extern tao_set_timestamp;
/* DOCUMENT tms = tao_get_timestamp(arr, i);
         or tao_set_timestamp, arr, i, tms;

     The call `tao_get_timestamp(arr)` yields the value of the `i`-th
     time-stamp of the shared array `arr`.  Yorick indexing rules apply: `i =
     0` for the last time-stamp, `i = -1` for the before last time-stamp, etc.
     The returned value is the number of seconds elapsed as given by
     `tao_get_monotonic_time` (i.e., since the first call to `_tao_init`).
     This value has nanosecond resolution and may be negative.  This is
     equivalent to:

         arr.timestampN

     with `N` the time-stamp number.

     The subroutine `tao_set_timestamp` sets the value of the time-stamp image
     of the TAO shared array `arr`.  Argument `tms` is either a real -- the
     fractional number of seconds as given by `tao_get_monotonic_time` (i.e.,
     since the first call to `_tao_init`) -- or a vector of two integers -- the
     number of seconds and of nanoseconds since the same time as assumed by
     `tao_get_monotonic_time_origin`.  If called as a function,
     `tao_set_timestamp` yields its first argument (the shared array).

   SEE ALSO: tao_wait_image, tao_get_data, tao_get_serial,
             tao_get_monotonic_time, tao_get_monotonic_time_origin.
 */

extern tao_get_image_shmid;
/* DOCUMENT shmid = tao_get_image_shmid(cam, n);

  This function yields the shared memory identifier of the `n`-th image
  acquired by the remote camera `cam`.  The result may be -1 to indicate an
  error (the serial number is not strictly positive or no image is currently
  stored at the position corresponding to the serial number in the cyclic list
  of images).

*/

if (is_void(tao_wait_image_serial)) tao_wait_image_serial = 0;
if (is_void(tao_wait_image_timestamps))
    tao_wait_image_timestamps = array(0.0, 6);
local tao_wait_image_serial, tao_wait_image_timestamps, tao_wait_image_reason;
func tao_wait_image(cam, serial=, timeout=, noweights=, no_errors=)
/* DOCUMENT arr = tao_wait_image(cam);

     This function waits for the acquisition of a given image by the remote
     camera `cam`.  The acquired image is returned as a Yorick array and,
     unless some error occured (see below), global variables
     `tao_wait_image_serial` and `tao_wait_image_timestamps` are set with the
     serial number and the timestamps of the acquired image.

     Keyword `serial` specifies the serial number of the image to retrieve.  If
     `serial` is less or equal zero or unspecified, the next image is retrieved
     (i.e., as if `serial = cam.serial + 1`).

     Keyword `timeout` specifies the maximum amount of time to wait (in
     seconds).  If the requested image is not available during the allowed
     time, an empty result is returned, otherwise an array is returned with the
     image data.  If `timeout` is nil or unspecified (not recommended), the
     call blocks until the requested image becomes available or the server be
     killed.

     Keyword `noweights` specifies whether to remove the weights if any.

     If a timeout error occured or if the image is overwritten, an error is
     thrown unless keyword `no_errors` is true which causes to return an empty
     result instead.  Global variable `tao_wait_image_reason` is set so as to
     indicate the reason of the failure.  The value of keyword `no_errors` is
     ignored for other errors (which usually indicate a bad usage of the code).

   SEE ALSO: tao_create_shared_array, tao_attach_shared_object, tao_capture.
 */
{
    // Global variables.
    extern tao_wait_image_reason, tao_wait_image_serial,
        tao_wait_image_timestamps;

    // Catch errors to prevent deadlocks.
    local cam, arr;
    if (catch(-1)) {
        tao_ensure_unlocked, cam, arr;
        error, catch_message;
    }

    if (is_void(timeout)) timeout = TAO_WAIT_FOREVER;

    // Get next image identifier.
    if (is_void(serial) || serial <= 0) {
        serial = cam.serial + 1;
    }

    // Wait for acquisition to complete and retrieve the shared memory
    // identifier of the image.
    serial = tao_wait_output(cam, serial, timeout);
    if (serial > 0) {
        tao_lock, cam;
        shmid = tao_get_image_shmid(cam, serial);
        tao_unlock, cam;
    } else {
        shmid = -1;
    }
    if (shmid == -1) {
        if (serial == 0 || serial == -2) {
            // A timeout has occured or the server has been killed.
            tao_wait_image_reason = "timeout occurred";
        } else if (serial == -1 || serial > 0) {
            // Requested image is too old or shared array has been destroyed
            // before we can get it.
            tao_wait_image_reason = "image has been overwritten";
        } else {
            tao_wait_image_reason = "unknown error";
        }
        if (no_errors) {
            return;
        }
        error, tao_wait_image_reason;
    }

    // Retrieve image data.
    arr = tao_attach_shared_array(shmid);
    if (tao_rdlock(arr)) {
        time_recv = tao_get_monotonic_time();
        if (arr.serial == serial) {
            img = arr.data;
            tao_wait_image_serial = serial;
            timestamps = arr.timestamps;
        } else {
            tao_wait_image_reason = "image has been overwritten";
            if (!no_errors) {
                error, tao_wait_image_reason;
            }
            img = [];
        }
        tao_unlock, arr;
        if (!is_void(img)) {
            tao_wait_image_timestamps = grow(timestamps, time_recv);
        }

    }
    if (noweights && !is_void(img) && dimsof(img)(1) > 2) {
        return img(,,1);
    }
    return img;
}

func tao_capture(arg, number, timeout=, statistics=, structured=, eltype=,
                 no_errors=)
/* DOCUMENT res = tao_capture(cam);
         or res = tao_capture(cam, number);

     Capture one or `number` images from remote camera `cam` (which can be a
     remote camera instance, the shared memory identifier of such an instance,
     or the name of the server owning the camera).  If the acquisition is not
     running, acquisition is started (a maximum of 5 seconds is allowed for
     starting acquisition) before capturing image and stopped when done.

     Keyword `timeout` may be used to specify the maximum time (in seconds) to
     wait for an image.

     Keyword `eltype` may be used to enforce the type of the pixel encoding.
     Default is to use the type received by `tao_wait_image` that can promote
     the type to float. The given value may be the result of structof(x), or x
     itself.

     Keyword `statistics` may be set true to get the empirical mean and
     standard deviation of images.

     Keyword `structured` may be set true to get the result in a hash table
     with all information.  Otherwise, the result is an array.  In a structured
     result, the captured data is named "data", the number of images is named
     "samples" and member "contents" is set with "statistics" or "sequence"
     (depending whether empirical mean and standard deviation have been
     computed), all other information are named as for a TAO remote camera
     object.

     If an event prevents tao_capture to acquire images, an error is issued
     unless keyword `no_errors` is set to true. In this case, an empty result
     is returned, and the global variable `tao_error_reason` is set so as to
     indicate the reason of the failure.

     Global variable `tao_capture_overwritten" counts the number of
     overwritten images while acquiring.

   SEE ALSO: tao_wait_image, tao_start, tao_save_fits.
 */
{
    // Global variable.
    extern tao_error_message, tao_capture_overwritten1,
        tao_capture_overwritten2;

    // Check arguments and options.
    if (is_void(number)) number = 1;
    if (statistics && number < 2) {
        error, "too few images for computing statistics";
    }
    if (!is_void(eltype)) {
        if (is_array(eltype)) eltype = structof(eltype);
        if (identof(eltype) != Y_STRUCTDEF) {
            error, "keyword `eltype` must give a structure definition";
        }
    }
    if (is_void(timeout)) timeout = TAO_WAIT_FOREVER;

    // Catch errors to prevent deadlocks.
    local cam, arr;
    if (catch(-1)) {
        tao_ensure_unlocked, cam, arr;
        error, catch_message;
    }
    has_error_message = 0n;

    // Connect to the remote camera.
    if (is_scalar(arg) && (is_string(arg) || is_integer(arg))) {
        cam = tao_attach_remote_camera(arg);
    } else {
        eq_nocopy, cam, arg;
    }
    if (tao_is_shared_object(cam) != TAO_REMOTE_CAMERA) {
        error, "invalid remote camera argument";
    }

    // Make sure acquisition is running.  No needs to lock the camera as its
    // state is an atomic variable.
    initial_state = state = cam.state;
    if (state == TAO_STATE_QUITTING || state == TAO_STATE_UNREACHABLE) {
        error, "server has been killed";
    }
    stopped = (state != TAO_STATE_STARTING && state != TAO_STATE_WORKING);
    if (stopped) {
        num = tao_start(cam, 1.0);
        if (!tao_wait_command(cam, num, 5.0)) {
            error, "timeout occurred while starting acquisition";
        }
        state = cam.state;
        if (state != TAO_STATE_STARTING && state != TAO_STATE_WORKING) {
            error, "failed to start acquisition";
        }
    }

    // Run acquisition.
    // FIXME: Use in-line statistics to avoid loss of precision.
    if (statistics) {
        sum1 = 0.0;
        sum2 = 0.0;
        minval = [];
        maxval = [];
    } else {
        data = [];
    }
    nimgs = 0;                // number of collected images
    serial = 0;               // want to start with last acquired image
    tao_capture_overwritten1 = 0;
    tao_capture_overwritten2 = 0;
    while (nimgs < number) {
        serial = max(serial + 1, cam.serial); // want last or next image

        // Wait for acquisition to complete and retrieve the shared memory
        // identifier of the image.
        ans = tao_wait_output(cam, serial, timeout);
        if (ans > 0) {
            tao_lock, cam;
            // shmid = -1 if no image is stored at the position serial in the
            // cyclic list of images.
            shmid = tao_get_image_shmid(cam, serial);
            tao_unlock, cam;
        } else {
            shmid = -1;
        }
        // Deal with errors.
        if (shmid == -1) {
            if (ans == 0 || ans == -2) {
                // A timeout has occured or the server has been killed.
                tao_error_message = "timeout occurred";
            } else if (ans == -1 || ans > 0) {
                // Requested image is too old or shared array has been
                // destroyed before we can get it. Just count.
                tao_capture_overwritten1 += 1;
                continue;
            } else {
                tao_error_message = "unknown error";
            }
            has_error_message = 1n;
            break;
        }

        // Retrieve image data.
        arr = tao_attach_shared_array(shmid);
        ndims = arr.ndims;
        if (!tao_rdlock(arr, timeout)) {
            tao_error_message = "timeout occurred";
            has_error_message = 1n;
            break;
        }
        if (arr.serial == serial) {
            img = arr.data;
        } else {
            // Requested image has been overwritten. Just count.
            tao_capture_overwritten2 += 1;
            img = [];
        }
        tao_unlock, arr;
        if (is_void(img)) {
            continue;
        }
        // One more image has been acquired.
        ++nimgs;
        if (ndims > 2) {
            // Discard weights.
            img = img(,,1);
        }
        if (statistics) {
            img = double(img);
            sum1 += img;
            sum2 += img*img;
            if (structured) {
                minval = (is_void(minval) ? min(img) : min(min(img), minval));
                maxval = (is_void(maxval) ? max(img) : max(max(img), maxval));
            }
        } else {
            if (!is_void(eltype)) {
              img = eltype(img);
            }
            if (number == 1) {
                eq_nocopy, data, img;
            } else {
                if (nimgs == 1) {
                    data = array(structof(img), dimsof(img), number);
                }
                data(.., nimgs) = img;
            }
        }
    }
    if (initial_state == TAO_STATE_WAITING && cam.state == TAO_STATE_WORKING) {
        tao_stop, cam;
    }
    if (has_error_message) {
        if (no_errors) {
            return;
        }
        error, tao_error_message;
    }

    // If requested, compute empirical mean and standard deviation.
    if (statistics) {
        data = array(double, dimsof(sum1), 2);
        mean = (1.0/nimgs)*sum1; // empirical mean
        var = (1.0/(nimgs - 1))*(sum2 - mean*sum1); // empirical variance
        data(.., 1) = mean;
        data(.., 2) = sqrt(max(var, 0.0));
        sum1 = sum2 = a = v = []; // free memory
    }

    // If requested, store the data in a hash-table with all information.
    if (structured) {
        data = h_set(
            tao_get_members(cam),
            data = data,
            samples = long(number),
            contents = (statistics ? "statistics" : "sequence"));
    }
    return data;
}

func tao_save_fits(src, dest, overwrite=, hook=, bitpix=,
                   comment=, history=, extname=, hduname=)
/* DOCUMENT tao_save_fits, src, dest;
         or fh = tao_save_fits(img, dest);

     Save data specified by SRC in FITS file DEST (can be a file name or a
     FITS handle).  Source data SRC can be a structured object as returned by
     `tao_capture` or an array of pixel values.  When called as a function,
     the FITS handle is returned and can be used, for instance, to append more
     FITS extensions.

     Keywords COMMENT and HISTORY can be set with an array of strings to
     specify comments and history records.

     For more flexibility with the contents of the FITS header, keyword HOOK
     may be set with a callable object called as:

         hook, fh;

     with FH the FITS handle and in order to add custom FITS keywords to the
     header before writing the data.

     If DEST is a string, keyword OVERWRITE can be set true to allow for
     overwriting file DEST if it already exists.

     Keyword EXTNAME can be used to specify the name of the FITS extension.

     Keyword HDUNAME can be used to specify the name of the FITS HDU.

     Keyword BITPIX can be used to force a specific binary format of array
     values written in the file.  For instance, BITPIX=-32 to force writing
     the array data as single precision floating point values.

   SEE ALSO: fits, tao_capture.
 */
{
    if (! is_func(is_hash)) include, "yeti.i", 1;
    local arr;
    eq_nocopy, arr, (is_hash(src) ? src.data : src);
    if (! is_array(arr)) {
        error, "invalid source argument";
    }
    dims = dimsof(arr);
    ndims = dims(1);
    width  = (ndims >= 1 ? dims(2) : 1);
    height = (ndims >= 2 ? dims(3) : 1);
    depth  = (ndims >= 3 ? dims(4) : 1);

    if (is_hash(src)) {
        // Check consistency of dimensions.
        contents = src.contents;
        if (contents == "statistics") {
            statistics = 1n;
            if (ndims != 3 || depth != 2 || src.samples < 2) {
                error, "unexpected dimensions for statistics";
            }
        } else {
            if (ndims < 2 || ndims > 3 || depth != src.samples) {
                error, "unexpected dimensions for image sequence";
            }
        }
        if (width != src.width) {
            error, "inconsistent image width";
        }
        if (height != src.height) {
            error, "inconsistent image height";
        }
    } else {
        contents = "array";
    }

    // Determine the "bits per pixel" parameter if not specified.
    if (is_void(bitpix)) {
        type = structof(arr);
        if (type == char || type == short || type == int || type == long) {
            bitpix = 8*sizeof(type);
        } else if (type == float || type == double) {
            bitpix = -8*sizeof(type);
        } else {
            error, "invalid array element type";
        }
    }

    // Get FITS handle and instantiate header.
    if (is_string(dest)) {
        fh = fits_open(dest, 'w', overwrite=overwrite);
    } else {
        eq_nocopy, fh, dest;
        fits_new_hdu, fh, "IMAGE", "Image extension";
    }
    hdu = fits_current_hdu(fh);
    if (hdu == 1) {
        fits_set, fh, "SIMPLE", 'T', "True FITS file";
    }
    fits_set, fh, "BITPIX", bitpix, "Bits per pixel";
    fits_set, fh, "NAXIS",  ndims,  "Number of dimensions";
    for (d = 1; d <= ndims; ++d) {
        sfx = swrite(format="%d", d);
        fits_set, fh, "NAXIS"+sfx, dims(d+1), "Number of elements along axis";
    }
    fits_set, fh, "EXTEND", 'T', "This file may contain FITS extensions";
    if (hdu > 1 && ! is_void(extname)) {
        fits_set, fh, "EXTNAME", extname, "Name of this HDU";
    }
    if (! is_void(hduname)) {
        fits_set, fh, "HDUNAME", hduname, "Name of this HDU";
    }
    if (! is_void(comment)) {
        for (k = 1; k <= numberof(comment); ++k) {
            fits_set, fh, "COMMENT", comment(k);
        }
    }
    if (! is_void(history)) {
        for (k = 1; k <= numberof(history); ++k) {
            fits_set, fh, "HISTORY", history(k);
        }
    }
    if (is_hash(src)) {
        if (is_integer(arr) && bitpix > 8) {
            // Output from cameras are unsigned, but the arr is signed.
            offset = 1<<(bitpix-1);
            type = structof(arr);
            arr -= type(offset);
            fits_set, fh, "BZERO", offset, "Offset for unsigned integers";
            fits_set, fh, "BSCALE", 1, "Data are not scaled";
            /* ------
             * This should be the exact formulation, but seems not standard.
            fits_set, fh, "TZERO", 1<<(bitpix-1),
                "Offset for unsigned integers";
            fits_set, fh, "TSCAL", 1, "Data are not scaled";
               ------ */
        }
        xbin = src.xbin;
        ybin = src.ybin;
        if (! is_void(xbin) && ! is_void(ybin)) {
            fits_set, fh, "XBIN", xbin,
                "Pixel binning factor along first axis";
            fits_set, fh, "YBIN", ybin,
                "Pixel binning factor along second axis";
        }
        xoff = src.xoff;
        yoff = src.yoff;
        if (! is_void(xoff) && ! is_void(yoff)) {
            fits_set, fh, "XOFFSET", xoff,
                "Offset of region of interest along first axis";
            fits_set, fh, "YOFFSET", yoff,
                "Offset of region of interest along second axis";
        }
        //fits_set, fh, "DEPTH", src.pixeltype, "Bits per pixel";
        gain = src.gain;
        if (! is_void(gain)) {
            fits_set, fh, "GAIN", gain, "Detector gain";
        }
        bias = src.bias;
        if (! is_void(bias)) {
            fits_set, fh, "BIAS", bias, "Detector bias or black-level";
        }
        exposuretime = src.exposuretime;
        if (! is_void(exposuretime)) {
            fits_set, fh, "EXPOSURE", exposuretime, "Exposure time [seconds]";
        }
        framerate = src.framerate;
        if (! is_void(framerate)) {
            fits_set, fh, "RATE", framerate, "Frames per second [Hz]";
        }
        sensorencoding = src.sensorencoding;
        if (! is_void(sensorencoding)) {
            fits_set, fh, "ENCODING", int(sensorencoding),
                "Bitwise combination of information on detector";
        }
        sensorwidth = src.sensorwidth;
        sensorheight = src.sensorheight;
        if (! is_void(sensorwidth) && ! is_void(sensorheight)) {
            fits_set, fh, "SSWIDTH", sensorwidth,
                "Detector number of pixels along first axis";
            fits_set, fh, "SSHEIGHT", sensorheight,
                "Detector number of pixels along second axis";
        }
        if (contents == "statistics") {
            samples = src.samples;
            if (! is_void(samples)) {
                fits_set, fh, "SAMPLES", samples, "Number of averaged images";
            }
            minval = src.minval;
            if (! is_void(minval)) {
                fits_set, fh, "MINVAL", minval, "Minimum value seen in images";
            }
            maxval = src.maxval;
            if (! is_void(maxval)) {
                fits_set, fh, "MAXVAL", maxval, "Maximum value seen in images";
            }
        }
    }

    // Call the hook, if any, to edit the FITS header.
    if (! is_void(hook)) {
        hook, fh;
    }

    // Write the header, the data and pad the HDU.
    fits_write_header, fh;
    fits_write_array, fh, arr, rescale=0;
    fits_pad_hdu, fh;

    // Return the FITS handle.
    return fh;
}

extern tao_lock;
extern tao_rdlock;
extern tao_wrlock;
extern tao_unlock;
extern tao_ensure_unlocked;
extern tao_is_locked;
/* DOCUMENT tao_lock, obj;
         or tao_rdlock, obj;
         or tao_wrlock, obj;
         or ans = tao_lock(obj, secs);
         or ans = tao_rdlock(obj, secs);
         or ans = tao_wrlock(obj, secs);
         or tao_unlock, obj;
         or tao_ensure_unlocked, obj, ...;
         or tao_is_locked(obj);
         or obj.lock;

     The functions `tao_lock`, `tao_rdlock`, and `tao_wrlock` lock object `obj`
     for exclusive, read-only, and read-write access respectively.  If the
     object is locked by another process and no other argument is specified,
     the call will block until the object is unlocked.  If argument `secs` is
     specified and non-nil, the call will not block longer than that number of
     seconds.  The returned value `ans` is the same as that would be given by
     `tao_is_locked(obj)`: 0 if the object is not locked by the caller, 1 if it
     is locked only for reading and 2 if it is locked for reading and writing.

     The function `tao_unlock` unlocks object `obj` which has been locked
     by the caller.

     The function `tao_ensure_unlocked` unlocks any number of TAO shared
     objects if they are locked.  Nothing is done for an argument which is
     not a TAO shared object or which is not locked.

     The call `tao_is_locked(obj)` yields 0 if `obj` is not a TAO shared object
     or if the object is not locked by the caller, 1 if the object is locked
     only for reading and 2 if it is locked for reading and writing.  If `obj`
     is a TAO shared object, the call `obj.lock` yields the same result as
     `tao_is_locked(obj)`.

     The object is automatically unlocked and detached when it is no longer
     referenced by Yorick.

     Deadlocks are avoided (an error is thrown if the caller tries to lock an
     object already locked by him/her or to unlock an object not locked by
     him/her).

   SEE ALSO: tao_attach_shared_object.
 */

extern tao_configure;
extern tao_start;
extern tao_stop;
extern tao_abort;
extern tao_reset;
extern tao_kill;
/* DOCUMENT ans = tao_configure(obj [, secs], key1=val1, key2=val2, ...);
         or ans = tao_stop(obj [, secs]);
         or ans = tao_abort(obj [, secs]);
         or ans = tao_reset(obj [, secs]);
         or ans = tao_kill(obj [, secs]);

     These functions send a simple command to the server owning the remote
     object `obj` not waiting more than `secs` seconds if this argument is
     specified.  Not all remote objects implement all these commands.

     The caller must not have locked the remote object `obj`.

     The returned value is a strictly positive integer which is the serial
     number of the command or 0 if the command could not be sent before the
     time limit.  The function `tao_wait_command` can be called to ensure that
     the command has been processed by the server and, possibly, to check that
     the command was effective.

     The `tao_configure` command is to change the settings of the remote object
     `obj`.  All settings are specified by keywords whose names match those of
     the members of `obj`.  Configuring a remote object may only be possible
     when the server owning the object is idle.  For example:

         cam = tao_attach_remote_camera("Andor0");
         tao_configure, cam, pixeltype=float, preprocessing=2;

   SEE ALSO: tao_wait_command.
 */

extern tao_send_commands;
extern tao_set_reference;
extern tao_set_perturbation;
/* DOCUMENT ans = tao_send_commands(dm, reqcmds[, secs[, mark]]);
         or ans = tao_set_reference(dm, refcmds[, secs]);
         or ans = tao_set_perturbation(dm, perturb[, secs]);

     The function `tao_send_commands` sends actuators commands `reqcmds` to the
     remote deformable mirror `dm`.  Optional argument `mark` is to set the
     mark of the resulting data-frame in the deformable mirror telemetry, if
     not specified, `obj.mark+1` is assumed.

     The function `tao_set_reference` sets the reference values `refcmds` for
     the actuators commands sent to the remote mirror `dm`.  The reference
     values will be effective until other reference values are set.

     The function `tao_set_perturbation` sets perturbation values `perturb` for
     the actuators commands sent to the remote mirror `dm`.  The perturbation
     will only be effective for the next actuators commands sent to the
     deformable mirror.  I any perturbation has been set, it is overwritten by
     the one.  The pertubation is cleared after sending the next actuators
     commands to the deformable mirrors

     These functions wait no longer than `secs` seconds for the server to be
     ready to accept the request.  If `secs` is not specified, they may wait
     forever.

     The caller of these functions must not have locked the remote deformable
     mirror `dm`.

     The returned value is a a pair of serial numbers `ans = [cmdnum,datnum]`
     where `cmdnum` is the serial number of the request while `datnum` is the
     serial number of the first output data-frame that will be affected by the
     request.  The function `tao_wait_command` can be called as:

         tao_wait_command(dm, ans);
         tao_wait_command(dm, ans(1));
         tao_wait_command(dm, ans.first);

     to ensure that the request numbered `cmdnum` has been processed by the
     server.  The function `tao_wait_output` can be called as:

         tao_wait_output(dm, ans);
         tao_wait_output(dm, ans(2));
         tao_wait_output(dm, ans.second);

     to ensure that the output data-frame `datnum` has been published.  These
     two functions take an optional third argument to specify a maximum amount
     of time to wait.

   SEE ALSO: tao_kill, tao_attach_remote_mirror, tao_wait_command,
             tao_wait_output.
 */

extern tao_wait_command;
extern tao_wait_output;
/* DOCUMENT tao_wait_command, obj, n[, secs];
         or ok = tao_wait_command(obj, n[, secs]);
         or num = tao_wait_output(obj, n[, secs]);

     The function `tao_wait_command` waits for a given command to have been
     processed by the server owning the remote object `obj`.  Argument `n` is
     the serial number of the command to wait for, it can also be the pair of
     serial numbers returned by some commands like `tao_send_commands`.  When
     called as subroutine, errors and timeouts are raised.  When called as a
     function, the returned value is `ok = 1` on success, or `ok = 0` if a
     timeout occurred.  Other errors are raised.

     The function `tao_wait_output` waits for a given output data frame to be
     delivered by the remote object `obj`.  Argument `n` specifies which output
     data frame to wait for.  If `n` is an integer, then if `n ≥ 1`, the
     function waits for the `n`-th output data frame; otherwise (i.e., `n ≤
     0`), the function waits for the next output data frame (as if `n =
     obj.serial + 1`).  Argument `n` may also be the pair of serial numbers
     returned by some commands like `tao_send_commands`.
     The returned value is an integer `num`.  If `num > 0`, `num` is the serial
     number of the requested data frame; `num = 0` if the requested data frame
     is not available before the time limit (i.e. timeout), `num = -1` if the
     requested data frame is too old (it has been overwritten by some newer
     contents or it is beyond the last available data frame), or `num = -2` if
     the server has been killed and the requested data frame is beyond the last
     available one.

     These functions wait no longer than `secs` seconds if this argument is
     specified; otherwise, they may wait forever.

     The caller of these functions must not have locked the remote object
     `obj`.

  EXAMPLE:
     The function `tao_get_image_shmid` can be called to retrieve the shared
     memory identifier of a given output image.

     See the code of `tao_wait_image` for an example of usage.

   SEE ALSO: tao_send_commands, tao_kill, tao_wait_output.
*/

extern tao_get_current_time;
/* DOCUMENT t = tao_get_current_time();

     This function yields the time since the Epoch (that is 00:00:00 UTC, 1st
     of January 1970) with nanosecond resolution.  The returned value is a pair
     of integers: `t(1)` is a number of seconds and `t(2)` is a number of
     nanoseconds.

     In principle, this function takes a single nil argument; for efficiency
     reasons, any given arguments are ignored.

   SEE ALSO: tao_get_monotonic_time.
 */

extern tao_get_monotonic_time;
extern tao_get_monotonic_time_origin;
/* DOCUMENT secs = tao_get_monotonic_time();
         or orig = tao_get_monotonic_time_origin();

     The function tao_get_monotonic_time yields the time elapsed since
     initialization of the plugin (i.e., the first call to _tao_init).  The
     returned value is in seconds with nanosecond resolution.

     The function tao_get_monotonic_time_origin returns the origin of the
     elapsed time given by tao_get_monotonic_time.  The result is the time of
     the first call to _tao_init since some unspecified starting point but
     which is not affected by discontinuous jumps in the system time (e.g., if
     the system administrator manually changes the clock), but is affected by
     the incremental adjustments performed by adjtime() and NTP.  The returned
     value is a pair of integers: `orig(1)` is a number of seconds and
     `orig(2)` is a number of nanoseconds.

     All time-stamps in TAO are measured since this origin.

   SEE ALSO: tao_get_current_time, _tao_init.
 */

extern tao_sleep;
/* DOCUMENT tao_sleep, secs;

     Sleep for a specified high-resolution number of seconds.  This subroutine
     causes the calling thread to sleep either until the number of specified
     seconds have elapsed or until a signal arrives which is not ignored.

     If you want that Yorick's events be processed, calling `pause` is probably
     better.  However, `pause` has a resolution of only one millisecond.

   SEE ALSO: pause.
*/

extern tao_debug;
/* DOCUMENT curval = tao_debug();
         or oldval = tao_debug(newval);

      Query or set debug mode state.  `curval` is the current value of the
      debug mode, `newval` is its new value, and `oldval` is its value prior to
      the change.

      If debug mode is false, certain warning messages are not printed.

   SEE ALSO: tao_attach_shared_object.
 */

extern _tao_init;
/* DOCUMENT _tao_init;

      Initializes constants of TAO plugin.  Can be called again to restore
      them.

   SEE ALSO: tao_attach_shared_object.
 */
_tao_init;
