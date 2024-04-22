/*
 * tao-rt-tests.i --
 *
 * Tests and examples using Yorick interface to TAO real-time software.
 *
 *-----------------------------------------------------------------------------
 *
 * This file is part of the TAO real-time software licensed under the MIT
 * license (https://git-cral.univ-lyon1.fr/tao/tao-rt-yorick).
 *
 * Copyright (C) 2018-2021, Éric Thiébaut.
 */

require, "tao-rt.i";

func tao_measure_latency(cam, number=, timeout=, minimal=)
/* DOCUMENT res = tao_measure_latency(cam);

     Yield an array of time latencies when receiving images from a TAO camera
     server.  CAM is a TAO shared camera, the shared memory identifier of a TAO
     shared camera or the XPA access point name of an XPA camera server.

     The returned value is an array of size 6-by-NUMBER such that:

         res(1,i) = counter value for i-th image (0 if none);
         res(2,i) = "frame start" time for i-th image;
         res(3,i) = "frame end" time for i-th image;
         res(4,i) = "buffer ready" time for i-th image;
         res(5,i) = "image sent" time for i-th image;
         res(6,i) = "image received" time for i-th image;

     all times in seconds.  Note that all times may not be actually measured,
     if `res(k,0) > res(k,1)`, then the k-th timer is measured.

     Keyword NUMBER can be used to specify the number of images to acquire.
     Default value is 100.

     Keyword TIMEOUT can be used to specify the timeout in seconds (default to
     1 second) when waiting for images.

     If keyword MINIMAL is true, the time of arrival of an image is measured
     just after the corresponding shared array can be locked for read-only
     access and image pixels are not copied to minimize the latency; otherwise,
     tao_wait_image is called and the latency accounts for any overheads in
     this function.

   SEE ALSO: tao_attach_shared_camera, tao_wait_image, tao_analyze_latency.
 */
{
    cam = tao_connect_camera(cam);
    tao_ensure_unlocked, cam;
    if (is_void(number)) number = 100;
    if (is_void(timeout)) timeout = 1.0;
    res = array(double, 6, number);
    t0 = tao_get_monotonic_time();

    // Catch errors to prevent deadlocks.
    local cam, shmarr;
    if (catch(-1)) {
        tao_ensure_unlocked, cam, shmarr;
        error, catch_message;
    }
    prev_counter = 0;
    for (i = 1; i <= number; ++i) {
        if (minimal) {
            // Emulate tao_wait_image for minimal latency.
            //
            // Get next image identifier.
            tao_rdlock, cam;
            next = tao_get_next_image_shmid(cam);
            tao_unlock, cam;
            if (next >= 0) {
                // Wait for acquisition to complete and retrieve image data.
                shmarr = tao_attach_shared_array(next);
                if (!tao_rdlock(shmarr, timeout)) {
                    error, "timeout";
                }
                t5 = tao_get_monotonic_time(); // get it first
                t1 = shmarr.timestamp1;
                t2 = shmarr.timestamp2;
                t3 = shmarr.timestamp3;
                t4 = shmarr.timestamp4;
                counter = shmarr.counter;
                tao_unlock, shmarr;
                shmarr = []; // detach shared array
            } else {
                counter = 0;
            }
        } else {
            // Use tao_wait_image.
            arr = tao_wait_image(cam, timeout);
            t1 = tao_wait_image_timestamp1;
            t2 = tao_wait_image_timestamp2;
            t3 = tao_wait_image_timestamp3;
            t4 = tao_wait_image_timestamp4;
            t5 = tao_wait_image_timestamp5;
            counter = tao_wait_image_counter;
        }
        if (counter > prev_counter) {
            res(1,i) = counter;
            res(2,i) = t1 - t0;
            res(3,i) = t2 - t0;
            res(4,i) = t3 - t0;
            res(5,i) = t4 - t0;
            res(6,i) = t5 - t0;
            prev_counter = counter;
        }
    }
    return res;
}

func tao_analyze_latency(dat, io)
/* DOCUMENT tao_analyze_latency, dat[, io];

      Print analysis of measurements DAT returned by tao_measure_latency.
      Optional argument IO is the output file stream.

   SEE ALSO tao_measure_latency.
 */
{
    // Check input.
    names = ["Frame start", "Frame end", "Buffer ready",
             "Image sent", "Image received"];
    dims = dimsof(dat);
    if (numberof(dims) != 3 || (nrows = dims(2)) < 2 ||
        nrows > numberof(names) + 1) {
        error, swrite(format="expecting a 2-dimensional array of measurements with 2-%d rows",
                      numberof(names) + 1);
    }

    // Collect data, stripping invalid samples.
    cnt = lround(dat(1,)); // sample counter
    if (min(cnt) < 1) {
        // Strip invalid samples.
        sel = where(cnt > 0);
        cnt = cnt(sel);
    } else {
        sel = [];
    }
    nsamples =numberof(cnt);
    if (nsamples < 2) error, "insufficient number of samples";
    times = dat(2:nrows, sel);
    nframes = cnt(0) - cnt(1) + 1;

    // Figure out what are the first valid time-stamps (the first ones showing
    // an increase in time).
    org = 0;
    while (1n) {
        org += 1;
        if (org >= nrows || times(org,0) > times(org,1)) {
            break;
        }
    }
    if (org >= nrows) error, "time-stamps never increase";

    // Measure the mean and standard deviation of the send times between
    // samples.  Note the factor 1/2 in the variance which assumes that the
    // send times are independent random variables.
    dt = times(org,)(dif)/cnt(dif);
    send_rate_avg = dt(avg);
    send_rate_std = dt(rms)/sqrt(2);

    // Lost samples.
    losses = cnt(dif) - 1;
    sel = where(losses > 0);
    if (is_array(sel)) {
        losses = losses(sel);
        max_losses = max(losses);
        min_losses = min(losses);
        lost_syncs = numberof(losses);
        lost_frames = sum(losses);
        lost_histo = histogram(losses - (min_losses - 1));
    } else {
        lost_syncs = 0;
        lost_frames = 0;
    }

    // Report analysis.
    if (lost_syncs > 0) {
        i = where(lost_histo > 0);
        hist = swrite(format="%d frame(s) lost (%d times)",
                      i + (min_losses - 1), lost_histo(i));
    } else {
        hist = [];
    }
    delays = [];
    for (nxt = (prv = org) + 1; nxt < nrows; ++nxt) {
        if (times(nxt,0) > times(nxt,1)) {
            delay = times(nxt,) - times(prv,);
            grow, delays, swrite(format="'%s' - '%s': %.3f +/- %.3f µs",
                                 names(nxt), names(prv),
                                 delay(avg)*1E6, delay(rms)*1E6/sqrt(2));
            prv = nxt;
        }
    }

    tao_print_tree, io=io,
        swrite(format="Latency measurements on %d samples at %.3f Hz:",
               nsamples, 1/send_rate_avg),
        swrite(format="'%s' interval: %.3f +/- %.3f µs",
               names(org), send_rate_avg*1E6, send_rate_std*1E6),
        swrite(format="Lost syncs: %d / %d samples",
               lost_syncs, nsamples),
        swrite(format="Lost frames: %d / %d frames",
               lost_frames, nframes),
        hist,
        "Delays:", delays;
}

local _tao_tree_prefixes;
func tao_print_tree(.., prefix=, io=)
/* DOCUMENT tao_print_tree, args...;

     Print arguments in the form of a simple tree.  At any level of the tree, a
     scalar string entry is a leave, a list or an array is printed as a
     sub-tree.

     Keyword IO is to specify the output stream.  Keyword PREFIX is to specify
     a string prefix.
 */
{
    // Group arguments into a list, preserving sub-lists.
    args = [];
    while (more_args()) {
        local arg; eq_nocopy, arg, next_arg();
        args = _cat(args, (is_list(arg) ? _lst(arg) : arg));
    }
    _tao_print_tree, io, (is_void(prefix) ? "" : prefix), args, 0;
}
if (is_void(_tao_tree_prefixes))
    _tao_tree_prefixes = [" ├─ ", " │  ", " └─ ", "    "];
func _tao_print_tree(io, pfx, arg, stage)
{
    if (is_scalar(arg)) {
        if (stage == 1) {
            pfx += _tao_tree_prefixes(1);
        } else if (stage == 2) {
            pfx += _tao_tree_prefixes(3);
        }
        write, io, format="%s%s\n", pfx, arg;
    } else if (! is_void(arg)) {
        if (is_array(arg)) {
            n = numberof(arg);
        } else if (is_list(arg)) {
            // Wrap the list into a closure so that it is indexable like an
            // array or a tuple.
            n = _len(arg);
            arg = closure(_car, arg);
        } else if (is_tuple(arg)) {
            n = arg();
        } else {
            error, "unexpected argument type";
        }
        if (n > 0) {
            if (stage == 1) {
                pfx += _tao_tree_prefixes(2);
            } else if (stage == 2) {
                pfx += _tao_tree_prefixes(4);
            }
            ilast = 0;
            for (i = 1; i <= n; ++i) {
                local arg_i;
                eq_nocopy, arg_i, arg(i);
                if (is_scalar(arg_i) || is_void(arg_i)) {
                    ilast = i;
                }
            }
            for (i = 1; i <= n; ++i) {
                _tao_print_tree, io, pfx, arg(i), (i < ilast ? 1 : 2);
            }
        }
    }
}

func tao_live(cam, number=, timeout=, cmin=, cmax=, win=, wait=)
/* DOCUMENT tao_live, cam;

     Display images from a TAO camera server.  CAM is a TAO shared camera, the
     shared memory identifier of a TAO shared camera or the XPA access point
     name of an XPA camera server.

     Keyword NUMBER can be used to specify the number of images to display.

     Keywords TIMEOUT can be used to specify the timeout in seconds when
     waiting for images.

     Keywords WIN, CMIN and CMAX can be used to set the window and the levels
     for displaying the images.

   SEE ALSO: tao_attach_shared_camera, tao_wait_image, window, pli.
 */
{
    if (! is_void(win)) window, win;
    cam = tao_connect_camera(cam);
    if (is_void(wait)) wait = 100; // wait 0.1 seconds
    if (is_void(number)) number = 100000;
    if (is_void(timeout)) timeout = 1.0;
    for (i = 1; i <= number; ++i) {
        arr = tao_wait_image(cam, timeout, noweights=1);
        if (is_void(arr)) {
            write, format="%s\n", "Timeout!";
            continue;
        }
        fma;
        pli, arr, cmin=cmin, cmax=cmax;
        pltitle, swrite(format="Frame %d", arr.counter);
        pause, wait;
    }
}
