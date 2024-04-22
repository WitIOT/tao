/*
 * tao-visu.i --
 *
 * Yorick interface to TAO real-time software. TAO is a library for Adaptive
 * Optics software
 *
 *-----------------------------------------------------------------------------
 *
 * This file if part of the TAO software (https://git-cral.univ-lyon1.fr/tao)
 * licensed under the MIT license.
 * Updates can be found at https://git-cral.univ-lyon1.fr/tao/tao-yorick.
 *
 * Copyright (C) 2018-2019, Michel Tallon, Éric Thiébaut.
 */

/* TODO:
 * - If set to N>1 the frame used to compute the layout is shown during N
 *   seconds, if it is available.
 * - find a way to suspend after.
 *   may be when make a function with a pause, or the function in a waiting
 *   loop.
 * - make functions for each step in _tao_visu.
 * - Reminder: data from shm in wfs_process mode.
 *   (xmin, xmax, ymin, ymax, xoff, yoff, Wxx, Wxy, Wyy)
 *
 * - display
 */

/* SECTIONS
 * --------
 *
 * - CONFIGURATION
 *       - _tao_visu_private
 *
 * - PUBLIC FUNCTIONS
 *       - tao_visu
 *       - tao_visu_reset_cam
 *       - tao_visu_save_layout
 *       - tao_visu_show
 *       - tao_visu_stop
 *
 * - PRIVATE FUNCTIONS
 *       - _tao_visu
 *       - _tao_visu_cbar
 *       - _tao_visu_connect_camera
 *       - _tao_visu_debug
 *       - _tao_visu_display
 *       - _tao_visu_display_cmin_cmax
 *       - _tao_visu_display_fps
 *       - _tao_visu_display_this_frame
 *       - _tao_visu_display_scanned_servers
 *       - _tao_visu_error
 *       - _tao_visu_error_loop_count
 *       - _tao_visu_get_cam
 *       - _tao_visu_get_private
 *       - _tao_visu_get_servers
 *       - _tao_visu_get_snapshot
 *       - _tao_visu_info
 *       - _tao_visu_insert_template
 *       - _tao_visu_is_andor
 *       - _tao_visu_is_an_image
 *       - _tao_visu_plot_circle
 *       - _tao_visu_plot_hline
 *       - _tao_visu_plot_square
 *       - _tao_visu_plot_target
 *       - _tao_visu_plot_vline
 *       - _tao_visu_plot_wfs
 *       - _tao_visu_relaunch_on_error
 *       - _tao_visu_string_exposure
 *       - _tao_visu_string_uptime
 *       - _tao_visu_ticks
 *       - _tao_visu_too_old
 *       - _tao_visu_warn
 *       - _tao_visu_wfs_is_blessed
 *       - _tao_visu_wfs_load
 *       - _tao_visu_wfs_not_saved
 *       - _tao_visu_wfs_reset
 */

/*--+=^=+----------------------------------------------------------------------
 *                              CONFIGURATION
 *---------------------------------------------------------------------------*/

_TAO_VISU_SOURCE = current_include();

if (!is_func(strjoin)) {
  // strjoin is currently not in autoload in "ylib-start.i".
  // Including "utils.i" destroys h_new if h_new is still an autoload object
  // (bug?). But if "yeti.i" is already included, h_new is a function and
  // including "utils.i" does not destroy h_new. In case h_new is still an
  // autoload object, "yeti.i" is included after.
  include, "utils.i", 1;
}

if (!is_func(h_new)) {
  // For yeti 6.4.0 and earlier.
  // With yeti 6.4.1 and above, h_new is an autoload object and "yeti.i" will
  // not be included here.
  include, "yeti.i", 1;
}
if (is_void(TAO_WFS_LAYOUT)) {
  include, dirname(_TAO_VISU_SOURCE)+"/tao-wfs-layout.i", 1;
}

if (!is_hash(_tao_visu_private)) {
  _tao_visu_private = h_new(
    //
    // ---- keywords default values
    //
    autoreset = 0n,          // true if autoreset is requested.
    autocmax = 1n,           // flag for auto mode for cmax.
    autocmin = 1n,           // flag for auto mode for cmin.
    cam = [],                // TAO remote camera object.
    cbar = 1n,               // default is to display the color bar.
    circle = [],             // no circle displayed.
    circle_col = "green",    // default color for circle.
    cmax = [],               // not fixed by default.
    cmin = [],               // not fixed by default.
    dark = [],               // dark to be removed.
    debug = [],              // activate messages with _tao_visu_debug.
    debugmode = 0n,          // do no stop _tao_visu if an error occurs.
    dpi = [],                // do no change default window dpi (75 by default).
    exposure = [],           // no average of frames to mimic long exposure.
    flip = 0n,               // flip the displayed frame along x axis.
    flop = 0n,               // flip the displayed frame along y axis.
    fps = [],                // maximum frame rate.
    hline= [],               // no horizontal line displayed.
    hline_col = "green",     // default color for hline.
    hoppingcmax = 0n,        // asymmetric autocmax.
    hoppingcmin = 0n,        // asymmetric autocmin.
    sensorcoord = 1n,        // flag for relative/absolute coord. of the ROI.
    square = [],             // no square displayed.
    square_col = "green",    // default color for square.
    target = [],             // no target displayed.
    target_col = "green",    // default color for target.
    template = [],           // do no show template.
    transpose = 0n,          // swap x and y axes of the displayed frame.
    vline= [],               // no vertical line displayed.
    vline_col = "green",     // default color for vline.
    wfs_col = "green",       // default color for wavefront sensor layout.
    wfs_process = 0n,        // do not show display from wfs process in memory.
    wfs_profile= 0n,         // do not show vert./horiz. pixel profiles.
    wfs_reset = [],          // no wfs layout to load or to compute.
    wfs_show = 0n,           // no display of wfs layout.
    wfs_slopes = [],         // do no show slopes.
    wfs_tune = 0n,           // no display of wfs layout with flux meter.
    win = [],                // use current window, or open a fresh one.
    //
    // ---- default parameters
    //
    cmax_time_cst = 3.0,     // time constant (s) for smoothing cmax variations.
    cmin_time_cst = 3.0,     // time constant (s) for smoothing cmin variations.
    delay_min = 5e-3,        // shortest delay between frames when running.
    delay_reset = 3.0,       // waiting time for change of state when resetting.
    delay_loop = 5e-3,       // starting val of measured delay of display loop.
    delay_running = 5e-3,    // default value of delay while running.
    delay_too_old = 10.0,    // delay we consider _tao_visu is dead.
    delay_waiting = 1.0,     // long delay between frames when not running.
    fps_time_cst = 2.0,      // time constant (s) for fps stabilisation.
    margin = 0,              // default margin for tao_wfs_layout_boxes
    numberof_resetAndor = 0, // number of reset USB port of Andor camera.
    numberof_timeout = 0,    // store the number of timeout.
    reset_level = 0,         // number of times abort/start reset fails.
    timeout = 1.0,           // timeout for waiting for a new frame.
    start_time_visu = 0,     // for computing the uptime of tao_visu.
  __=);
}


/*--+=^=+----------------------------------------------------------------------
 *                             PUBLIC FUNCTIONS
 *---------------------------------------------------------------------------*/

func tao_visu(cam, autoreset=, autocmax=, autocmin=, cbar=, circle=,
              circle_col=, cmax=, cmin=, debug=, debugmode=, dpi=, exposure=,
              flip=, flop=, fps=, hline=, hline_col=, hoppingcmax=,
              hoppingcmin=, sensorcoord=, square=, square_col=, target=,
              target_col=, template=, transpose=, vline=, vline_col=, wfs_col=,
              wfs_error=, wfs_flux=, wfs_full_pup=, wfs_mask=, wfs_process=,
              wfs_profile=, wfs_reset=, wfs_show=, wfs_slopes=, wfs_tune=,
              win=)
/* DOCUMENT  tao_visu, cam
     - or -  tao_visu

    launches a display attached to the given TAO remote camera CAM (first
    form), or the first available TAO remote camera if CAM is not given
    (second form). If given, CAM can be a TAO shared camera, the shared memory
    identifier of a TAO shared camera or the XPA access point name of an XPA
    camera server. In this case, tao_visu will stay attached to this camera or
    will wait for it to appear. If CAM is not given, the function looks for the
    first available XPA camera server returned by xpa_list(), or waits for any
    one to appear. In this case, tao_visu may switch to another camera if the
    attached one is lost.

    Once opened, the display is running in the background and new commands can
    be entered at the prompt, for instance for interacting with the XPA camera
    server (e.g. tao_start). The display is always opened and running, and
    informs on the status of the server, i.e. if the XPA camera is sleeping, or
    if the server is lost. In this last case, the display just waits for a new
    server to appear. In any case, the display can be stopped with function
    tao_visu_stop, or connected to another camera with first form.

    Command "animate,1" given at the command line removes the possible
    flickering of the displayed images, but prevent to zoom in the window.

    When launching tao_visu or while tao_visu is running, the following
    keywords are available to tune its behavior.

KEYWORDS:
    autoreset=   - if set to true, tao_visu attempts to reset the TAO camera
                   server if timeout appears. The method used is to abort/start
                   the server. In case of an Andor camera attached to a USB
                   port, the port is reset if 3 successive retries are
                   unsuccessful. Default is false, since the display is not
                   supposed to operate the server. A command tao_visu_reset_cam
                   is available separately.
    cbar=        - By default, a color bar is displayed on the right side of
                   the frame. Setting cbar=0 removes this color bar, so that
                   the display can be slightly faster.
    circle=      - If set to vector [x, y, d], a circle of diameter d is
                   displayed centered at coordinates [x, y] on the display.
                   Several circles can be plotted with [x1,y1,d1,x2,y2,d2,...],
                   or [[x1,y1,d1],[x2,y2,d2],...]. Any value that does not
                   match will remove the circles. For instance: circle="".
    circle_col=  - color of the circle (default is "green"). See help, color.
    cmax=        - same as keyword cmax of the pli plotting function. A
                   numerical value is expected. Setting cmax to a non numerical
                   value, for instance cmax="no", will reset cmax to the
                   maximum value for each frame.
    autocmax=    - if set to 1, cmax will slowly follow the maximum value in
                   the frames, reducing the flickering of the displayed images
                   because of transients, which may be sensitive with small
                   images. For large images, looking for the maximum in each
                   frames may reduce the frame rate of the display. If set to a
                   non-zero value, N, the tracked value will be the Nth
                   greatest value from the maximum. This allows to reject N
                   values corresponding to hot pixels for instance. Default is
                   true.
    hoppingcmax= - if set to 1, autocmax control is asymmetric: cmax increases
                   instantaneously if the tracking value is higher, and the
                   relaxation is only active for decreasing its value. Default
                   if false.
    cmin=        - same as keyword cmax, but for minimum instead of maximum.
    autocmin=    - same as autocmax, but for cmin.
    hoppingcmin= - same as hoppingcmax, but for cmin.
    debugmode=   - tao_visu will not be stopped if an error occurs in yorick
                   shell. But the method used prevents entering in yorick debug
                   mode after any error (see help, dbinfo). Setting debugmode
                   to true enables yorick debug mode, but tao_visu will need to
                   be launched again (just by entering "tao_visu") after any
                   error in yorick shell.
    fps=         - gives the number of frames per second to be displayed.
                   Anyway, the displayed effective frame rate cannot be higher
                   than the camera frame rate. Setting fps to a unacceptable
                   value, for instance fps="", cancels fps constraint
                   so that the display runs at its maximum speed (and at its
                   maximum CPU load).
    exposure=    - if set to a positive value in seconds, the displayed frames
                   are averaged over this period to slow down fast evolution.
                   Setting to zero or to an invalid value cancels this mode.
    flip=        - If set to true, flip the x axis of the displayed image.
    flop=        - If set to true, flip the y axis of the displayed image.
    hline=       - gives the ordinate at which a horizontal line is displayed.
                   Setting to [y1, y2, y3, ...] will draw several lines at
                   these ordinates. The lines can be removed if set to any
                   unacceptable value, for instance hline="". (see vline).
    hline_col=   - color of hline (default is "green"). See help, color.
    sensorcoord= - If set to true, displayed coordinates are the absolute
                   coordinates of the sensor, instead of the relative
                   coordinates in the displayed images. Default is true.
    square=      - If set to vector [x, y, s], a square of side s is
                   displayed centered at coordinates [x, y] on the display.
                   Several squares can be plotted with [x1,y1,s1,x2,y2,s2,...],
                   or [[x1,y1,s1],[x2,y2,s2],...]. Any value that does not
                   match will remove the squares. For instance: square="".
    square_col=  - color of the square (default is "green"). See help, color.
    target=      - If set to coordinates [x, y], a target is displayed at these
                   coordinates on the display. Any value that does not match
                   [x, y] will remove the target. For instance: target=0.
    target_col=  - color of the target (default is "green"). See help, color.
    template=    - If set to true, and if the template is available, it is
                   inserted in the middle of the display.
    transpose=   - If set to true, swap x and y axes of the display.
    vline=       - gives the abscissa at which a vertical line is displayed.
                   Setting to [x1, x2, x3, ...] will draw several lines at
                   these abscissae. The lines can be removed if set to any
                   unacceptable value, for instance vline="". (see hline).
    vline_col=   - color of hline (default is "green"). See help, color.
    wfs_col=     - color of the displayed information selected with keywords
                   WFS_SHOW and WFS_TUNE (default is "green"). See help, color.
    wfs_flux=    - If set to 1, tao_visu displays the value of the flux in each
                   subimage, relatively to the brightest one, to help in the
                   alignment of the wavefront sensor.
   wfs_full_pup= - If set to 1, the selected geometry of the wavefront sensor
                   corresponds to a full pupil instead of the obscurated one.
                   Setting to 0 comes back to the obscurated pupil (default).
    wfs_error=   - If set to 1, the measurement errors are displayed as
                   ellipses in the subimages if data are available. This
                   display is activated only in wfs_process mode. Setting
                   wfs_error also fixes (or fixes again) the limits of the
                   display. Default is false.
    wfs_mask=    - If set to 1, the mask of the wfs layout is applied on the
                   displayed frames. This mask is removed if set to 0. The wfs
                   layout must be already loaded with keyword WFS_SHOW, or
                   WFS_MASK can be set to the name of a file that contains a
                   WFS layout previously saved with tao_visu_save_layout (see
                   keyword wfs_reset).
    wfs_process= - If set to 1, tao_visu reads the data from shared memory and
                   displays the result of the wavefront sensing processing.
    wfs_profile= - If set to 1, tao_visu overplots the sum of the pixels along
                   the lines and the columns, to help in the alignment of the
                   wavefront sensor.
    wfs_reset=   - If set to 1, tao_visu is stopped and 1000 frames are
                   acquired and averaged to compute a new WFS layout, saved in
                   memory. The number N of averaged frames can be specified by
                   giving N instead of 1. If set to -1, the layout is computed
                   again from the saved averaged image, after loading it for
                   instance. The layout can be saved in a file by using
                   tao_visu_save_layout. If set to an image of the WFS (flat
                   field for instance), or to the name of a file that contains
                   such an image (fits file or file previously written with
                   tao_visu_save_layout), a new layout is computed again from
                   the image.
    wfs_show=    - If set to 1, the wfs layout is overplotted on the displayed
                   frames. This display is removed if set to 0. If set to N>1
                   the frame used to compute the layout is shown during N
                   seconds, if it is available. The wfs layout must be already
                   loaded. WFS_SHOW can be set to the name of a file that
                   contains a WFS layout previously saved with
                   tao_visu_save_layout (see keyword wfs_reset).
    wfs_slopes=  - If set to 1, measured slopes are displayed as flying crosses
                   if data are available. This display is activated only in
                   wfs_process mode. Default is false.
    wfs_tune=    - Same as keyword wfs_show, but circles are also overplotted
                   in each subaperture, with an area proportional to the flux
                   in the subimage. This layout is designed to help in the
                   alignment of the wavefront sensor when used with the
                   reference source.
    win=         - number of the window to use. It can be change on the fly.
    dpi=         - sets the dpi of the window. Default is 75 dpi (see window
                   help).

SEE ALSO: tao_visu_stop, tao_visu_reset_cam, tao_visu_show, tao_start,
          tao_connect_camera, animate, after, pli, palette, window, color
*/
{
  h = _tao_visu_get_private();         // Get the private space.

  // ---- cam
  // Load the given cam if cam is given. If cam is not given, _tao_visu will
  // find a TAO camera server by itself or wait for it.
  if (!is_void(cam)) {
    obj = _tao_visu_connect_camera(cam);
    if (is_void(obj)) {
      if (is_string(cam)) {
        _tao_visu_warn, "cannot connect to camera "+cam+".";
      } else {
        _tao_visu_warn, "cannot connect to camera.";
      }
      return;
    } else {
      _tao_visu_set_cam_in_ht, obj, h;
      // Record the name of the camera server so that to reconnect if it is
      // killed and launched again.
      server_name = h.cam_name;
      if (!is_string(h.stick_to_server) || server_name != h.stick_to_server) {
        _tao_visu_info, "tao_visu will keep tracking "+server_name;
      }
      h_set, h, stick_to_server = server_name;
    }
  }

  // ---- autoreset
  if (!is_void(autoreset)) h_set, h, autoreset = autoreset;

  // ---- autocmax
  if (is_integer(autocmax)) {
    autocmax = max(autocmax, 0);
    if (h.autocmax && !autocmax && !is_void(h.cmax) && is_void(cmax)) {
      // autocmax mode was selected before: inform cmax will be fixed at the
      // last value automatically computed.
      _tao_visu_info, swrite(format="cmax fixed at %g", h.cmax);
    }
    h_set, h, autocmax = autocmax;
    h_delete, h, "prev_autocmax";
  }

  // ---- autocmin
  if (is_integer(autocmin)) {
    autocmin = max(autocmin, 0);
    if (h.autocmin && !autocmin && !is_void(h.cmin) && is_void(cmin)) {
      // autocmin mode was selected before: inform cmin will be fixed at the
      // last value automatically computed.
      _tao_visu_info, swrite(format="cmin fixed at %g", h.cmin);
    }
    h_set, h, autocmin = autocmin;
    h_delete, h, "prev_autocmin";
  }

  // ---- cbar
  if (!is_void(cbar)) {
    h_set, h, cbar = (cbar ? 1n : 0n);
  }

  // ---- circle
  if (!is_void(circle)) {
    if (is_numerical(circle)) {
      if (numberof(circle)%3 == 0) {
        circle = reform(circle, 3, numberof(circle)/3);
        h_set, h, circle = circle;
      } else {
        _tao_visu_error, "wrong format for circle definition";
        h_set, h, circle = [];
        return;
      }
    } else {
      h_set, h, circle = [];
    }
  }

  // ---- circle_col
  if (!is_void(circle_col) && is_string(circle_col)) {
    h_set, h, circle_col = circle_col(1);
  }

  // ---- cmax
  if (!is_void(cmax)) {
    if (is_numerical(cmax)) {
      if (h.autocmax) {
        h_set, h, prev_autocmax = h.autocmax, autocmax = 0;
        _tao_visu_info, "autocmax unset";
      }
      if (!is_void(h.cmin) && cmax < h.cmin) {
        _tao_visu_warn, "cmax set to cmin (cannot be lower)";
        h_set, h, cmax = double(h.cmin);
      } else {
        h_set, h, cmax = double(cmax);
      }
    } else {
      if (!is_void(h.prev_autocmax)) {
        h_set, h, autocmax = h.prev_autocmax;
        h_delete, h, "prev_autocmax";
      }
      h_set, h, cmax = [];
    }
  }

  // ---- cmin
  if (!is_void(cmin)) {
    if (is_numerical(cmin)) {
      if (h.autocmin) {
        h_set, h, prev_autocmin = h.autocmin, autocmin = 0;
        _tao_visu_info, "autocmin unset";
      }
      if (!is_void(h.cmax) && cmin > h.cmax) {
        _tao_visu_warn, "cmin set to cmax (cannot be larger)";
        h_set, h, cmin = double(h.cmax);
      } else {
        h_set, h, cmin = double(cmin);
      }
    } else {
      if (!is_void(h.prev_autocmin)) {
        h_set, h, autocmin = h.prev_autocmin;
        h_delete, h, "prev_autocmin";
      }
      h_set, h, cmin = [];
    }
  }

  // ---- debug
  if (!is_void(debug)) {
    h_set, h, debug = (debug ? 1n : 0n);
  }

  // ---- debugmode
  extern after_error;
  if (!is_void(debugmode)) h_set, h, debugmode = (debugmode ? 1n : 0n);
  if (h.debugmode) {
    after_error = [];
  } else {
    after_error = _tao_visu_relaunch_on_error;
    h_set, h, error_start = [];      // Reset the detection of error loops.
  }

  // ---- dpi
  if (!is_void(dpi)) {
    if (is_integer(dpi) && is_scalar(dpi) && dpi > 0) {
      h_set, h, dpi = dpi;
    } else {
      _tao_visu_error, "dpi must be a positive integer";
      return;
    }
  }

  // ---- exposure
  if (!is_void(exposure)) {
    if (is_numerical(exposure)
        && is_scalar(exposure) && exposure > 0) {
      h_set, h, exposure = double(exposure);
    } else {
      h_delete, h, "exposure";
    }
  }

  // ---- fps
  if (!is_void(fps)) {
    // delay_running must always been defined.
    if (is_numerical(fps) && is_scalar(fps) && fps > 0) {
      delay_min = h.delay_min;
      h_set, h, fps_requested = fps,
                delay_running = max(1.0/fps-delay_min, delay_min);
    } else {
      h_set, h, fps_requested = [],
                delay_running = h.delay_min;
    }
    h_set, h, fps = [];   // must be updated.
  }

  // ---- flip
  if (!is_void(flip)) {
    h_set, h, flip = (flip ? 1n : 0n);
  }

  // ---- flop
  if (!is_void(flop)) {
    h_set, h, flop = (flop ? 1n : 0n);
  }

  // ---- hline
  if (!is_void(hline)) {
    if (is_numerical(hline) && allof(hline >= 0)) {
      h_set, h, hline = hline;
    } else {
      h_set, h, hline = [];
    }
  }

  // ---- hline_col
  if (!is_void(hline_col) && is_string(hline_col)) {
    h_set, h, hline_col = hline_col(1);
  }

  // ---- hoppingcmax
  if (!is_void(hoppingcmax)) {
    h_set, h, hoppingcmax = (hoppingcmax ? 1n : 0n);
  }

  // ---- hoppingcmin
  if (!is_void(hoppingcmin)) {
    h_set, h, hoppingcmin = (hoppingcmin ? 1n : 0n);
  }

  // ---- sensorcoord
  if (!is_void(sensorcoord)) {
    h_set, h, sensorcoord = (sensorcoord ? 1n : 0n);
  }

  // ---- square
  if (!is_void(square)) {
    if (is_numerical(square)) {
      if (numberof(square)%3 == 0) {
        square = reform(square, 3, numberof(square)/3);
        h_set, h, square = square;
      } else {
        _tao_visu_error, "wrong format for square definition";
        h_set, h, square = [];
        return;
      }
    } else {
      h_set, h, square = [];
    }
  }

  // ---- target
  if (!is_void(target)) {
    if (is_numerical(target) && numberof(target) == 2) {
      h_set, h, target = target;
    } else {
      h_set, h, target = [];
    }
  }

  // ---- target_col
  if (!is_void(target_col) && is_string(target_col)) {
    h_set, h, target_col = target_col(1);
  }

  // ---- template
  if (!is_void(template)) {
    h_set, h, template = (template ? 1n : 0n);
  }

  // ---- transpose
  if (!is_void(transpose)) {
    h_set, h, transpose = (transpose ? 1n : 0n);
  }

  // ---- vline
  if (!is_void(vline)) {
    if (is_numerical(vline) && allof(vline >= 0)) {
      h_set, h, vline = vline;
    } else {
      h_set, h, vline = [];
    }
  }

  // ---- vline_col
  if (!is_void(vline_col) && is_string(vline_col)) {
    h_set, h, vline_col = vline_col(1);
  }

  // ---- wfs_col
  if (!is_void(wfs_col) && is_string(wfs_col)) {
    h_set, h, wfs_col = wfs_col(1);
  }

  // ---- wfs_error
  if (!is_void(wfs_error)) {
    h_set, h, wfs_error = (wfs_error ? 1n : 0n);
    // Fix the limits in case the ellipses are plotted outside the images.
    lim = limits();
    limits, lim(1), lim(2), lim(3), lim(4);
  }

  // ---- wfs_flux
  if (!is_void(wfs_flux)) {
    h_set, h, wfs_flux = (wfs_flux ? 1n : 0n);
  }

  // ---- wfs_full_pup
  if (!is_void(wfs_full_pup)) {
    h_set, h, wfs_full_pup = (wfs_full_pup ? 1n : 0n);
  }

  // ---- wfs_mask
  if (!is_void(wfs_mask)) {
    if (is_hash(wfs_mask) || is_string(wfs_mask)) {
      _tao_visu_wfs_load, wfs_mask;
      wfs_mask = 1n;
    }
    if (is_integer(wfs_mask)) {
      n = wfs_mask;
      if (n == 0) {
        h_set, h, wfs_mask = 0n;
      } else if (n > 0) {
        if (h.wfs_layout) {
          h_set, h, wfs_mask = 1n;
          if (n > 1) {
            if (is_void(h.wfs_layout.mean)) {
              _tao_visu_error, "no image in current loaded WFS layout.";
              return;
            } else {
              _tao_visu_display_this_frame, h.wfs_layout.mean, n;
            }
          }
        } else {
          _tao_visu_error,
            "no WFS layout currently loaded. Please give a file or kwd wfs_reset.";
          return;
        }
      } else {
        _tao_visu_error, "negative wfs_mask has no meaning.";
        return;
      }
      _tao_visu_wfs_not_saved;
    } else {
      _tao_visu_error, "wfs_mask not understood";
      return;
    }
  }

  // ---- wfs_process
  if (!is_void(wfs_process)) {
    if (is_hash(wfs_process)) {     // FIXME: to be removed.
      _tao_visu_warn, "fake seen";
      h_set, h, wfs_process_workspace = h_new(
         wfs_frame = h_new(data=wfs_process.frame, counter=-1),
         wfs_data = h_new(data=wfs_process.data));
      h_set, h, wfs_process = -1n;

    } else if (wfs_process) {
      file = "/tmp/tao-wfs-outputs-shmids";
      if (open(file, "r", 1)) {
        fh = open(file,"r");
        shmid_frame = int(tonum(rdline(fh)));
        shmid_data = int(tonum(rdline(fh)));
        close, fh;
        wfs_frame = tao_attach_shared_array(shmid_frame);
        wfs_data = tao_attach_shared_array(shmid_data);
        h_set, h, wfs_process_workspace = h_new(
                    counter = 0,
                    shmid_frame = shmid_frame,
                    shmid_data = shmid_data,
                    wfs_frame = wfs_frame,
                    wfs_data = wfs_data);
        h_set, h, wfs_process = 1n;
      } else {
        _tao_visu_error, "file \""+file+"\" not found.";
        h_set, h, wfs_process = 0n;
      }
    } else {
      h_set, h, wfs_process = 0n;
      pause, lround(h.delay_running*2e3);  // 2*delay, in milliseconds.
      if (h.wfs_process_workspace) {
        if (is_void(h.wfs_process_workspace.wfs_layout_bak)) {
          h_delete, h.wfs_layout, "cache";
        } else {
          h_set, h, wfs_layout = h.wfs_process_workspace.wfs_layout_bak;
        }
        h_delete, h, "wfs_process_workspace";
      }
    }
  }

  // ---- wfs_profile
  if (!is_void(wfs_profile)) {
    h_set, h, wfs_profile = (wfs_profile ? 1n : 0n);
  }

  // ---- wfs_reset
  if (!is_void(wfs_reset)) {
    if (is_integer(wfs_reset)) {
      n = wfs_reset(1);
      if (n < 0) {
        if (_tao_visu_wfs_is_blessed(h.wfs_layout)) {
          _tao_visu_info, "resetting current WFS layout."
          _tao_visu_wfs_reset, h.wfs_layout;
        } else {
          _tao_visu_error, "no WFS layout currently loaded.";
        }
      } else if (n > 0) {
        if (n == 1) n = 1000;              // Default is 1000 frames.
        _tao_visu_info,
          swrite(format="acquiring %d frames for a new WFS layout.", n);
        img = _tao_visu_get_snapshot(n);
        if (_tao_visu_is_an_image(img)) {
          _tao_visu_wfs_reset, img;        // reset and load if successful
        }
      } else {
        _tao_visu_error, "wfs_reset must not be set to zero.";
        return;
      }
    } else {
      _tao_visu_wfs_reset, wfs_reset;    // reset and load if successful
    }
  }

  // ---- wfs_slopes
  if (!is_void(wfs_slopes)) {
    h_set, h, wfs_slopes = (wfs_slopes ? 1n : 0n);
  }

  // ---- wfs_show
  if (!is_void(wfs_show)) {
    if (!is_void(wfs_tune)) {
      _tao_visu_error, "Please chose either wfs_show or wfs_tune.";
      return;
    }
    if (is_hash(wfs_show) || is_string(wfs_show)) {
      _tao_visu_wfs_load, wfs_show;
      wfs_show = 1n;

    } else if (is_integer(wfs_show)) {
      n = wfs_show;
      if (n == 0) {
        h_set, h, wfs_show = 0n, wfs_tune = 0n;
      } else if (n > 0) {
        if (h.wfs_layout) {
          h_set, h, wfs_show = 1n;
          if (n > 1) {
            if (is_void(h.wfs_layout.mean)) {
              _tao_visu_error, "no image in current loaded WFS layout.";
              return;
            } else {
              _tao_visu_display_this_frame, h.wfs_layout.mean, n;
            }
          }
        } else {
          _tao_visu_error,
            "no WFS layout currently loaded. Please give a file or kwd wfs_reset.";
          return;
        }
      } else {
        _tao_visu_error, "negative wfs_show has no meaning.";
        return;
      }
      _tao_visu_wfs_not_saved;
    } else {
      _tao_visu_error, "wfs_show not understood";
      return;
    }
  }

  // ---- wfs_tune
  if (!is_void(wfs_tune)) {
    if (is_hash(wfs_tune) || is_string(wfs_tune)) {
      _tao_visu_wfs_load, wfs_tune;
      wfs_tune = 1n;
    }
    if (is_integer(wfs_tune)) {
      n = wfs_tune;
      if (n == 0) {
        h_set, h, wfs_tune = 0n;
      } else if (n > 0) {
        if (h.wfs_layout) {
          h_set, h, wfs_tune = 1n;
          if (n > 1) {
            if (is_void(h.wfs_layout.mean)) {
              _tao_visu_error, "no image in current loaded WFS layout.";
              return;
            } else {
              _tao_visu_display_this_frame, h.wfs_layout.mean, n;
            }
          }
        } else {
          _tao_visu_error,
            "no WFS layout currently loaded. Please give a file or kwd wfs_reset.";
          return;
        }
      } else {
        _tao_visu_error, "negative wfs_tune has no meaning.";
        return;
      }
      _tao_visu_wfs_not_saved;
    } else {
      _tao_visu_error, "wfs_tune not understood";
      return;
    }
  }

  // ---- win
  if (!is_void(win)) {
    if (is_integer(win) && is_scalar(win) && win >= 0 && win <= 64) {
      window, win, wait=1, dpi = h.dpi;
      pause, 300;             // Wait for the window in case display is running.
    } else {
      _tao_visu_error, "win number must be an integer in range 0-64.";
      return;
    }
  } else {
    if (current_window() == -1) {
      // No current window. Prefer style "work.gs"
      window, style="work.gs", dpi=h.dpi;
    }
    win = window();           // Get the current win number.
  }
  // Store win so that to stop when win is closed, and dpi to stick to same dpi
  // when tao_visu is relaunched.
  h_set, h, win = win, dpi = long(window_geometry()(1));

  // ---- update flag plot_wfs
  h_set, h, plot_wfs = (h.wfs_show || h.wfs_tune || h.wfs_flux || h.wfs_profile
                        || (h.wfs_process && (h.wfs_slopes || h.wfs_error)));

  // launch if not already launched.
  if (! h.running || _tao_visu_too_old()) {
    h_set, h, running = 1n;
    after, h.delay_running, _tao_visu;
  }
}


func tao_visu_reset_cam(resetusb=)
/* DOCUMENT   tao_visu_reset_cam

    attempts to reset the TAO camera server. To be used when the camera issues
    a timeout. The method used is to abort/start the server. If keyword
    resetusb is given and if an Andor camera is attached to a USB port, the USB
    port is also reset (shell command resetAndor must be available).

SEE ALSO: tao_visu
*/
{
  h = _tao_visu_get_private();                 // Get the private space.

  if (is_void(h.cam)) return;                  // no camera currently loaded.

  // Abort camera
  tao_abort, h.cam;

  // Wait for sleeping state.
  time_step = 100;                             // every 100 ms
  count = lround(h.delay_reset*1e3/time_step); // nb of retries for this delay.
  camera_is_waiting = 0n;
  for (i=1; i<=count; i++) {
    if (h.cam.state == TAO_STATE_WAITING) {
      camera_is_waiting = 1n;
      break;
    }
    pause, time_step;
  }
  // FIXME: reset Andor only if cam is Andor!
  // Attempt a reset of USB if server is an Andor camera, and timeout
  // seriousness is high enough.
  if (!camera_is_waiting && (h.reset_level > 2 || resetusb)) {
    system, "type resetAndor 1>/dev/null 2>&1 && resetAndor 1>/dev/null";
    h_set, h, numberof_resetAndor = h.numberof_resetAndor + 1;
  }

  // Now restart
  tao_start, h.cam;
}


func tao_visu_save_layout(file)
/* DOCUMENT  tao_visu_save_layout, filename

    saves the wavefront sensor layout in given file FILENAME. If no WFS layout
    is loaded, a message is issued.

SEE ALSO: tao_visu
*/
{
  h = _tao_visu_get_private();         // Get the private space.

  layout = h.wfs_layout;
  if (_tao_visu_wfs_is_blessed(layout)) {
    wfs = h_copy(layout, 1);
    h_delete, wfs, "cache", "layout_is_saved";
    yhd_save, file, wfs, overwrite=1;
    h_set, layout, layout_is_saved = file;
    _tao_visu_info, "WFS layout saved in \""+basename(file)+"\".";
  } else {
    _tao_visu_error, "no WFS layout to save";
  }
}


func tao_visu_show(cam)
/* DOCUMENT  tao_visu_show
     - or -  tao_visu_show, cam

    First form prints a summary of the parameters of the current displayed
    camera and of the display with the following layout:

       sensor  Andor0 - 2048x2048   fps= 20.0   exp= 0.1ms   acquiring
          roi  offset= 1380,1100   size= 200x200   center= 1480,1200
               #timeout= 0   #reset= 0
      display  fps= 14.5   cmin= 85.9   cmax= 2728.2   uptime= 2:14:22

    Second form connects to the given camera and only shows the two first
    lines of the layout.

SEE ALSO: tao_visu
*/
{
  h = _tao_visu_get_private();         // Get the private space.

  if (is_void(cam)) {
    cam = h.cam;
    display_visu_info = 1n;
  } else {
    obj = _tao_visu_connect_camera(cam);
    if (is_void(obj)) {
      // FIXME: print(cam) ???
      _tao_visu_warn, "cannot connect to " + print(cam);
      return;
    }
    // FIXME: remove display only if obj != h.cam
    display_visu_info = 0n;
  }
  if (!is_void(cam)) {
    prop = tao_get_members(cam, autolock=1);
    write, format="   sensor  %s - %4dx%-4d  fps= %6.1f     exp= %s   %s\n",
           prop.owner, prop.sensorwidth, prop.sensorheight, prop.framerate,
           _tao_visu_string_exposure(prop.exposuretime, utf=1),
           tao_get_state_name(prop.state);
    xoff = prop.xoff;
    yoff = prop.yoff;
    width = prop.width;
    height = prop.height;
    write, format="      roi  offset= %4d,%-6d  size= %dx%d   center= %4d,%-4d\n",
           xoff, yoff, width, height, xoff+width/2+1, yoff + height/2+1;
    timeout = h.numberof_timeout;
    if (is_void(timeout)) timout = 0;
    // FIXME: only Andor hard reset? add general reset if autoreset=1.
    reset = h.numberof_resetAndor;
    if (is_void(reset)) reset = 0;
    write, format="           pixel-type= %-6s   #timeout= %-5d #reset= %d\n",
           typeof(tao_type(prop.pixeltype)(0)), timeout, reset;
  } else {
    write, format="%s\n", "   sensor  (no server)";
  }
  if (display_visu_info) {
    cmin = is_void(h.cmin) ? "----" : swrite(format="%.1f", h.cmin);
    cmax = is_void(h.cmax) ? "----" : swrite(format="%.1f", h.cmax);
    fps  = is_void(h.fps)  ? "----" : swrite(format="%.1f", h.fps);
    uptime= _tao_visu_string_uptime(tao_get_monotonic_time()-h.start_time_visu);
    write, format="  display  fps= %s   cmin= %s   cmax= %s   uptime= %s\n",
           fps, cmin, cmax, uptime;
  }
}


func tao_visu_stop(void)
/* DOCUMENT  tao_visu_stop

    stops the running display and cleans up.

SEE ALSO: tao_visu
*/
{
  h = _tao_visu_get_private();          // Get the private space.

  if (!is_void(h.win)) {
    if (window_exists(h.win)) {
      // If animate is on, the last image will not be displayed. No way to know
      // if animate was selected or not. So we set it to 0 anyway. But only if
      // a window is available since "animate" will opens a new window
      // otherwise.
      animate, 0;
    } else {
      // Ensure we have no more window in private hash table.
      h_set, h, win=[];
    }
  }
  h_set, h, running = 0n;               // kind signal to exit.

  delay = h.delay_waiting;
  if (!is_void(h.cam) && h.cam.state == TAO_STATE_WORKING) {
    delay = h.delay_running;
  }
  pause, lround(delay*2e3);             // 2*delay, in milliseconds.
  h_delete, h, "cam_already_seen";      // Next launch will be first time.
  after, -, _tao_visu;                  // Kill in case something wrong.
}



/*--+=^=+----------------------------------------------------------------------
 *                              PRIVATE FUNCTIONS
 *---------------------------------------------------------------------------*/

func _tao_visu(void)
/* DOCUMENT  _tao_visu

    operates the display, reacting to the states of the XPA camera server.
    This is the underground worker for tao_visu.

SEE ALSO: tao_visu
*/
{
  h = _tao_visu_get_private();             // Get the private space.

  // ---- Save current time, time of previous call, and delay_loop
  time_prev_call = h.time_last_call;
  time_last_call = tao_get_monotonic_time();
  if (is_void(time_prev_call)) {
    h_set, h, time_last_call = time_last_call;
  } else {
    h_set, h, time_prev_call = time_prev_call,
              time_last_call = time_last_call,
              delay_loop = time_last_call - time_prev_call;
  }

  // ---- Exit if the window was closed.
  if (is_void(h.win) || !window_exists(h.win)) {
    _tao_visu_info, "tao_visu stopped by closing window";
    tao_visu_stop;
    return;
  }

  // ---- Exit requested with tao_visu_stop. Display last frame if available.
  if (! h.running) {
    if (is_void(h.counter)) {
      _tao_visu_display, "Display stopped";
    } else {
      _tao_visu_display, swrite(format="Display stopped at frame %d",
                                h.counter);
    }
    return;
  }

  // ---- Shortcut for display of wfs processing
  if (h.wfs_process) {
    wfs_process_ws = h.wfs_process_workspace;
    wfs_frame = wfs_process_ws.wfs_frame;
    wfs_data = wfs_process_ws.wfs_data;

    // ---- Get frame and data
    if (catch(-1)) {
      tao_ensure_unlocked, wfs_frame, wfs_data;
      _tao_visu_display, "WFS process: no frame. " + catch_message;
      after, h.delay_running, _tao_visu;   // Launch again in waiting mode.
    }
    tao_rdlock, wfs_frame;
    tao_rdlock, wfs_data;
    frame = wfs_frame.data;         // read frame.
    data = wfs_data.data;           // read data.
    tao_ensure_unlocked, wfs_frame, wfs_data;

    if (_tao_visu_is_an_image(frame)) {
      wfs_data = wfs_process_ws.wfs_data;

      if (!wfs_process_ws.wfs_layout_bak_done) {
        // save the current wfs layout and init a fake one.
        h_set, wfs_process_ws, wfs_layout_bak = h.wfs_layout,
                               wfs_layout_bak_done = 1n;
        xmin = int(data(1,));
        xmax = int(data(2,));
        ymin = int(data(3,));
        ymax = int(data(4,));
        dims = dimsof(frame);

        h_set, h, wfs_layout = h_new(
          __class=TAO_WFS_LAYOUT,
          ncols = 10, nrow = 10, nsubs = numberof(xmin),
          subsize = xmax(1) - xmin(1) + 1,
          xmin=xmin, ymin=ymin, xmax=xmax, ymax=ymax,
          x0 = min(xmin), y0 = min(ymin), x1 = max(xmax), y1 = max(ymax),
          xdim=dims(2), ydim=dims(3), mean=frame,
          layout_is_saved = 1);
      }
      if (h.wfs_slopes) {
        h_set, h, wfs_slopes_x = data(5,), wfs_slopes_y = data(6,);
      }
      if (h.wfs_error) {
        // FIXME: compute without any assumption on data received
        Wxx = data(7,);
        Wxy = data(8,);
        Wyy = data(9,);
        det = Wxx*Wyy - Wxy*Wxy;
        is_zero = (det == 0.);
        det = !is_zero / (det + is_zero)
        h_set, h, wfs_Cxx = Wyy * det,
                  wfs_Cxy = -Wxy * det,
                  wfs_Cyy = Wxx * det;
      }
      _tao_visu_exposure, frame;
      fps = _tao_visu_display_fps();       // Tune delay_running and get fps.
      counter = wfs_process_ws.counter;
      counter++;
      h_set, wfs_process_ws, counter = counter;
      if (is_void(fps)) {
        _tao_visu_display, swrite(format="WFS process: %d", counter);
      } else {
        _tao_visu_display, swrite(format="WFS process: %d (visu %.1f Hz)",
                                  counter, fps);
      }
    } else {
      _tao_visu_display, "WFS process: no frame received.";
    }
    after, h.delay_running, _tao_visu;   // Launch again in waiting mode.

//   } else if (h.display_this_frame) {
//     _tao_visu_save_cmin_cmax;
//     _tao_visu_wfs_display;
//     _tao_visu_restore_cmin_cmax;
//     h_delete, h, "delay_loop", "time_last_call"; // reset computation of fps.

  } else {

    // ---- Get state.
    cam = h.cam;
    state = is_void(cam) ? [] : cam.state;

    // ---- Server lost or not yet available. Wait for a new one.
    //
    if (is_void(state) || state == TAO_STATE_UNREACHABLE) {

      _tao_visu_display_scanned_servers;
      cam = _tao_visu_get_cam();             // Look for a new camera server.
      cam = _tao_visu_set_cam_in_ht(cam, h);
      if (!is_void(cam)) {
        _tao_visu_display, swrite(format="Found camera %s", h.cam_name);
        h_set, h, numberof_timeout = 0,      // Reset timeout/reset counters.
                  numberof_resetAndor = 0;
      }
      after, h.delay_waiting*2., _tao_visu;  // Launch again in waiting mode.

    // ---- Acquiring. Get a new frame and display.
    //
    } else if (state == TAO_STATE_WORKING) {

      // FIXME: get next image. Should get last image instead (make use of
      // cam.counter)
      // FIXME: could display all error messages
      //   extern tao_wait_image_reason
      //   img = tao_wait_image(cam, timeout=, noweights=1)
      //   if (is_void(img)) {
      //     error, tao_wait_image_reason;
      //   }
      frame = tao_wait_image(cam, timeout=h.timeout, noweights=1);

      if (is_void(frame)) {    // ------------ Timeout.

        h_set, h, numberof_timeout = h.numberof_timeout + 1;
        if (h.autoreset) {
          h_set, h, reset_level = h.reset_level + 1;
          _tao_visu_display, swrite(
                format="TIMEOUT! No frame from %s. Resetting server...(%d)",
                h.cam_name, h.reset_level);
          tao_visu_reset_cam;
        } else {
          _tao_visu_display, swrite(format="TIMEOUT! No more frame from %s",
                                    h.cam_name);
        }
        after, h.delay_waiting, _tao_visu;   // Launch again in waiting mode.

      } else {                 // ------------- New frame received.

        _tao_visu_exposure, frame;           // Get frame possibly averaged.
        h_set, h, reset_level = 0,           // Collect data.
                  counter = cam.serial;
        fps = _tao_visu_display_fps();       // Tune delay_running and get fps.
        if (is_void(fps)) {
          _tao_visu_display, swrite(format="%s : %d (cam %.1f Hz)",
                                    h.cam_name, h.counter, cam.framerate);
        } else {
          _tao_visu_display, swrite(format="%s : %d (visu %.1f Hz / cam %.1f Hz)",
                                    h.cam_name, h.counter, fps, cam.framerate);
        }
        after, h.delay_running, _tao_visu;   // Launch again in running mode.
      }

    // ---- Other states (sleeping or transient state).
    //
    } else {
      // Display last frame if available and the state in title.
      if (state == TAO_STATE_WAITING) {
        _tao_visu_display, swrite(format="Camera %s is %s (cam %.1f Hz)",
                                  h.cam_name, tao_get_state_name(state),
                                  cam.framerate);
        after, h.delay_waiting, _tao_visu;  // Sleeping: use a longer delay.
      } else {
        _tao_visu_display, swrite(format="Camera %s is %s", h.cam_name,
                                  tao_get_state_name(state));
        after, h.delay_running, _tao_visu;  // Transient: short delay.
      }
    }
  }
}


func _tao_visu_cbar(z, adjust=, cmax=, cmin=, color=, extrema=, font=, format=,
                    height=, nlabs=, opaque=, orient=, thickness=, ticklen=,
                    vert=, vport=, width=)
/* DOCUMENT  _tao_visu_cbar, z;
     - or -  _tao_visu_cbar, cmin=CMIN, cmax=CMAX;

    draws a color bar on the right of the current coordinate system. The colors
    and the associated label values are from min(Z) to max(Z) (first form) or
    from CMIN to CMAX by using the second form. The actual number of labels is
    automatically adjusted so that the corresponding values are rounded.

KEYWORDS:
    adjust=    - moves the bar closer to (adjust<0) or further from (adjust>0)
                 the viewport. The value is in NDC units. The current spacing
                 is 0.022 NDC for vertical color bar and 0.045 NDC for the
                 horizontal one.
    cmax=/cmin=- when specified, min/max of the colorbar is set to the given
                 value instead of computed from the given array Z.
    color=     - specifies the color of the labels, the ticks and the frame of
                 the color bar. Default is foreground color.
    extrema=   - when set to 1, the values of min and max are also displayed at
                 the bottom and top of the color bars. Default is to display
                 labels only at rounded values.
    font=      - changes the font used for the labels. Default is "helvetica".
    format=    - sets the format of the labels. Default is format= "%g".
    height=    - changes the font height of the labels. Default is 14 points.
    nlabs=     - sets the number of displayed labels; by default, nlabs=11
                 which correspond to a label every 10% of the dynamic. The
                 actual number of labels is automatically adjusted so that the
                 corresponding values are rounded. Use nlabs=0 to suppress all
                 labels.
    opaque=    - By default, opaque=0 and text is transparent. Set opaque=1 to
                 white-out a box before drawing the text.
    orient=    - changes the orientation of the labels. The default orient
                 (orient=0) is left-to-right text; set orient=1 for text
                 rotated 90 degrees so it reads upward, orient=2 for 180 degree
                 rotation so it is upside down, and orient=3 for 270 degree
                 rotation so it reads downward.
    thickness= - sets the thickness of the color bar (in NDC units). Default is
                 0.020 NDC.
    ticklen=   - sets the length (in NDC units) of the ticks. Default is 0.007
                 NDC.
    vert=      - vert=1 (default) sets the color bar on the right of the
                 viewport, and vert=0 sets it on the top.
    vport=     - Other viewport coordinates can be given by
                 vPORT=[xmin,xmax,ymin,ymax]. By default the current viewport
                 is used.
    width=     - sets the width of the lines used to draw the frame and the
                 ticks of the colorbar.

EXAMPLE:
         z = random_n(100,100);
         pli, z, cmin=0;
         _tao_visu_cbar, z, cmin=0, extrema=1;

    displays the random field clipped to zero, and shows the maximum value at
    the top of the color bar.

SEE ALSO: pli, font, height, opaque, orient
*/
{
  if (is_void(cmin)) {
    if (is_void(z)) error, "both z and cmin are void";
    cmin = min(z);
  } else if (!is_real(cmin) && !is_integer(cmin)) {
    error, "if given, cmin must be a number";
  }
  if (is_void(cmax)) {
    if (is_void(z)) error, "both z and cmax are void";
    cmax = max(z);
  } else if (!is_real(cmax) && !is_integer(cmax)) {
    error, "second argument must be a number";
  }
  if (cmin > cmax) swap, cmin, cmax;
  if (is_void(adjust)) adjust = 0.0;
  if (is_void(nlabs)) nlabs = 11;
  if (is_void(thickness)) thickness = 0.020;
  if (is_void(ticklen)) ticklen = 0.007;
  if (is_void(vport)) vport = viewport();  // [xmin,xmax,ymin,ymax]
  if (is_void(vert)) vert = 1n;

  local red, green, blue;
  palette, red, green, blue, query=1;
  ncolors = numberof(red);
  if (ncolors < 2) {
    ncolors = 240;
  }
  levs = span(cmin, cmax, ncolors + 1);
  cells = char(indgen(0 : ncolors - 1));

  linetype = 1; /* "solid" */

  if (vert) {
    x0 = vport(2) + adjust + 0.022;
    x1 = x0 + thickness;
    y0 = vport(3);
    y1 = vport(4);
    cells = cells(-,);
  } else {
    x0 = vport(1);
    x1 = vport(2);
    y0 = vport(3) - adjust - 0.045;
    y1 = y0 - thickness;
    cells = cells(,-);
  }
  sys = plsys(0);
  pli, cells, x0, y0, x1, y1;
  if (is_void(width) || width != 0) {
    plg, [y0,y0,y1,y1], [x0,x1,x1,x0], closed=1n,
      color=color, width=width, type=linetype, marks=0;
  }

  if (nlabs) {
    ticks = _tao_visu_ticks(cmin, cmax, nlabs, extrema=extrema);
    nlabs = numberof(ticks);

    local lx0, lx1, lx2, ly0, ly1, ly2;
    if (vert) {
      lx0 = array(x1, nlabs);
      lx1 = array(x1 + ticklen, nlabs);
      lx2 = array(x1 + 1.67*ticklen, nlabs);
      if (cmin == cmax) {
        ly0 = (y1-y0)/ncolors*(bytscl(cmin)+0.5) + y0;
      } else {
        ly0 = (y1-y0)/(cmax-cmin)*(ticks-cmin) + y0;
      }
      eq_nocopy, ly1, ly0;
      eq_nocopy, ly2, ly0;
      justify = "LH";
    } else {
      ly0 = array(y1, nlabs);
      ly1 = array(y1 - ticklen, nlabs);
      ly2 = array(y1 - 1.67*ticklen, nlabs);
      if (cmin == cmax) {
        lx0 = (x1-x0)/ncolors*(bytscl(cmin)+0.5) + x0;
      } else {
        lx0 = (x1-x0)/(cmax-cmin)*(ticks-cmin) + x0;
      }
      eq_nocopy, lx1, lx0;
      eq_nocopy, lx2, lx0;
      justify = "CT";
    }
    if (ticklen && (is_void(width) || width != 0)) {
      pldj, lx0, ly0, lx1, ly1, color=color, width=width, type=linetype;
    }
    if (is_void(format)) format= "%g";
    text = swrite(format=format, ticks);
    for (i = 1; i <= nlabs; ++i) {
      plt, text(i), lx2(i), ly2(i), tosys=0, color=color, font=font,
        height=height, opaque=opaque, orient=orient, justify=justify;
    }
  }
  plsys, sys;
}


func _tao_visu_connect_camera(cam)
/* DOCUMENT  obj = _tao_visu_connect_camera(arg)

    returns a TAO remote camera object OBJ or void if first argument CAM does
    not correspond to a TAO remote camera. CAM can be a TAO remote camera
    object, the shared memory identifier (shmid) of a TAO remote camera or the
    name of the remote camera.

SEE ALSO:
*/
{
  if (catch(-1)) {
    // Catch errors of type "0x10 interpreted errors (error)"
    // return void instead of crashing with error.
//     _tao_visu_error, catch_message;
    return;
  }
  is_shared = tao_is_shared_object(cam);
  if (is_shared) {
    if (is_shared == TAO_REMOTE_CAMERA) {
      return cam;
    } else {
      _tao_visu_error, "This remote object is not a remote camera";
      return;
    }
  } else {
    return tao_attach_remote_camera(cam);
  }
}


func _tao_visu_debug(text)
/* DOCUMENT  _tao_visu_debug, text

    displays the debug message given by TEXT, only if in debug mode.
*/
{
  if (h.debug) write, format="DEBUG: %s\n", text;
}


func _tao_visu_display(text)
/* DOCUMENT  _tao_visu_display, text

    displays the frame currently stored in tao_visu private space, with the
    given TEXT (scalar string) in the title. If no frame is available yet, an
    empty frame is displayed, possibly with the same size as the camera frame.

SEE ALSO: tao_visu
*/
{
  h = _tao_visu_get_private();         // Get the private space.
  cam = h.cam;
  allow_mark = 1n;
  allow_wfs = 1n;

  // ---- Get an image to display, anyway.
  //
  if (is_void(cam)) {
    if (is_void(h.frame)) {
      width = height = 1;
      h_set, h, frame = [[0.]];
      allow_wfs = allow_mark = 0n;
    } else {
      dims = dimsof(h.frame);
      width = dims(2);
      height = dims(3);
    }
  } else {
    // FIXME: tao_rdlock, cam (with catch ?)
    width = cam.width;
    height = cam.height;
    // FIXME: tao_unlock, cam
    dims = dimsof(h.frame);   // dims is void if h.frame is void.
    if (is_void(dims) || dims(1) != 2) {
      h_set, h, frame = array(double, width, height);
      allow_wfs = 0n;
    } else {
      cur_width = dims(2);
      cur_height = dims(3);
      if (cur_width != width || cur_height != height) {
        // The ROI was changed while the camera is waiting. Reset the image
        // only if the ROI is changed.
        h_set, h, frame = array(double, width, height);
        allow_wfs = 0n;
      }
    }
  }
  h_set, h, width = width, height = height;

  // ---- Plot
  //
  // - Only if the window is the current window.
  if (current_window() == h.win) {
    fma;
    x0 = y0 = 0.5;
    if (is_void(cam)) {
      // Display size.
      xytitles, swrite(format="size %dx%d", width, height);
    } else {
      xoff = cam.xoff;
      yoff = cam.yoff;
      if (h.sensorcoord) {
        x0 += xoff;
        y0 += yoff;
        xytitles, swrite(format="size %dx%d - exp %s", width, height,
                         _tao_visu_string_exposure(cam.exposuretime));
      } else {
        xytitles, swrite(format="offset %d,%d - exp %s", xoff, yoff,
                         _tao_visu_string_exposure(cam.exposuretime));
      }
    }
    // compute max/min if requested by autocmin, autocmax, cbar, or wfs_mask
    _tao_visu_display_cmin_cmax, allow_wfs;
    cmax = h.cmax;
    cmin = h.cmin;
    frame = [];
    eq_nocopy, frame, h.frame;

    // ---- Apply mask
    // If wfs_mask is set, cmin have already been computed by
    // _tao_visu_display_cmin_cmax taking mask into account.
    if (allow_wfs && h.wfs_mask) {
      frame(h.wfs_layout.mask_background_idx) = cmin;
    }

    if (allow_wfs && h.template) {
      _tao_visu_insert_template;
    }
    if (h.transpose) {
      frame = transpose(frame);
    }
    if (h.flip) {
      frame = frame(::-1,);
    }
    if (h.flop) {
      frame = frame(,::-1);
    }
    pli, frame, x0, y0, x0+width, y0+height, cmin=cmin, cmax=cmax;
    if (h.cbar) {
      _tao_visu_cbar, h.frame, cmin=cmin, cmax=cmax, extrema=1;
    }
    if (is_string(text)) pltitle, text;
    if (allow_mark) {
      if (!is_void(h.circle)) _tao_visu_plot_circle;
      if (!is_void(h.hline))  _tao_visu_plot_hline;
      if (!is_void(h.square)) _tao_visu_plot_square;
      if (!is_void(h.target)) _tao_visu_plot_target;
      if (!is_void(h.vline))  _tao_visu_plot_vline;
    }
    if (allow_wfs && h.plot_wfs) {
      _tao_visu_plot_wfs;
    }
  }
}


func _tao_visu_display_cmin_cmax(with_wfs)
/* DOCUMENT  _tao_visu_display_cmin_cmax

    updates the values of cmax and cmin when auto mode is activated (see
    tao_visu), i.e when flags autocmax and autocmin are set to a non-zero value
    in _tao_visu_private hash table. If autocmin is set to 1, the tracked value
    in the frame is the min (respectively the max for autocmax). If autocmin is
    set to a larger value, N, the tracked value is the Nth lower pixel value
    from the minimum (respectively the Nth greatest value from the maximum).
    cmax and cmin are considered separately, but in the same way. They are
    updated by smoothing the variations of the tracked value between frames
    with a leaking integrator:

        cmax = (n-1)/n * cmax + 1/n * max(frame),

    where n is the number of frames that are averaged. n is computed from the
    product of requested fps by the time constant (fps, cmin_time_cst, and
    cmax_time_cst in _tao_visu_private).

- take wfs_mask into account
- compute cmin if h.cbar, and wfs_mask
- compute cmax if h.cbar

SEE ALSO: tao_visu
*/
{
  h = _tao_visu_get_private();             // Get the private space.

  // ---- Return immediately if frame is void.
  pixels = [];
  eq_nocopy, pixels, h.frame;
  if (is_void(pixels)) return;             // no change in cmin and cmax.

  // ---- Apply mask
  need_wfs = with_wfs && !is_void(h.wfs_mask);
  if (need_wfs) {
    pixels = pixels(h.wfs_layout.mask_idx);
  }

  // ---- Initialiaze variables.
  n = numberof(pixels);
  autocmax = h.autocmax;
  autocmin = h.autocmin;
  cmax = h.cmax;
  cmin = h.cmin;

  // ---- autocmax
  if (autocmax > 0) {
    // autocmax converted to the position tracked in the sequence of the pixel
    // values sorted from min to max: position 1 is the min, and position n is
    // the max.
    autocmax = max(n-autocmax+1, 1);       // thus 1 <= autocmax <= n
    if      (autocmax == 1) new_cmax = min(pixels);
    else if (autocmax == n) new_cmax = max(pixels);
    else                    new_cmax = quick_select(pixels, autocmax);

    if (!autocmin && !is_void(cmin) && new_cmax < cmin) {
      new_cmax = cmin;             // If cmin is fixed, cmax cannot be smaller.
    }
    if (is_void(cmax) || (h.hoppingcmax && new_cmax > cmax)) {
      cmax = double(new_cmax);
    } else {
      g = min(h.delay_loop/h.cmax_time_cst, 1.); // fps = 1/h.delay_loop
      cmax = (1.-g) * cmax + g * double(new_cmax);
    }
  }

  // ---- autocmin
  if (autocmin > 0) {
    if (autocmax) {
      autocmin = min(autocmin, autocmax);  // autocmin <= autocmax
    } else {
      autocmin = min(autocmin, n);         // thus 1 <= autocmin <= n
    }
    if (autocmin == autocmax) new_cmin = new_cmax;
    else if (autocmin == 1)   new_cmin = min(pixels);
    else if (autocmin == n)   new_cmin = max(pixels);
    else                      new_cmin = quick_select(pixels, autocmin);

    if (!autocmax && !is_void(cmax) && new_cmin > cmax) {
      new_cmin = cmax;             // If cmax is fixed, cmin cannot be larger.
    }
    if (is_void(cmin) || (h.hoppingcmin && new_cmin < cmin)) {
      cmin = double(new_cmin);
    } else {
      g = min(h.delay_loop/h.cmin_time_cst, 1.); // fps = 1/h.delay_loop
      cmin = (1.-g) * cmin + g * double(new_cmin);
    }
  }

  // ---- cmax needed by cbar, cmin needed by cbar and wfs_mask.
  cbar = h.cbar;
  if (is_void(cmax) && cbar) {
    cmax = max(pixels);
  }
  if (is_void(cmin) && (cbar || need_wfs)) {
    cmin = min(pixels);
  }
  h_set, h, cmin=cmin, cmax=cmax;
}


func _tao_visu_display_fps(void)
/* DOCUMENT  fps = _tao_visu_display_fps()

    returns the current frame rate of the display, given in frames per second
    (i.e. Hz), or void if the frame rate cannot be computed yet. This measured
    frame rate is lower or equal to the camera frame rate. The value is
    computed by smoothing its variations with a moving average over the
    duration given by fps_time_cst computed with a leaking integrator:

        fps = (n-1)/n * fps + 1/n * (1/delay),

    where delay is the time between the two last frames, and n is the number of
    frames that are averaged. n is computed from the product of current fps by
    the time constant (fps, fps_time_cst in _tao_visu_private).

    The display will go as fast as possible. If a lower fps is requested (thus
    reducing the cpu load of the display), delay_running in _tao_visu_private
    hash table is also updated so that the effective frame rate of the display
    reaches the requested one (see tao_visu). The equation used is:

        tn = (1 - (t-t0)/t0) * t = (2 - t/t0) * t

    where tn is the updated value of the delay, t is the delay to be updated,
    and t0 is the requested delay, with t0 = 1/fps where fps is the requested
    fps.

SEE ALSO: tao_visu
*/
{
  h = _tao_visu_get_private();              // Get the private space.

  // ---- Ensure timing can be used.
  delay_loop = h.delay_loop;
  if (is_void(delay_loop) || delay_loop > h.delay_too_old) {
    return;
  }

  // ---- Tune delay_running if a given fps is requested.
  delay_running = h.delay_running;
  fps0 = h.fps_requested;                   // Requested fps, if defined.
  if (!is_void(fps0)) {
    // Tune delay_running so that fps gets closer to the requested fps.
    // The gain factor is (1 - (d-d0)/d0) = 2 - d/d0 = 2 - d*fps0,
    // where d is delay, and d0 = 1/fps0 is the requested delay.
    // delay_running is kept larger than delay_min.
    delay_running = max((2.0-delay_loop*fps0)*delay_running, h.delay_min);
    h_set, h, delay_running = delay_running;
  }

  // ---- Compute and return an averaged value of fps.
  fps = 1./delay_loop;
  prev_fps = h.fps;
  if (!is_void(prev_fps)) {
    g = min(delay_loop/h.fps_time_cst, 1.); // Gain for given time constant.
    fps = (1.-g) * prev_fps + g * fps;      // Moving average.
  }
  h_set, h, fps = fps;                      // Save for next frame.
  return fps;
}


func _tao_visu_display_this_frame(frame, time)
{
  return;
  // - load the frame to show.
  // - set a flag.
  // - cmin, cmax, set to a value.
  // - cmin, cmax, reset after.
  // - reset delay_loop.
}


func _tao_visu_display_scanned_servers(void)
/* DOCUMENT  _tao_visu_display_scanned_servers(void)

    displays successively all the TAO servers in the list of running servers
    but to which no connection are possible. For instance, a server can be dumb
    if tao_visu has no right access to it. If a server is accessible, tao_visu
    should connect automatically to it.

    A FIFO queue of running TAO servers is maintained so that the names of
    these servers can be displayed successively on the visu, as long as they
    are online. The queue is a 2-by-N array of strings, where N is the number
    of servers and, for each of them, the 2 elements are: name (i.e. owner),
    and user name.

SEE ALSO: _tao_visu_display, _tao_visu_get_servers
*/
{
  h = _tao_visu_get_private();             // Get the private space.

  // If tracked_servers is void, all the servers will be displayed.
  tracked_servers = is_string(h.stick_to_server) ? h.stick_to_server : [];

  no_server = [["",""]];                   // Trigger display "waiting".

  // ---- Update the queue of servers to display.
  servers_to_display = h.servers_to_display;
  servers_seen = _tao_visu_get_servers(tracked_servers);
  grow, servers_seen, no_server;           // "no_server" is always seen.
  names_seen = servers_seen(1,);

  if (is_void(servers_to_display)) {
    servers_to_display = no_server;
  }

  // ---- Remove no more seen servers from the list to display.
  names_to_display = servers_to_display(1,);
  n = numberof(names_to_display);
  new_to_display = [];
  for (i=1; i<=n; i++) {
    if (anyof(names_seen == names_to_display(i))) {
      grow, new_to_display, servers_to_display(,i)(,-);
    }
  }
  servers_to_display = new_to_display;
  if (is_void(servers_to_display)) {
    servers_to_display = no_server;
  }

  // ---- Append new seen servers in the list to display.
  names_to_display = servers_to_display(1,);
  n = numberof(names_seen);
  for (i=1; i<=n; i++) {
    if (noneof(names_to_display == names_seen(i))) {
      grow, servers_to_display, servers_seen(,i)(,-);
    }
  }

  // ---- Display the first in the list.
  name = servers_to_display(1,1);
  user = servers_to_display(2,1);

  if (name == "") {
    if (h.cam_already_seen) {

      if (is_string(h.stick_to_server)) {
        s = h.stick_to_server;
        _tao_visu_display, s+" server is lost. Waiting for it...";
      } else {
        _tao_visu_display, "Camera server lost. Waiting for a new one...";
      }

    } else {

      if (is_string(h.stick_to_server)) {
        s = h.stick_to_server;
        _tao_visu_display, "Waiting for "+s+" server to appear...";
      } else {
        _tao_visu_display, "Waiting for a camera server to appear...";
      }

    }

  } else {
    if (random() < 0.5) {
      _tao_visu_display, "Server "+name+" seen but is dumb (user "+user+")";
    } else {
      _tao_visu_display, "Server "+name+" doesn't talk to me (user "+user+")";
    }
  }

  // ---- Delete displayed server from the list to display.
  if (dimsof(servers_to_display)(0) == 1) {
    h_delete, h, "servers_to_display";
  } else {
    h_set, h, servers_to_display = servers_to_display(,2:);
  }
}


func _tao_visu_exposure(frame)
/* DOCUMENT  leframe = _tao_visu_exposure(frame)

    returns an update of the long exposure frame obtained with a moving average
    of the frames over the duration given by entry EXPOSURE (in seconds)
    stored in _tao_visu_private hash table. This average is computed with a
    leaking integrator:

        leframe = (n-1)/n * leframe + 1/n * frame,

    where FRAME is the new frame to be blended in the long exposure image
    LEFRAME, and N is the number of frames in the average. N is computed from
    the product of current fps by the time constant given by EXPOSURE
    keyword in seconds.

    The entry FRAME in _tao_visu_private is also replaced by the new returned
    estimation of LEFRAME.

SEE ALSO: tao_visu
*/
{
  h = _tao_visu_get_private();              // Get the private space.
  delay_loop = h.delay_loop;
  leframe = h.frame;
  exposure = h.exposure;

  // ---- Return frame if exposure not defined or if not able to compute.
  if (!exposure || is_void(leframe) || anyof(dimsof(frame) != dimsof(leframe))
      || is_void(delay_loop) || delay_loop > h.delay_too_old) {
    h_set, h, frame = double(frame);
    return frame;
  }

  // ---- Compute the long exposure frame.
  g = delay_loop / exposure;
  leframe = (1.-g) * leframe + g * frame;
  h_set, h, frame = leframe;
  return leframe;
}


func _tao_visu_error(text)
/* DOCUMENT  _tao_visu_warn, text

    displays the error message given by TEXT, with label "ERROR:"
*/
{
  write, format="ERROR: %s\n", text;
}


func _tao_visu_error_loop_count(void)
/* DOCUMENT  count = _tao_visu_error_loop_count()

    returns the number of error loops seen in less than the delay stored in
    delay_too_old entry in _tao_visu_private hash table. If COUNT is larger
    than a threshold, _tao_visu_relaunch_on_error will stop relaunching
    tao_visu if an error occurs. Errors can be raised independently of
    tao_visu, by another task, for instance in yorick terminal.

    The detection of the error loop is reset in function tao_visu with this
    line:

        h_set, h, error_start = [];

SEE ALSO: tao_visu, _tao_visu_relaunch_on_error
*/
{
  h = _tao_visu_get_private();          // Get the private space.

  // ---- Get times
  error_start = h.error_start;          // Time of first error, if any.
  now = tao_get_monotonic_time();

  // ---- Check if in error loop.
  if (is_void(error_start)) {           // First error seen for a while.
    h_set, h, error_start = now,        // Save the starting time.
              error_count = 0;          // Will count the loops from now.
    return 0;

  } else {                              // Not the first error seen.
    delay = now - error_start;          // Time from the first error.
    error_count = h.error_count + 1;    // Get and increment the counter.
    h_set, h, error_count= error_count; // Save the loop counter.

    if (delay > h.delay_too_old) {      // Delay is long enough to reset
      h_set, h, error_start= [],        // the error loop counter.
                error_count=0;
    }
    return error_count;
  }
}


func _tao_visu_get_cam(cam)
/* DOCUMENT  obj = _tao_visu_get_cam(cam)

    returns a TAO remote camera object OBJ or void if first argument CAM does
    not correspond to a TAO remote camera. CAM can be a TAO remote camera, the
    shared memory identifier (shmid) of a TAO remote camera, or the name of a
    camera server.

    If CAM is void, the behaviour depends on entry "stick_to_server" in hash
    table _tao_visu_private. If this entry is void, the function looks for the
    first available camera server returned by _tao_visu_get_servers(), and
    returns the corresponding remote camera object or void if none is found. If
    "stick_to_server" is set to the name of a remote camera, (e.g. "Andor0"),
    the function looks only for this camera and returns either the
    corresponding remote camera object or void if this camera is not yet
    available.

SEE ALSO: _tao_visu_get_servers, tao_visu
*/
{
  h = _tao_visu_get_private();                // Get the private space.

  if (is_void(cam)) {
    // Look for a camera to connect to.

    if (is_string(h.stick_to_server)) {
      // tao_visu is attached to a specific camera. Try a new connection.
      return _tao_visu_connect_camera(h.stick_to_server);

    } else {

      servers_seen = _tao_visu_get_servers(); // Available TAO servers.
      if (is_void(servers_seen)) {
        n = 0;
      } else {
        n = dimsof(servers_seen)(0);
      }
      for (i=1; i<=n; i++) {                  // Choose the first we can attach.
        name = servers_seen(1,i);
        cam = _tao_visu_connect_camera(name);
        if (!is_void(cam)) {
          return cam;
        }
      }
    }
  } else {
    return _tao_visu_connect_camera(cam);
  }
}


func _tao_visu_set_cam_in_ht(cam, ht)
/* DOCUMENT  cam = _tao_visu_set_cam_in_ht(cam, ht)

    updates hash table HT with informations from TAO remote camera object CAM
    given as first argument. If CAM is not a remote camera object, the
    entries related to CAM are cleaned up and the function returns void.
    These entries are:

      - cam, a reference to the TAO shared camera object,
      - cam_name, the short name of the camera as a scalar string, essentially
        for messages,
      - cam_already_seen, set to true to make tao_visu messages appropriate to
        the situation.

SEE ALSO: tao_visu
*/
{
  if (!is_hash(ht)) {
    error, "second argument must be a hash table";
  }
  if (tao_is_shared_object(cam) == TAO_REMOTE_CAMERA) {
    h_set, ht, cam = cam,
               cam_already_seen = 1n,
               cam_name = cam.owner;
    return cam;

  } else {
    h_set, ht, cam = [], cam_name = [];
    return;
  }
}


func _tao_visu_get_private(void)
/* DOCUMENT  h = _tao_visu_get_private(void)

    returns _tao_visu_private private hash table that contains all the
    parameters for the tao_visu_* functions, ensuring that the hash table is
    properly defined.

SEE ALSO: tao_visu
*/
{
  extern _tao_visu_private;

  if (!is_hash(_tao_visu_private)) {
    // _tao_visu_private is lost: reset private space and tao_visu uptime.
    include, _TAO_VISU_SOURCE, 1;
    h_set, _tao_visu_private, start_time_visu = tao_get_monotonic_time();

  } else if (current_window() == -1) {
    // No window yet: reset tao_visu uptime.
    h_set, _tao_visu_private, start_time_visu = tao_get_monotonic_time();
  }
  return _tao_visu_private;
}


func _tao_visu_get_servers(name)
/* DOCUMENT  servers = _tao_visu_get_servers()
     - or -  servers = _tao_visu_get_servers(name)

    returns the list of the running TAO camera servers, or void if none are
    found. First form returns all the TAO camera servers. Second form returns
    only the (first) one that matches the name (scalar string), e.g. "Andor0".

    The returned array of strings SERVERS is 2-by-N in size where N is the
    number of servers and, for each of them, the 2 elements are: name (i.e
    owner) and user name.
    For instance, if a server answer is "TAO Andor0 gs 7f000101:56611 michel",
    the 3 elements will be ["TAO:Andor0", "Andor0", "michel"]. This information
    is collected to inform on dumb servers while waiting for a TAO server to
    appear.

SEE ALSO: xpa_list
*/
{
  // ---- Get the list of servers (only remote cameras)
  servers=[];
  cmd =  "(for f in $(find /tmp/tao -type f); do "
  cmd += "echo $(basename \"$f\") $(stat -c %U \"$f\") :$(cat \"$f\"):; done)"
  cmd += " | egrep :[0-9]+:"
  f = popen(cmd, 0);

  serv = user = "";
  while (line= rdline(f)) {
    if (sread(line, serv, user) != 2) {
      error, "cannot read servers";
    } else {
      if (tao_attach_shared_object(serv).type == TAO_REMOTE_CAMERA) {
        grow, servers, [[serv, user]];
      }
    }
  }
  if (is_void(servers)) return;

  // ---- Return all or seeked ones.
  if (is_void(name)) {
    return servers;
  } else {
    return servers(, where(strmatch(servers(1,), name, 1)));
  }
}


func _tao_visu_get_snapshot(n)
/* DOCUMENT  img = _tao_visu_get_snapshot(n)

    acquires an image from the current camera obtained from N frames (default
    is 1). If N > 1, the returned image is averaged over N frames. The
    function returns void if no frame can be acquired, e.g. camera is not
    running or a timeout occured.

SEE ALSO: tao_visu
*/
{
  h = _tao_visu_get_private();        // Get the private space.

  if (is_void(n) || n <= 1) n = 1;
  statistics = (n==1 ? 0n : 1n);

  cam = h.cam;
  if (!is_void(cam) && cam.state != TAO_CAMERA_STATE_ACQUIRING) {
    _tao_visu_error, "cannot get frames since camera is not acquiring.";
    return;
  }
  // FIXME: tao_capture must be updated in tao-rt.i (2022.03.05).
  img = tao_capture(cam, n, statistics=statistics, timeout=1);
  if (is_void(img)) {
    _tao_visu_error, "no image obtained. Timeout ?";
  } else if (statistics) {
    img = img(,,1);                   // Keep average only.
  }
  return img;
}


func _tao_visu_info(text)
/* DOCUMENT  _tao_visu_info, text

    displays the informative message given by TEXT.
*/
{
  write, format="INFO: %s\n", text;
}


func _tao_visu_insert_template(void)
/* DOCUMENT  _tao_visu_insert_template

    inserts the template in the middle of the frame, if a template is
    available. The caller knows that a frame is available in _tao_visu_private
    and that such an insertion is possible.

SEE ALSO: tao_visu
*/
{
  h = _tao_visu_get_private();        // Get the private space.
  template = h.template_frame;

  if (is_void(template)) return;

  f_width = h.width;     // frame
  f_height = h.height;

  dims = dimsof(template);
  t_width = dims(2);
  if (t_width > f_width) return;
  t_height = dims(3);
  if (t_height > f_width) return;

  cx = f_width/2+1;         // Pixel at the center of frame.
  cy = f_height/2+1;
  x0 = cx - t_width/2;      // Range in the frame.
  x1 = x0 + t_width - 1;
  y0 = cy - t_height/2;
  y1 = y0 + t_height - 1;

  tmin = min(template);
  tmax = max(template);
  if (tmax != tmin) {
    a = (cmax-cmin)/(tmax-tmin);
    h.frame(x0:x1,y0:y1) = a * template + (cmin - a * tmin);
  } else {
    h.frame(x0:x1,y0:y1) = 0.5*(cmin+cmax);
  }
}


func _tao_visu_is_andor(void)
/* DOCUMENT  bool = _tao_visu_is_andor()

    returns true if an Andor camera is currently loaded in the tao_visu
    private space.

SEE ALSO: tao_visu
*/
{
  h = _tao_visu_get_private();        // Get the private space.
  cam_name = h.cam_name;
  return (is_string(cam_name) && strmatch(cam_name, "andor", 1));
}


func _tao_visu_is_a_cube(a)
/* DOCUMENT  bool = _tao_visu_is_a_cube(a)

    returns true if the argument is an numerical array of 3 dimensions.

SEE ALSO: tao_visu
*/
{
  return (is_numerical(a) && dimsof(a)(1) == 3);
}


func _tao_visu_is_an_image(a)
/* DOCUMENT  bool = _tao_visu_is_an_image(a)

    returns true if the argument is an image (2D array).

SEE ALSO: tao_visu
*/
{
  return (is_numerical(a) && dimsof(a)(1) == 2);
}


func _tao_visu_plot_circle(void)
/* DOCUMENT  _tao_visu_plot_circle

    plots a set of circles at centers [x_i,y_i] and with diameters d_i, where i
    is the index of a cercle. The values [x_i,y_i] and d_i are given as an
    2-D array [[x_1,y_1,d_1],[x_2,y_2,d_2],...] with keyword CIRCLE of command
    tao_visu and saved in _tao_visu_private with array reshaping if necessary.
    The function will crash if CIRCLE was not properly defined and checked
    before call.

SEE ALSO: tao_visu
*/
{
  local c;
  h = _tao_visu_get_private();        // Get the private space.
  eq_nocopy, c, h.circle;
  n = numberof(c)/3;
  for (i=1; i<=n; i++) {
    pl_circle, c(1,i), c(2,i), c(3,i)*0.5, number=24, color=h.circle_col;
  }
}


func _tao_visu_plot_square(void)
/* DOCUMENT  _tao_visu_plot_square

    plots a set of squares at centers [x_i,y_i] and with sides s_i, where i
    is the index of a square. The values [x_i,y_i] and s_i are given as an
    2-D array [[x_1,y_1,s_1],[x_2,y_2,s_2],...] with keyword SQUARE of command
    tao_visu and saved in _tao_visu_private with array reshaping if necessary.
    The function will crash if SQUARE was not properly defined and checked
    before call.

SEE ALSO: tao_visu
*/
{
  local s;
  h = _tao_visu_get_private();        // Get the private space.
  eq_nocopy, s, h.square;
  n = numberof(s)/3;
  for (i=1; i<=n; i++) {
    x = s(1,i);
    y = s(2,i);
    r = s(3,i)*0.5;
    xmin = x - r;
    xmax = x + r;
    ymin = y - r;
    ymax = y + r;
    plg, [ymin, ymin, ymax, ymax], [xmin, xmax, xmax, xmin], closed=1n,
         color=h.square_col;
  }
}


func _tao_visu_plot_hline(void)
/* DOCUMENT  _tao_visu_plot_hline

    plots a set of horizontal lines at ordinates [y1,y2,y3,...]. The ordinates
    are given with keyword HLINE of command tao_visu and saved in
    _tao_visu_private. The function will crash if HLINE was not properly
    defined and checked before call.

SEE ALSO: tao_visu
*/
{
  // ---- Get y.
  local y;
  h = _tao_visu_get_private();        // Get the private space.
  eq_nocopy, y, h.hline;
  n = numberof(y);

  // ---- Get x, with same number of elements as y.
  lim = limits();
  if (n == 1) {
    xmin = lim(1);
    xmax = lim(2);
  } else {
    xmin = array(lim(1), n);
    xmax = array(lim(2), n);
  }
  // ---- Plot.
  pldj, xmin, y, xmax, y, color=h.hline_col;
}


func _tao_visu_plot_target(void)
/* DOCUMENT  _tao_visu_plot_target

    plots a cross (with void center) at coordinates [x,y]. The vector [x,y] is
    given with keyword TARGET of command tao_visu and saved in
    _tao_visu_private. The size of the target is 25% of the size of the image.

SEE ALSO: tao_visu
*/
{
  local t;
  h = _tao_visu_get_private();        // Get the private space.
  if (!is_void(h.target)) {
    size = (h.width+h.height)*0.125;
    re = size*0.5;                    // external.
    ri = size*0.16;                   // internal.
    eq_nocopy, t, h.target;
    x = t(1);
    y = t(2);
    pldj, [x-re, x+ri,    x,    x],
          [   y,    y, y-re, y+ri],
          [x-ri, x+re,    x,    x],
          [   y,    y, y-ri, y+re],
          color=h.target_col, width=3;
  }
}


func _tao_visu_plot_vline(void)
/* DOCUMENT  _tao_visu_plot_vline

    plots a set of vertical lines at abscissae [x1,x2,x3,...]. The abscissae
    are given with keyword VLINE of command tao_visu and saved in
    _tao_visu_private. The function will crash if VLINE was not properly
    defined and checked before call.

SEE ALSO: tao_visu
*/
{
  // ---- Get x.
  local x;
  h = _tao_visu_get_private();        // Get the private space.
  eq_nocopy, x, h.vline;
  n = numberof(x);

  // ---- Get y, with same number of elements as x.
  lim = limits();
  if (n == 1) {
    ymin = lim(3);
    ymax = lim(4);
  } else {
    ymin = array(lim(3), n);
    ymax = array(lim(4), n);
  }
  // ---- Plot.
  pldj, x, ymin, x, ymax, color=h.vline_col;
}


func _tao_visu_plot_wfs(void)
/* DOCUMENT  _tao_visu_plot_wfs

    overplots the layout of the wavefront sensor, the flux meter, or the
    slopes, depending on the keywords wfs_show, wfs_tune, and wfs_slopes set
    with tao_visu. The layout shows the contours of the subimages, the flux
    meter shows circles whose area is proportional to the flux received by each
    subaperture, and the slopes are shown by crosses following the displacement
    of the subimages.

SEE ALSO: tao_visu
*/
{
  local xmin, xmax, ymin, ymax;    // subap. coordinates in the camera ROI.
  local x0, x1, y0, y1, cx, cy;    // subap. coordinates in the displayed area.
  local xprofile_coord, yprofile_coord;  // for wfs_profile.
  h = _tao_visu_get_private();     // Get the private space.

  layout = h.wfs_layout;

  if (!is_hash(layout)) return;

  // ---- Get the various needed coordinates.
  //
  // - update cache if necessary.
  cache = layout.cache;
  if (is_void(cache)) {
    cache = h_new();
    h_set, layout, cache = cache;
  }
  cam = h.cam;
  eq_nocopy, xmin, layout.xmin;    // Coordinates in the ROI, for addressing
  eq_nocopy, xmax, layout.xmax;    // pixels in the frame.
  eq_nocopy, ymin, layout.ymin;
  eq_nocopy, ymax, layout.ymax;

  // FIXME
  // if (catch());
  // tao_rdlock, cam;
  // cam_xoff = cam.xoff;
  // cam_yoff = cam.yoff;
  // tao_unlock, cam;
  if (!is_void(cam) && (h.sensorcoord != cache.sensorcoord
      || cam.xoff != cache.xoff || cam.yoff != cache.yoff)) {
    // Cache must be updated.
    xoff = cam.xoff;
    yoff = cam.yoff;
    sensorcoord = h.sensorcoord;
    h_set, cache, sensorcoord = sensorcoord, xoff = xoff, yoff = yoff;

    if (sensorcoord) {
      x0 = xmin + xoff;            // subap. coordinates in the displayed area.
      x1 = xmax + xoff;
      y0 = ymin + yoff;
      y1 = ymax + yoff;
      left = 1 + xoff;             // min x-coord of the displayed area.
      bottom = 1 + yoff;           // min y-coord of the displayed area.
    } else {
      eq_nocopy, x0, xmin;
      eq_nocopy, y0, ymin;
      eq_nocopy, x1, xmax;
      eq_nocopy, y1, ymax;
      left = 1;                    // min x-coord of the displayed area.
      bottom = 1;                  // min y-coord of the displayed area.
    }
    right = left +layout.xdim - 1; // max x-coord of the displayed area.
    top = bottom +layout.ydim - 1; // max y-coord of the displayed area.
    cx = 0.5*(x0+x1);              // subap. centers in the displayed area.
    cy = 0.5*(y0+y1);
    xprofile_coord = indgen(0:(right-left)) + left;   // for wfs_profile
    yprofile_coord = indgen(0:(top-bottom)) + bottom;

    h_set, cache, x0=x0, x1=x1, y0=y0, y1=y1, cx=cx, cy=cy,
                  left=left, right=right, bottom=bottom, top=top,
                  xprofile_coord=xprofile_coord, yprofile_coord=yprofile_coord;

  } else if (!is_void(cache.xoff)) {
    // Read in cache since there is something in it.
    eq_nocopy, x0, cache.x0;       // subap. coordinates in the displayed area.
    eq_nocopy, x1, cache.x1;
    eq_nocopy, y0, cache.y0;
    eq_nocopy, y1, cache.y1;
    eq_nocopy, cx, cache.cx;       // subap. coordinates in the displayed area.
    eq_nocopy, cy, cache.cy;
    left = cache.left;             // min x-coord of the displayed area.
    right = cache.right;           // max x-coord of the displayed area.
    bottom = cache.bottom;         // min y-coord of the displayed area.
    top = cache.top;               // max y-coord of the displayed area.
    eq_nocopy, xprofile_coord, cache.xprofile_coord;
    eq_nocopy, yprofile_coord, cache.yprofile_coord;

  } else {
    // Give up since no way to put something in cache, and nothing in cache.
    return;
  }

  n = numberof(x0);

  // ---- Display the subaperture locations.
  //
  if (h.wfs_show || h.wfs_tune) {
    for (i=1; i<=n; i++) {
      x0_i = x0(i);
      x1_i = x1(i);
      y0_i = y0(i);
      y1_i = y1(i);
      plg, [y0_i, y0_i, y1_i, y1_i],
           [x0_i, x1_i, x1_i, x0_i],
           closed=1n, color= h.wfs_col, type="dot", marks=0;
    }
  }
  // ---- Display flux in subimages
  //
  // - wfs_tune: as circles in the subapertures.
  // - wfs_flux: value of the relative flux.
  if (h.wfs_tune || h.wfs_flux) {
    flux = array(double, n);
    local frame;
    eq_nocopy, frame, h.frame;
    for (i=1; i<=n; i++) {
      flux(i) = sum(frame(xmin(i):xmax(i), ymin(i):ymax(i)));
    }
    flux *= 1./max(flux);
    if (h.wfs_tune) {
      pl_circle, cx, cy, (0.5*layout.subsize)*flux, number=12, color=h.wfs_col;
    }
    if (h.wfs_flux) {
      val = swrite(format="%.0f", flux*100.);
      for (i=1; i<=n; i++) {
        plt, val(i), cx(i), cy(i), justify="CH", tosys=1, color=h.wfs_col;
      }
    }
  }
  // ---- Display the sums of the pixels along the lines and the columns.
  //
  if (h.wfs_profile) {
    local frame;
    eq_nocopy, frame, h.frame;

//     xprofile_min = _tao_visu_sort_unique(xmin);
//     xprofile_max = _tao_visu_sort_unique(xmax);
//
//     coord = grow(xprofile_min, xprofile_max(0));
//     space = (xprofile_min(2:) - xprofile_max(:-1))*0.5;
//     coord(2:-1) -= space;
//     coord(1) -= space(1);
//     coord(0) += space(0);
//     xprofile_coord = coord(-:1:2,)(*); // duplicate the values.
//
//     n = numberof(xprofile_min);
//
//     xprofile = array(double, xprofile_nb*2+2);
//     p = frame(,sum);
//     k = 2;
//     for (i=1; i<=xprofile_nb; i++) {
//       flux = sum(p(xprofile_min(i):xprofile_max(i)));
//       xprofile(k) = flux;
//       xprofile(k+1) = flux;
//       k += 2;
//     }
//     xprofile *= double(top-bottom)/max(xprofile);
//     plg, xprofile + (bottom + 0.5), xprofile_coord, color="red", width=3;
//
//
//     yprofile_min = tao_visu_sort_unique(ymin);
//     yprofile_max = tao_visu_sort_unique(ymax);
//
    xprofile = frame(,sum);
    xprofile *= double(top-bottom)/max(xprofile);
    plg, xprofile + (bottom + 0.5), xprofile_coord, color="red", width=2;

    yprofile = frame(sum,);
    yprofile *= double(right-left)/max(yprofile);
    plg, yprofile_coord, yprofile + (left + 0.5), color="yellow", width=2;
  }

  // ---- Display slopes and/or measurement errors
  //
  if (h.wfs_process && (h.wfs_slopes || h.wfs_error)) {

    if (h.wfs_slopes && !is_void(h.wfs_slopes_x)) {
      // Keep cx, cy in the displayed area.
      // The given wfs_slopes_x and wfs_slopes_y are in absolute coordinates
      // in the displayed area.
      cx = min(max(left - 1 + h.wfs_slopes_x, left), right);
      cy = min(max(bottom - 1 + h.wfs_slopes_y, bottom), top);
      plp, cy, cx, symbol="+", color=h.wfs_col;
    }
    if (h.wfs_error && !is_void(h.wfs_Cxx)) {
      // Get the errors
      local Cxx, Cxy, Cyy;
      eq_nocopy, Cxx, h.wfs_Cxx;
      eq_nocopy, Cxy, h.wfs_Cxy;
      eq_nocopy, Cyy, h.wfs_Cyy;

      // Parameters of the ellipses
      a = 0.5 * atan(2.*Cxy, Cxx-Cyy);
      su = sqrt( max(Cxx*(cos(a))^2 + Cyy*(sin(a))^2 + Cxy*sin(2*a), 0.) );
      sv = sqrt( max(Cxx*(sin(a))^2 + Cyy*(cos(a))^2 - Cxy*sin(2*a), 0.) );

      // scale : a standard deviation of 0.5 pixel is scaled as 0.5 subimage
      // size, so that a diameter of 1 subimage means a standard deviation of
      // half a pixel: thus the subimage size is considered as a pixel size.
      scale = 1.0 * layout.subsize;
      pl_ellipse, cx, cy, su*scale, sv*scale, a*(180./pi), color=h.wfs_col;
    }
  }
}


func _tao_visu_relaunch_on_error(void)
/* DOCUMENT  _tao_visu_relaunch_on_error

    is called when an error occurs, preventing tao_visu to stop. But the use
    of after_error prevents entering debug mode after an error. The command
    "tao_visu, debugmode=1" will switch back to the standard behavior, but the
    display will need to be relaunched if an error occurs in yorick shell for
    instance. Entering "tao_visu, debugmode=0" gets back to the nonstop
    running.

SEE ALSO: after_error, _tao_visu_error_loop_count
*/
{
  extern after_error;
  h = _tao_visu_get_private();             // Get the private space.

  write, format="  %s\n",
         "Enter \"tao_visu, debugmode=1\" to enable yorick debug mode."

  loop_count = _tao_visu_error_loop_count();

  if (loop_count < 3) {
    after_error = _tao_visu_relaunch_on_error;
    if (h.running) after, h.delay_running, _tao_visu;

  } else {
    msg = "Display stopped: error loop detected!";
    _tao_visu_display, msg;
    write, format="\n%s\n", msg;
    after_error = [];                      // Will stop from now.
    if (h.running) after, h.delay_running, _tao_visu;
  }
}
errs2caller, _tao_visu_relaunch_on_error;


func _tao_visu_sort_unique(x)
/* DOCUMENT  x_sorted = _tao_visu_sort_unique(x)

    returns the unique values of X array sorted in ascending order.

SEE ALSO: sort
*/
{
  if (numberof(x) > 1) {              // Sort unique.
    x = x(sort(x));
    i_uniq = where(grow(1, x(:-1) != x(2:)));
    return x(i_uniq);
  } else {
    return x;
  }
}


func _tao_visu_string_exposure(time, format=, utf=)
/* DOCUMENT  s = _tao_visu_string_exposure(time)

    converts the scalar value TIME given in seconds to a string that specifies
    the time units (s, ms, or µ). The returned string is suitable for
    plotting, i.e. "µ" is returned as "!m" (see plt), unless keyword UTF is
    set to true. Keyword FORMAT can specify a format different from "%.1f".

SEE ALSO:
*/
{
  if (is_void(format)) {
    format = "%.1f";
  }
  time = double(time);
  if (time < 1e-3) {
    unit = utf ? "µs" : "!ms"
    return swrite(format=format+unit, time*1e6);
  }
  if (time < 1.) {
    return swrite(format=format+"ms", time*1e3);
  }
  return swrite(format=format+"s", time);
}


func _tao_visu_string_uptime(uptime)
/* DOCUMENT  s = _tao_visu_string_uptime(duration)

    returns a string of the form "h:mm:ss" corresponding to the given DURATION.

SEE ALSO:
*/
{
  t = max(lround(uptime),0);
  h = t/3600;
  t -= h*3600;
  m = t/60;
  s = t - m*60;
  return swrite(format="%d:%02d:%02d", h, m, s);
}


func _tao_visu_ticks(cmin, cmax, n, extrema=, outside=)
/* DOCUMENT  ticks = _tao_visu_ticks(cmin, cmax, n)

    returns a vector of rounded values between cmin and cmax that can be the
    values where we naturaly puts the ticks of a graph between cmin and cmax.
    The spacing between the ticks are 1, 2, or 5 units. The third argument, N,
    is the requested number of values. The actual number of ticks is as close
    as possible as the requested one, with the constrain of rounded values.

KEYWORDS:
    extrema= - when set to 1, the values of cmin and cmax are added
               respectively at the beginning and at the end of the returned
               vector. Any rounded value which is too close to the added cmin
               or cmax is discarded.

    outside= - Default is to return values within the interval [cmin, cmax]. If
               outside=1, extra ticks are added at the beginning and at the end
               of the returned vector, ouside the [cmin, cmax] interval.
               Keyword extrema is ineffective in this case.

SEE ALSO: span
*/
{
  if (cmin == cmax) return cmin;
  if (n <= 1.) return (cmin+cmax)*0.5;
  // Assume that the requested step is large enough so that the spacing
  // between the labels is at least the height of a label, so that we could
  // write a label between without superimposition of labels.
  s0 = abs(cmax-cmin)/max(n-1.,2.);  // requested step.
  p = 10^floor(log10(s0));           // power.
  d = s0/p;                          // digit.
  if (d < 1.5)      s =  1.0;        // ticks every 1, 2, 5.
  else if (d < 3.0) s =  2.0;
  else if (d < 7.0) s =  5.0;
  else              s = 10.0;
  s *= p;                            // effective step.
  if (outside) {
    return indgen(long(floor(cmin/s)):long(ceil(cmax/s))) * s;
  } else {
    ticks = indgen(long(ceil(cmin/s)):long(floor(cmax/s))) * s;
    if (extrema) {
      r = s*0.001;                   // Precision is limited at 0.1% of step.
      cmin = ceil(cmin/r)*r;         // Round cmin/cmax at this precision so
      cmax = floor(cmax/r)*r;        // to limit the number of digits displayed.
      emin = ticks(1) - cmin;
      emax = cmax - ticks(0);
      e = 0.5*s0;                    // smaller spacing without superimposition.
      if (emin < e) {                // not enough spacing for min.
        ticks(1) = cmin;             // replace first ticks.
        if (emax < e) {              // not enough spacing for max.
          ticks(0) = cmax;           // replace last ticks
          return ticks;
        } else {
          return grow(ticks, cmax);  // enough spacing for max: ticks added.
        }
      } else {
        if (emax < e) {              // not enough spacing for max.
          ticks(0) = cmax;           // replace last ticks
          return grow(cmin, ticks);  // enough spacing for min: ticks added.
        } else {
          return grow(cmin, ticks, cmax); // ticks added for min and max.
        }
      }
    } else {
      return ticks;
    }
  }
}


func _tao_visu_too_old(void)
/* DOCUMENT  bool = _tao_visu_too_old()

    returns true if the delay from time_last_call is larger than the value of
    delay_too_old. time_last_call and delay_too_old are stored in
    _tao_visu_private hash table.

SEE ALSO: tao_visu
*/
{
  h = _tao_visu_get_private();             // Get the private space.
  return ((tao_get_monotonic_time()-h.time_last_call) > h.delay_too_old);
}


func _tao_visu_warn(text)
/* DOCUMENT  _tao_visu_warn, text

    displays the warning message given by TEXT.
*/
{
  write, format="WARNING: %s\n", text;
}


func _tao_visu_wfs_is_blessed(wfs)
/* DOCUMENT  bool = _tao_visu_wfs_is_blessed(wfs)

    return true/false if WFS is a hash table of a class corresponding to
    wavefront sensor object.

SEE ALSO: tao_visu
*/
{
  return (is_hash(wfs) && wfs.__class == TAO_WFS_LAYOUT);
}


func _tao_visu_wfs_load(wfs, fromfile=)
/* DOCUMENT   _tao_visu_wfs_load, wfs

    loads a wavefront sensor (wfs) layout from either a TAO_WFS_LAYOUT object
    (a hash table), or a file that contains such an object.

KEYWORDS:
    fromfile= - If set to true or to a string (a file name), the wavefront
                sensor object loaded in wfs_layout has a supplementary entry
                layout_is_saved set to the original file or to true. It is
                used to inform the user if the layout is not yet saved.

SEE ALSO: tao_visu, _tao_visu_wfs_reset
*/
{
  h = _tao_visu_get_private();                 // Get the private space.

  if (is_hash(wfs)) {
    if (_tao_visu_wfs_is_blessed(wfs)) {
      // Here is the only way to load wfs_layout in h. Add index in it.
      h_set, wfs, mask_idx = where(wfs.mask != 0),
                  mask_background_idx = where(wfs.mask == 0);
      h_set, h, wfs_layout = wfs,
                wfs_show =  1n;
      if (fromfile) {
        h_set, wfs, layout_is_saved = fromfile;
        _tao_visu_info, "new WFS layout loaded from file.";
      } else {
        _tao_visu_info, "new WFS layout loaded.";
      }
      _tao_visu_wfs_not_saved;
      return wfs;
    } else {
      if (fromfile) {
        _tao_visu_error, "WFS layout not found in file.";
      } else {
        _tao_visu_error, "given object is not a WFS layout.";
      }
      return;
    }
  } else if (is_string(wfs)) {
    file = wfs;
    if (yhd_check(file)) {
      wfs = yhd_restore(file);
      return _tao_visu_wfs_load(wfs, fromfile=file);
    } else {
      _tao_visu_error, "file not found for WFS layout.";
      return;
    }
  } else {
    _tao_visu_error, "not a WFS layout object nor a file name.";
    return;
  }
}


func _tao_visu_wfs_not_saved(void)
/* DOCUMENT  _tao_visu_wfs_not_saved

    displays a reminder to use tao_visu_save_layout if the current WFS layout
    is not yet saved.

SEE ALSO: tao_visu, tao_visu_save_layout
*/
{
  h = _tao_visu_get_private();                 // Get the private space.
  layout = h.wfs_layout;
  if (_tao_visu_wfs_is_blessed(layout) && is_void(layout.layout_is_saved)) {
    _tao_visu_warn, "current WFS layout unsaved (see tao_visu_save_layout).";
  }
}


func _tao_visu_wfs_reset(wfs, fromfile=)
/* DOCUMENT  layout = _tao_visu_wfs_reset(wfs)

    computes a new wavefront sensor layout from an image found in the given
    WFS, which can be:
      - An image (array of 2 dimensions).
      - A fits file that contains such an image;
      - A WFS layout object (i.e. a hash table) with an entry wfs.mean equal to
        such an image. All the entries of the object will be recomputed from
        the image.
      - A yhd file (see yhd_save) that contains such a WFS layout object.

    The computation is done by calling function tao_wfs_layout_boxes. The
    function returns the new object, or void if no image is available or if a
    problem occured during the computation. Various messages inform the user
    on what is going on.

KEYWORDS:
    fromfile= - If set to true or to a string (a file name), the wavefront
                sensor object loaded in wfs_layout has a supplementary entry
                layout_is_saved set to the original file or to true. It is
                used to inform the user if the layout is not yet saved.

SEE ALSO: yhd_save, tao_wfs_layout_boxes
*/
{
  h = _tao_visu_get_private();                 // Get the private space.

  // ---- input is a file
  if (is_string(wfs)) {
    file = wfs;
    if (yhd_check(wfs)) {                  // ---- yhd file.
      wfs = yhd_restore(file);
      _tao_visu_info, "file \""+basename(file)+"\" loaded.";
      return _tao_visu_wfs_reset(wfs, fromfile=file);

    } else if (fits_check_file(wfs)) {     // ---- fits file.
      img = fits_read(file);
      _tao_visu_info, "array found in file \""+basename(file)+"\".";
      return _tao_visu_wfs_reset(img, fromfile=file);

    } else {                               // ---- unknown file.
      _tao_visu_error, "type of file \""+basename(file)+"\" unknown.";
      return;
    }

  // ---- input is a wfs object
  } else if (_tao_visu_wfs_is_blessed(wfs)) {
    img = wfs.mean;
    if (is_void(img)) {
      _tao_visu_warn, "no image in this object for resetting WFS layout.";
      return;
    } else {
      return _tao_visu_wfs_reset(img, fromfile=fromfile);
    }

  // ---- input is a cube of images
  } else if (_tao_visu_is_a_cube(wfs)) {
    _tao_visu_info, "first image extracted from the found array";
    return _tao_visu_wfs_reset(wfs(..,1), fromfile=fromfile);

  // ---- input is an image
  } else if (_tao_visu_is_an_image(wfs)) {
    img = wfs;
    _tao_visu_info, "image found for resetting WFS layout.";
    if (h.wfs_full_pup) {
      map = TAO_WFS_FULL_MAP;
    } else {
      map = TAO_WFS_DEFAULT_MAP;
    }
    wfs = tao_wfs_layout_boxes(img, map, margin=h.margin, no_error=1);
    if (_tao_visu_wfs_is_blessed(wfs)) {
      _tao_visu_info, "new WFS layout computed.";
      // ---- display useful values
      _tao_visu_info, swrite(format="WFS size= %gx%g, subsize= %d, spacing= %g",
                             wfs.x1-wfs.x0+1, wfs.y1-wfs.y0+1,
                             wfs.subsize, wfs.sp);
      pos = wfs.xmin;
      pos = pos(sort(pos));
      pos = pos(where(grow(1, pos(dif))));
      _tao_visu_info, swrite(format="x pos.: %s", strjoin(totxt(pos), ", "));

      pos = wfs.ymin;
      pos = pos(sort(pos));
      pos = pos(where(grow(1, pos(dif))));
      _tao_visu_info, swrite(format="y pos.: %s", strjoin(totxt(pos), ", "));

      _tao_visu_wfs_load, wfs, fromfile=fromfile;
      _tao_visu_display_this_frame, img, 3;  // during 3 seconds.
      return wfs;
    } else {
      _tao_visu_error, "WFS layout could not be computed from image."
      return;
    }
  } else {
    _tao_visu_error, "cannot reset from given object.";
    return;
  }
}

