/*
 * tao-simcam.i --
 *
 * Simple remote camera simulator in Yorick for TAO.
 *
 *-----------------------------------------------------------------------------
 *
 * This file is part of the TAO real-time software licensed under the MIT
 * license (https://git-cral.univ-lyon1.fr/tao/tao-rt-yorick).
 *
 * Copyright (C) 2020-2021, Éric Thiébaut.
 */

/*---------------------------------------------------------------------------*/
/* CLIENT PART */

func tao_simcam_connect(owner, config)
/* DOCUMENT cam = tao_simcam_connect(owner [, config]);

     Connect to camera simulator owned by server OWNER and return shared object
     CAM to manage the client connection.  Open argument CONFIG is the name of
     the camera configuration file; if unspecified, it is given by:

         tao_simcam_default_config(owner)

   SEE ALSO: tao_simcam_default_config, tao_simcam_create.
 */
{
    if (is_void(config)) {
        config = tao_simcam_default_config(owner);
    }
    tail = string(0);
    cam_owner = string(0);
    cam_shmid = int(0);

    file = open(config, "r");
    if (sread(rdline(file), format=" owner: %s %s", cam_owner, tail) != 1 ||
        sread(rdline(file), format=" shmid: %d %s", cam_shmid, tail) != 1) {
        error, "invalid file format";
    }
    close, file;
    if (cam_owner != owner) {
        error, "not same owner";
    }
    return tao_attach_shared_array(cam_shmid);
}

local tao_simcam_counter, tao_simcam_timestamp;
local tao_simcam_get_next_image, tao_get_last_image;
/* DOCUMENT arr = tao_simcam_get_last_image(cam);
         or arr = tao_simcam_get_next_image(cam);

     Retrieve last or next acquired image from remote simulated camera `cam`.

     Keyword `timeout` is the limit of the time to wait for the image, 30
     seconds by default.  An empty result is returned in case of time-out.

     Keyword `noweights` can be set true to only get the image data, not its
     associated weights.

     Global variables `tao_simcam_counter` and `tao_simcam_timestamp` will be
     set with the counter and time-stamp of the returned image.

   SEE ALSO: tao_simcam_connect.
*/
func tao_simcam_get_last_image(cam, timeout=, noweights=)
{
    return _tao_simcam_get_image(cam, _TAO_SIM_LAST, timeout, noweights);
}

func tao_simcam_get_next_image(cam, timeout=, noweights=)
{
    return _tao_simcam_get_image(cam, _TAO_SIM_NEXT, timeout, noweights);
}

func _tao_simcam_get_image(cam, which, timeout, noweights)
{
    extern tao_simcam_counter, tao_simcam_timestamp;

    // Initialization.
    if (is_void(timeout)) {
        timeout = 30.0;
    }
    tao_ensure_unlocked, cam;

    // Catch errors to prevent deadlocks.
    local cam, buf;
    if (catch(-1)) {
        tao_ensure_unlocked, cam, buf;
        error, catch_message;
    }

    // Lock camera for writing, write the command and unlock.
    if (tao_rdlock(cam, timeout)) {
        shmid = cam(which);
        tao_unlock, cam;
        if (shmid > 0) {
            buf = tao_attach_shared_array(shmid);
            if (tao_rdlock(buf, timeout)) {
                tao_simcam_counter = buf.counter;
                tao_simcam_timestamp = buf.timestamp;
                arr = buf();
                tao_unlock, buf;
                return (noweights ? arr(,,1) : arr);
            }
        }
    }
    tao_simcam_counter = 0;
    tao_simcam_timestamp = 0.0;
    return;
}

/* Indices in camera shared data array. */
_TAO_SIM_STATE   = 1;
_TAO_SIM_COMMAND = 2;
_TAO_SIM_COUNTER = 3;
_TAO_SIM_LAST    = 4;
_TAO_SIM_NEXT    = 5;

/* Commands. */
_TAO_SIM_START = 1;
_TAO_SIM_STOP  = 2;
_TAO_SIM_QUIT  = 3;

local tao_simcam_start, tao_simcam_stop, tao_simcam_quit;
/* DOCUMENT tao_simcam_start(cam);
         or tao_simcam_stop(cam);
         or tao_simcam_quit(cam);

     Send a command to the remote camera simulator `cam`.  Call
     `tao_simcam_start` or `tao_simcam_stop` to start or stop acquisition.
     Call `tao_simcam_quit` to make the camera server exit its event loop.
     When called as functions, a boolean result is returned indicating whether
     the command was successfully sent.  When called as subroutines, an error
     is thrown if the command cannot be sent.

     After quitting, no other commands can be scheduled until the server is
     relaunched.

   SEE ALSO: tao_simcam_connect, tao_simcam_get_last_image, tao_simcam_run.
*/
func tao_simcam_start(cam, timeout)
{
    return _tao_simcam_send_command(cam, _TAO_SIM_START, timeout);
}

func tao_simcam_stop(cam, timeout)
{
    return _tao_simcam_send_command(cam, _TAO_SIM_STOP, timeout);
}

func tao_simcam_quit(cam, timeout)
{
    return _tao_simcam_send_command(cam, _TAO_SIM_QUIT, timeout);
}

func _tao_simcam_send_command(cam, cmd, timeout)
{
    // Initialization.
    if (is_void(timeout)) {
        timeout = 30.0;
    }
    tao_ensure_unlocked, cam;

    // Catch errors to prevent deadlocks.
    if (catch(-1)) {
        tao_ensure_unlocked, cam;
        error, catch_message;
    }

    // Lock camera for writing, write the command and unlock.
    status = tao_wrlock(cam, timeout);
    if (! status) {
        if (am_subroutine()) {
            error, "time-out!";
        }
        return status;
    }
    attrs = cam();
    if (attrs(_TAO_SIM_COMMAND) == _TAO_SIM_QUIT) {
        status = !status;
    } else {
        attrs(_TAO_SIM_COMMAND) = cmd;
        tao_set_data, cam, attrs;
    }
    tao_unlock, cam;
    if (! status) {
        if (am_subroutine()) {
            error, "camera has been closed";
        }
    }
    return status;
}

func tao_simcam_test(cam, number=, timeout=)
{
    if (is_string(cam)) {
        cam = tao_simcam_connect(cam);
    }
    if (is_void(number)) {
        number = 1000;
    }
    lag_min = 0.0;
    lag_max = 0.0;
    lag_sum = 0.0;
    previous = 0;
    nlosts = 0;
    nreads = 0;
    tao_simcam_start, cam;
    while (nreads < number) {
        tao_ensure_unlocked, cam;
        arr = tao_simcam_get_next_image(cam, timeout=timeout);
        received = tao_get_monotonic_time();
        if (is_void(arr)) {
            tao_warn, "time-out while waiting for next image";
            break;
        }
        ++nreads;
        sent = tao_simcam_timestamp;
        counter = tao_simcam_counter;
        lag = received - sent;
        if (nreads == 1) {
            lag_min = lag;
            lag_max = lag;
            lag_sum = lag;
            lag_sum2 = lag*lag;
        } else {
            lag_min = min(lag, lag_min);
            lag_max = max(lag, lag_max);
            lag_sum += lag;
            lag_sum2 += lag*lag;
            nlosts += max(counter - (previous + 1), 0);
        }
        previous = counter;
    }
    write, format="Images received: %d\n", nreads;
    write, format="Images lost:     %d\n", nlosts;
    if (nreads >= 1) {
        write, format="Minimum lag: %.3f µs\n", lag_min*1e6;
        write, format="Maximum lag: %.3f µs\n", lag_max*1e6;
        lag_avg = lag_sum/nreads;
        if (nreads < 2) {
            write, format="Average lag: %.3f µs\n", lag_avg*1e6;
        } else {
            lag_std = sqrt((lag_sum2 -  lag_avg*lag_sum)/(nreads - 1));
            write, format="Average lag: %.3f ± %.3f µs\n",
                lag_avg*1e6, lag_std*1e6;
        }
    }
    tao_simcam_stop, cam;
}

/*---------------------------------------------------------------------------*/
/* SERVER PART */

func tao_simcam_create(owner, config=, perms=)
/* DOCUMENT cam = tao_simcam_create(owner);

     Create a new camera simulator.

     Keyword `config` can be used to specify the path to another configuration
     file than the default (see `tao_simcam_default_config`).

     Keyword `perms` can be used to specify the access granted for clients.
     The default, `perms=0006`, only grants access for the user who creates the
     server.

   SEE ALSO: tao_simcam_create.
 */
{
    if (is_void(perms)) {
        perms = 0006;
    }
    if (is_void(config)) {
        config = tao_simcam_default_config(owner);
    }
    cam = tao_create_shared_array(owner, long, 5, perms=perms);
    _tao_simcam_initialize, cam;
    write, open(config, "w"), format="owner: %s\nident: %d\n", owner, cam.shmid;
    return cam;
}

func _tao_simcam_initialize(cam)
{
    attrs = cam();
    attrs(_TAO_SIM_STATE)   = TAO_STATE_WAITING;
    attrs(_TAO_SIM_COMMAND) = 0;
    attrs(_TAO_SIM_COUNTER) = 0;
    attrs(_TAO_SIM_LAST)    = -1;
    attrs(_TAO_SIM_NEXT)    = -1;
    tao_set_data, cam, attrs;
}

func tao_simcam_run(cam, data, fps=, nbufs=, perms=, timeout=, debug=)
/* DOCUMENT tao_simcam_run, cam, data;

     Run simulator camera CAM to cyclically provide images in DATA.

     Keyword `fps` can be used to specify the number of frames per seconds
     (default, 100Hz).

     Keyword `nbufs` can be used to specify the number acquisition buufers
     (default, 5).

     Keyword `perms` can be used to specify the access granted for acquired
     images.  The default, `perms=0006`, only grants access for the user who
     creates the server.

     Keyword `timeout` can be used to specify the maximum time (in seconds) to
     wait for locking resources (default, 30 seconds).

     Keyword `debug` can be set true to avoid catching of errors.

   SEE ALSO: tao_simcam_create.
 */
{
    // Check arguments.
    if (tao_is_shared_object(cam) != TAO_SHARED_ARRAY) {
        error, "expecting shared (fake) camera";
    }
    owner = cam.owner;
    dims = dimsof(data);
    if (dims(1) != 3) {
        error, "expecting a cube of images";
    }
    width = dims(2);
    height = dims(3);
    nimgs = dims(4);

    // Parse options.
    if (is_void(perms)) {
        perms = 0006;
    }
    if (is_void(nbufs)) {
        nbufs = 5;
    }
    if (is_void(timeout)) {
        timeout = 30.0;
    }
    if (is_void(fps)) {
        fps = 100.0;
    }

    // Prepare buffer names and hash table of acquisition buffers.
    keys = swrite(format="buf%d", indgen(nbufs));
    bufs = h_new();
    T = double;

    // Catch errors to prevent deadlocks.
    if (!debug) {
        local cam, buf;
        if (catch(-1)) {
            tao_ensure_unlocked, cam, buf;
            error, catch_message;
        }
    }

    // Initialize camera attributes.
    tao_ensure_unlocked, cam;
    if (!tao_wrlock(cam, timeout)) {
        tao_warn, "cannot lock camera for writing";
    }
    _tao_simcam_initialize, cam;
    tao_unlock, cam;

    // Enter event loop.
    true = 1n;
    false = 0n;
    buf = [];
    posted = false;
    acquiring = false;
    quitting = false;
    ticks = 0; // clock ticks in number of frames
    idx = 0; // buffer index in cyclic list
    initial = tao_get_monotonic_time();
    while (1n) {
        // Lock camera to update next/last frame information and process
        // commands.
        if (!tao_wrlock(cam, timeout)) {
            tao_warn, "cannot lock camera for writing";
            break;
        }
        attrs = cam();
        cmd = attrs(_TAO_SIM_COMMAND);
        if (cmd != 0) {
            if (cmd == _TAO_SIM_START) {
                attrs(_TAO_SIM_STATE) = TAO_STATE_WORKING;
                attrs(_TAO_SIM_COMMAND) = 0;
                acquiring = true;
            } else if (cmd == _TAO_SIM_STOP) {
                attrs(_TAO_SIM_STATE) = TAO_STATE_WAITING;
                attrs(_TAO_SIM_COMMAND) = 0;
                acquiring = false;
            } else if (cmd == _TAO_SIM_QUIT) {
                attrs(_TAO_SIM_STATE) = TAO_STATE_KILLED;
                attrs(_TAO_SIM_COUNTER) = 0;
                attrs(_TAO_SIM_LAST) = -1;
                attrs(_TAO_SIM_NEXT) = -1;
                acquiring = false;
                quitting = true;
            } else {
                tao_warn, "unknown command";
            }
        }
        if (! quitting) {
            if (posted) {
                // A new frame has just been posted.  Update information about
                // last acquired image.
                attrs(_TAO_SIM_COUNTER) = buf.counter;
                attrs(_TAO_SIM_LAST) = buf.shmid;
            }
            if (posted || ticks == 0) {
                // Next or first acquisition buffer.
                if (++idx > nbufs) {
                    idx = 1;
                }
                key = keys(idx);
                buf = bufs(key);
                if (is_void(buf) || ! tao_wrlock(buf, 0)) {
                    // Allocate a new acquisition buffer if none or if not
                    // immediately available for writing.
                    buf = tao_create_shared_array(owner, T, width, height,
                                                  perms=perms);
                    h_set, bufs, key, buf;
                    tao_wrlock, buf;
                }
                tao_set_counter, buf, 0;
                tao_set_timestamp, buf, 0.0;
                attrs(_TAO_SIM_NEXT) = buf.shmid;
            }
            posted = false;
        }
        tao_set_data, cam, attrs;
        tao_unlock, cam;
        if (quitting) {
            break;
        }

        // Pause to mimic frame rate.
        now = tao_get_monotonic_time();
        elapsed = now - initial;
        ticks = max(ticks + 1, long(ceil(elapsed*fps)));
        seconds = double(ticks)/fps - elapsed;
        if (seconds > 0) {
            tao_sleep, seconds;
        }
        end_of_frame = tao_get_monotonic_time();

        if (acquiring) {
            // Mimic preprocessing of data.
            tao_set_data, buf, data(.., 1 + (ticks - 1)%nimgs);

            // Update frame counter and time-stamp.
            tao_set_counter, buf, ticks;
            tao_set_timestamp, buf, end_of_frame;

            // Post next frame.
            tao_unlock, buf;
            posted = true;
        }

    }

    // Note: unlocking of all buffers is done automatically and camera should
    // have been locked.
}

/*---------------------------------------------------------------------------*/
/* COMMON ROUTINES */

func tao_simcam_default_config(owner)
/* DOCUMENT tao_simcam_default_config(owner);

     Yields the default name of the configuration file for camera owned by
     OWNER.

   SEE ALSO tao_simcam_connect, tao_simcam_create.
 */
{
    return "/tmp/"+get_env("USER")+"-"+owner+".cfg";
}

func tao_warn(mesg)
/* DOCUMENT tao_warn, mesg;

     Print a warning message.
 */
{
    write, format="WARNING - %s\n";
}
