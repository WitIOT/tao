/*
 * tao-ncpa.i --
 *
 * Yorick interface to TAO real-time software.  TAO is a library for Adaptive
 * Optics software
 *
 *-----------------------------------------------------------------------------
 *
 * This file if part of the TAO software (https://git-cral.univ-lyon1.fr/tao)
 * licensed under the MIT license.
 * Updates can be found in https://git-cral.univ-lyon1.fr/tao/tao-ncpa.
 *
 * Copyright (C) 2018-2022, Michel Tallon, Isabelle Tallon-Bosc, Éric Thiébaut.
 */

/* TODO:
 *
 * LIST OF POSSIBLE ERROR MESSAGES:
 *
 * error if DM already opened -> not sure.
 *
 * > dm = alpao_open("/home/michel/git/Themis_AO/config/BOL143");
 * ERROR (*main*) Cannot get IO interface status - Cannot call the driver. The
 * specified device was not found. Check that the specified device name exists.
 *
 *
 * 2019.07.28 -> this is when drivers are not mounted, after a reboot.
 * > dm = alpao_open("/home/michel/git/Themis_AO/config/BOL143");
 * ERROR (*main*) Invalid handler - The specified driver cannot open. Errors
 * occurred while the system opens the device.(For example, the work area in a
 * drive
 *
 * - update code where andor-server will support cam.sensorencoding.
 *
 * - the minimum allowed exposure time and the maximum framerate cannot be
 *   fetch from the camera [2019.07.21]. The camera should go at its maximum
 *   speed for a given exposure time.
 *
 */

/* SECTIONS
 * --------
 *
 * - CONFIGURATION
 *       - _tao_ncpa_private
 *
 * - PUBLIC FUNCTIONS
 *       - tao_ncpa
 *       - tao_ncpa_center_wfs
 *       - tao_ncpa_cmd_and_capture
 *       - tao_ncpa_dmshape
 *       - tao_ncpa_get / tao_ncpa_set
 *       - tao_ncpa_init_exposure
 *       - tao_ncpa_load_ref
 *       - tao_ncpa_rm_focus
 *       - tao_ncpa_rm_tilt
 *       - tao_ncpa_rm_tip
 *       - tao_ncpa_rm_tiptilt
 *       - tao_ncpa_roi_to_center
 *       - tao_ncpa_roi_to_max
 *
 * - COST FUNCTIONS
 *       - _tao_ncpa_normalize_var
 *       - _tao_ncpa_normalize_var_threshold
 *       - _tao_ncpa_normalize_var_threshold_worker
 *
 * - PRIVATE FUNCTIONS
 *       - _tao_ncpa_cam_configure
 *       - _tao_ncpa_cam_has_changed
 *       - _tao_ncpa_cam_start
 *       - _tao_ncpa_cam_stop
 *       - _tao_ncpa_check_fan
 *       - _tao_ncpa_check_influence_file
 *       - _tao_ncpa_check_pixeltype
 *       - _tao_ncpa_cmd_to_param
 *       - _tao_ncpa_compute_matrices_and_modes
 *       - _tao_ncpa_cost
 *       - _tao_ncpa_get_cam
 *       - _tao_ncpa_get_cam_config
 *       - _tao_ncpa_get_cmd
 *       - _tao_ncpa_get_dm
 *       - _tao_ncpa_get_private
 *       - _tao_ncpa_load_influence_maps
 *       - _tao_ncpa_load_matrices_and_modes
 *       - _tao_ncpa_param_to_cmd
 *       - _tao_ncpa_send_cmd
 *       - _tao_ncpa_set_cam_config
 *       - _tao_ncpa_wfs_cost
 *
 * - INPUT/OUTPUT PRIVATE FUNCTIONS
 *       - _tao_ncpa_cbar
 *       - _tao_ncpa_debug
 *       - _tao_ncpa_info
 *       - _tao_ncpa_plot_dm
 *       - _tao_ncpa_plot_img
 *       - _tao_ncpa_ticks
 *       - _tao_ncpa_warn
 *       - _tao_ncpa_yesno
*/

/*--+=^=+----------------------------------------------------------------------
 *                              CONFIGURATION
 *---------------------------------------------------------------------------*/

_TAO_NCPA_SOURCE = current_include();

if (!is_func(h_new))          include, "yeti.i", 1;
if (!is_func(pseudo_inverse)) include, "linalg.i", 1;
if (!is_func(tao_configure))  include, "tao-rt.i", 1;


if (!is_hash(_tao_ncpa_private)) {
  _tao_ncpa_private = h_new(
    // FIXME: these variables should be obtained from the configuration.
    // Currently hardcoded here and immediately copied in private space after.
    configuration = h_new(
      dm_name = "Alpao",     // name of the DM server.
      cam_name = "Andor0",   // name of the camera server.
      wfs_name = "Phoenix",  // name of the WFS camera server.
      dm_configuration = "/home/themis/Themis_AO/config/BOL143_zero_offsets",
      dm_influence_file = "/home/themis/Themis_AO/config/BOL143_InfFunctions_Cmd_0-5.fits",
    __=),
    data_keys = h_new(
        firstcommand =[],    // Quantities that can be obtained only with
        firstimage   =[],    // tao_ncpa_get, and set by computation (i.e they
        bestcommand  =[],    // are readonly).
        bestcost     =[],
        bestimage    =[],
        centercommand=[],
        refcommand=[]),
    always_defined = h_new(  // Quantities that are always defined so they
        cam_name=    [],     // can be read directly in _tao_ncpa_private.
        cost_fct=    [],
        delay=       [],
        dm_name=     [],
        filter_tilt= [],
        modal=       [],
        greedy=      [],
        retry=       [],
        rhobeg=      [],
        rhoend=      [],
        wfs_name=    [],
        windm=       [],
        winimg=      []),
    cost_fct     = symlink_to_name("_tao_ncpa_normalize_var"), // Cost function.
    cost_ws      = h_new(),  // Workspace for _tao_ncpa_cost.
    cam_keys     = h_new(
        exposuretime = [],   // Camera parameters that can be accessed by
        framerate    = [],   // tao_ncpa_get/tao_ncpa_set.
        height       = [],
        state        = [],
        width        = [],
        xoff         = [],
        yoff         = []),
    delay       = 0.01,   // [s] waiting time to get an image after command.
    dm_timeout  = 0.1,    // timeout for sending commands to the DM.
    filter_tilt = 1,      // filter out tilt mode.
    maxcmd      = 0.5,    // Maximum allowed value for any actuator.
    maxeval     = [],     // maximum number of evaluations of the cost fct
                          // for NEWUOA. Default value is computed from DM.
    minexposure = 10e-6,  // Minimum exposure time (should be asked to the cam).
    modal       = 0n,     // Select modal space.
    greedy      = 0,      // Select modal space with greedy optimization.
    retry       = 1,      // number of times the optimization is launched.
    rhobeg      = 0.02,   // starting value of rho for NEWUOA method.
    rhoend      = 0.0002, // Final value of rho for NEWUOA method.
    saturation  = [],     // Saturation level in ADU. Computed from camera
                          // sensorencoding.
    timeout     = 1.0,    // timeout for waiting for a new image.
    windm       = [],     // Window number for plotting dm; Default: no plot.
    winimg      = []      // Window number for plotting image; Default: no plot.
  );

  // ---- copy configuration to parent level.
  src = _tao_ncpa_private.configuration;
  keys = h_keys(src);
  for (i=numberof(keys); i>0; i--) {
    k = keys(i);
    h_set, _tao_ncpa_private, k, h_get(src, k); // no need to duplicate.
  }
}



/*--+=^=+----------------------------------------------------------------------
 *                             PUBLIC FUNCTIONS
 *---------------------------------------------------------------------------*/

func tao_ncpa(ground_cmd, cam=, cam_name=, delay=, dm=, dm_name=, exposuretime=,
              filter_tilt=, framerate=, height=, maxeval=, modal=, greedy=,
              cost_fct=, retry=, rhobeg=, rhoend=, roi=, saturation=, width=,
              windm=, winimg=, xoff=, yoff=)
/* DOCUMENT  tao_ncpa, x0, <key> = <value>, ...
     - or -  x = tao_ncpa(x0, <key> = <value>, ...)

    finds the command to be sent to the deformable mirror to maximize the image
    quality. This maximization is based on NEWUOA Powell's algorithm. The
    algorithm can be launched several times successively (see keyword RETRY).
    If the image gets saturated (see keyword SATURATION) at the end of a try,
    the exposure time is divided by two before the next retry. At the end of
    the optimization, the first, best, or last images and the corresponding
    commands and image quality value can be obtained by using tao_ncpa_get().

    If given, the first argument X0 is the starting value of the command.
    Default is to start from the command currently applied to the DM.
    Internally, the optimization is differential, i.e. done on the command dX
    to be added to X0 to get the best image quality. Anyway, the returned and
    saved values are the full commands X = X0+dX.

    The DM can be set to command zero before the optimization by using either

        > tao_ncpa_set, command=0;

      or

        > tao_reset, tao_ncpa_get("dm");

    The keywords <key> = <value> are directly passed to tao_ncpa_set. See
    tao_ncpa_set for the list of the available keywords. Keywords of interest
    are WINDM, WINIMG, MODAL, GREEDY, FILTER_TILT, RETRY, RHOBEG, and RHOEND.

    Before launching the optimization, set the exposure time as high as
    possible so that the maximum of the image is above half of the dynamics of
    the camera, and set the frame rate as high as possible for this exposure
    time. Function tao_ncpa_init_exposure is helpful for this task.

OTHER USEFUL FUNCTIONS:

    tao_ncpa_dmshape          - map of the DM surface for a given command.
    tao_ncpa_get              - get parameters and quantities for tao_ncpa.
    tao_ncpa_set              - set parameters for tao_ncpa.
    tao_ncpa_init_exposure    - optimize the exposure time of the camera.
    tao_ncpa_roi_to_center    - center the ROI on the given center.
    tao_ncpa_roi_to_max       - center the ROI on the max of the image.
    tao_ncpa_cmd_and_capture  - send a command and capture a cube of images.

EXAMPLES:

    We assume that the reference source is switched on and that a camera
    server Andor0 is running (see command "andor-server" or other available
    servers).

        > tao_ncpa_set, cam_name="Andor0", dm_name="Alpao";
        > tao_ncpa_roi_to_center, 1393, 1224, 200, 200;
        > tao_ncpa_init_exposure;
        > x = tao_ncpa(winimg=1, windm=2, retry=10);
        > pli, log(1 + tao_ncpa_get("bestimage"));

    The first line defines the camera server we want to work with. The second
    line connects to the camera server and defines a ROI 200x200 pixels in size
    and centered on pixel [1393, 1224]. The third line adjusts the exposure
    time so that the maximum of the image is higher than half the saturation
    level of the camera (see keyword SATURATION). The fourth line optimizes the
    image quality with default parameters, starting from the command currently
    applied to the deformable mirror, plotting the best images found on the
    road, showing the shape of the dm, and returns the best command as a vector
    of doubles. Last line retrieves the best image found so far and plot it in
    log scale.

        > tao_ncpa_set, command=0;
        > tao_ncpa_set, command=x;

    or

        > dm = tao_ncpa_get("dm");
        > tao_reset, dm;
        > tao_send_commands, dm, x, timeout;

    Previous example shows two different ways to reset the DM and send a
    command vector X to it. The second way directly gets a reference on the DM
    and allows commands for the TAO servers to be used.

        > x0 = tao_ncpa_get("bestcommand");
        > tao_ncpa_roi_to_max, 80, 80;
        > x = tao_ncpa(x0, winimg=1, windm=2, greedy=1, retry=1);
        > fits_write, "20190720-1533-ncpa_best_results.fits", x;

    The first line retrieves the best command previously found. The second line
    centers a 80x80 pixel ROI on the maximum of the image. The third line
    relaunches the optimization starting from previous best solution, in the
    modal space, with the modes added one at a time. Last line saves the result
    in a file.

SEE ALSO: tao_ncpa_set, tao_ncpa_get
*/
{
  // ---- Update the settings
  // FIXME: retain only the relevant keywords. Add them in the help.
  tao_ncpa_set, cam=cam, cam_name=cam_name, delay=delay, dm=dm, dm_name=dm_name,
                exposuretime=exposuretime, filter_tilt=filter_tilt,
                framerate=framerate, height=height, maxeval=maxeval,
                modal=modal, greedy=greedy, cost_fct=cost_fct, retry=retry,
                rhobeg=rhobeg, rhoend=rhoend, roi=roi, saturation=saturation,
                width=width, windm=windm, winimg=winimg, xoff=xoff, yoff=yoff;

  h = _tao_ncpa_get_private();      // Get the private space.
  cost_ws = h.cost_ws;              // Get the workspace of _tao_ncpa_cost().

  // ---- Check influence matrix is available and load if needed
  if (_tao_ncpa_check_influence()) {
    // Ensure the command-to-modes and modes-to-command matrices are loaded.
    _tao_ncpa_load_matrices_and_modes;
  }

  // ---- Save the current configuration of the camera
  // Save the current configuration of the camera (only the critical fields),
  // so that to check before each call to newuoa_maximize that this
  // configuration has not changed during the process. Also check that camera
  // state is acquiring.
  h_set, h, cam_config = _tao_ncpa_get_cam_config();
  if (h.cam_config.state != TAO_STATE_WORKING) {
    error, "camera is not acquiring (tao_start?)";
  }

  // ---- Check the fan of the camera
  // If the camera has a fan and the user ask stopping it, issue an error if
  // the fan cannot be stopped.
  //
  // FIXME: removed for FLI.
//   _tao_ncpa_check_fan, _tao_ncpa_get_cam();

  // ---- Check the pixel type (we want double)
  _tao_ncpa_check_pixeltype, _tao_ncpa_get_cam();

  // ---- Initialize ground_cmd
  dm = _tao_ncpa_get_dm();           // Ensure DM is opened (error if failure).
  if (is_void(ground_cmd)) {
    ground_cmd = _tao_ncpa_get_cmd();// Default is start from current command.
  } else if (numberof(ground_cmd) != dm.nacts) {
    error, swrite(format="given command must have %d values", dm.nacts);
  }
  if (tao_ncpa_get("filter_tilt")) {
    // Remove piston/tip/tilt from the ground_cmd. ptt_filter is initialized
    // by _tao_ncpa_load_matrices_and_modes called before.
    // - "filter_tilt" has just been allowed only if influence matrix is
    //    available.
    ground_cmd = h.ptt_filter(,+) * ground_cmd(+);
  }
  h_set, h, ground_cmd = ground_cmd; // Save the value for the cost function.

  // ---- Initialize x
  // Optimized part of the command that will be added to ground_cmd. Starting
  // point is always zero.
  x = array(double, dimsof(ground_cmd));

  // ---- Initialize maxeval
  // maxeval must be obtained with tao_ncpa_get to ensure its value is properly
  // defined, with a default value if necessary.
  maxeval = tao_ncpa_get("maxeval");

  // ---- Reset the trackers (since grow is used).
  h_delete, h, "track_bestcommand",  "track_bestcost",  "track_bestimage",
               "track_checkcommand", "track_checkcost", "track_checkimage",
               "track_costs",
               "track_firstcommand", "track_firstcost", "track_firstimage",
               "track_nevals",
               "track_saturation_nb_modes", "track_saturation_retry";

  globalbestcost = [];               // Best cost among all the retries.
  current_retry = 1;                 // Index of retry.
  stop_loop = 0n;                    // Will stop if camera config has changed.
  scale = [];                        // scale argument for newuoa_maximize

  // ---- Initialize nb_modes
  if (h.greedy) {
    // Maximum number of modes to use for the greedy method.
    nb_modes = min(h.greedy, dimsof(h.modes_to_cmd)(0));
  } else {
    // Not greedy, so all the parameters at once.
    nb_modes = 1;
  }

  // ---- Initialize the threshold of the cost function
  // - Only if _tao_ncpa_normalize_var is used.
  if (value_of_symlink(h.cost_fct) == _tao_ncpa_normalize_var) {
    // Get frames during 4s, but at least 4 frames and 300 frames at most.
    nframes = min(max(long(3 * tao_ncpa_get("framerate")), 4), 300);
    write, format="\n%s\n","----- Optimization of the threshold";
    write, format="  reading %d frames...\n",  nframes;
    c = _tao_ncpa_normalize_var_threshold(
            tao_ncpa_cmd_and_capture(ground_cmd, nframes));
    h_set, h, normalize_var_threshold = c;
    write, format="  threshold found: %g\n", c;
  }

  // ---- Loop on retry.
  while(current_retry <= retry && !stop_loop) {

    if (h.retry > 1) {
      write, format="\n----- retry %d\n", current_retry;
    }

    for (nb_active_modes=1; nb_active_modes <= nb_modes; nb_active_modes++) {

      if (_tao_ncpa_cam_has_changed()) {
        _tao_ncpa_warn, "Emergency stop: camera configuration has changed."
        stop_loop = 1n;
        break;
      }
      if (h.greedy) {
        write, format="\n    --- optimizing modes 1 - %d\n", nb_active_modes;
        h_set, h, nb_active_modes = nb_active_modes;
        scale = h.algebra.s(1:nb_active_modes);  // singular values.
      }
      // ---- Initialize workspace of _tao_ncpa_cost for this retry.
      h_set, cost_ws,
             neval = 0,              // Index of the evaluation.
             bestcost = [],          // This retry maximum value of cost func.
             title_shown = 0n,       // Set to true if title was shown.
             saturation_found = 0n;  // Set to true if saturation found.

      // ---- newuoa_maximize
      // With keyword all=1, returned object contents:
      //   r.fmax   = maximal function value found
      //   r.xmax   = corresponding parameters
      //   r.nevals = number of function evaluations
      //   r.status = status of the algorithm upon return
      //   r.rho    = final radius of the trust region for each variable

      // FIXME:
      // - should use scale with modes.
      // - choose err level:
      //    Keyword ERR sets the behavior in case of abnormal termination. If
      //    ERR=0, anything but a success throws an error (this is also the
      //    default behavior); if ERR > 0, non-fatal errors are reported by a
      //    warning message; if ERR < 0, non-fatal errors are silently ignored.
      x = newuoa_maximize(_tao_ncpa_cost, _tao_ncpa_cmd_to_param(x),
                          h.rhobeg, h.rhoend, scale,
                          maxfun=maxeval, all=0n, err=[]);
      x = _tao_ncpa_param_to_cmd(x); // get back to cmd space.
      full_cmd = ground_cmd + x;     // full command.

      // ---- Apply the best command and get the corresponding image.
      _tao_ncpa_send_cmd, full_cmd;  // Send best command.
      pause, lround(h.delay*1e3);    // delay in ms.
      image = tao_wait_image(h.cam, timeout=h.timeout, noweights=1);

      // ---- Save in trackers in _tao_ncpa_private.
      h_grow, h,
              // Number of evaluations of each retry, and costs found along the
              // maximization process. Collected in the _tao_ncpa_cost.
              "track_nevals",       cost_ws.neval,
              "track_costs",        &(cost_ws.allcosts(1:cost_ws.neval)),
              // Starting performances of each retry. Saved by the
              // _tao_ncpa_cost at first eval.
              "track_firstcommand", [cost_ws.firstcommand],
              "track_firstcost",    cost_ws.firstcost,
              "track_firstimage",   &cost_ws.firstimage,
              // Best values obtained within each retry. Recorded in the
              // _tao_ncpa_cost.
              "track_bestcommand",  [cost_ws.bestcommand],
              "track_bestcost",     cost_ws.bestcost,
              "track_bestimage",    &cost_ws.bestimage,
              // Same as best, but recomputed after NEWUOA has returned. In
              // principle, the performances should be very close (to be
              // checked).
              "track_checkcommand", [full_cmd],
              "track_checkcost",    h.cost_fct(image),
              "track_checkimage",   &image;

      // ---- Save the quantities of the starting point.
      if (current_retry == 1) {
        h_set, h, firstcommand = cost_ws.firstcommand,
                  firstimage = cost_ws.firstimage;
      }

      // ---- Save the quantities for the best performances.
      if (is_void(globalbestcost) || cost_ws.bestcost > globalbestcost) {
        globalbestretry = current_retry;
        globalbestcost = cost_ws.bestcost;
        globalbestretrymax = max(cost_ws.bestimage);
        h_set, h, bestcommand = cost_ws.bestcommand,
                  bestcost = globalbestcost,
                  bestimage = cost_ws.bestimage,
                  refcommand = dm.refcmds + cost_ws.bestcommand;
      }
      // ---- Display reminder of the best performances achieved so far, and
      //      the statistics on the command vector.
      write, format="\n  best at retry %d: quality=%7.5g, max= %d\n",
             globalbestretry,  globalbestcost, long(globalbestretrymax);
      write, format="  cmd DM: min= %.3f, max= %.3f, avg= %.2g, rms= %.2g\n",
             min(full_cmd), max(full_cmd), avg(full_cmd), full_cmd(rms);

      // ---- Reduce the exposure time if saturation was found.
      if ((retry > 1 || current_retry == retry) && cost_ws.saturation_found) {
        // Save in trackers.
        h_grow, h, "track_saturation_nb_modes", nb_active_modes,
                   "track_saturation_retry",    retry;

        exp = tao_ncpa_get("exposuretime");
        write, format="\nimage saturated with exposure time %.2g s\n", exp;
        exp *= 0.5;

        // FIXME: should ask the camera for its minimal exposure time.
        if (exp < h.minexposure) {
          _tao_ncpa_warn, "cannot reduce exposure time : minimum achieved."
          stop_loop = 1n;
          break;
        }
        tao_ncpa_set, exposuretime = exp;
        exp = tao_ncpa_get("exposuretime");
        write, format="     continuing with exposure time = %.2g s\n", exp;
        // FIXME: pause should not be necessary anymore.
        // Reload the new configuration. Wait 1s to give time for the camera to
        // change its state.
        // pause, 1000;
        h_set, h, cam_config = _tao_ncpa_get_cam_config();
        if (h.cam_config.state != TAO_STATE_WORKING) {
          error, "camera is not acquiring (tao_start?)";
        }
        h_set, cost_ws, saturation_found = 0n;
      }
    }
    current_retry++;
  }
  return full_cmd;
}


func tao_ncpa_center_wfs(wfs_layout, npix=)
/* DOCUMENT  tao_ncpa_center_wfs
     - or -  tao_ncpa_center_wfs, wfs_layout
     - or -  cmd = tao_ncpa_center_wfs()
     - or -  cmd = tao_ncpa_center_wfs(wfs_layout)

    drives the deformable mirror to center the subimages in the subapertures.
    If used on spots, they will be at the center of each subaperture at the end
    of the process. The command applied to the deformable mirror is stored and
    available with tao_ncpa_get("centercommand"). Third and fourth forms also
    return this command. The WFS layout (a TAO_WFS_LAYOUT object) can be either
    first loaded with "tao_ncpa_set, wfs_layout=..." (first and third forms),
    or given as an argument (second and fourth forms). In this later case, the
    given WFS layout will overwrite the current one.

KEYWORDS:
    npix= - number of pixels selected to compute the center of gravity in each
            subimage. Default value is 40 pixels (see tao_wfs_cog_on_spots).

EXAMPLES:
        > tao_ncpa_set, wfs_layout="filename"
        > tao_ncpa_center_wfs
        > cmd = tao_ncpa_get("centercommand");

        or

        > cmd = tao_ncpa_center_wfs("filename")

SEE ALSO: tao_ncpa_get, tao_ncpa_set, TAO_WFS_LAYOUT, tao_wfs_cog_on_spots
*/
{
  if (!is_func(tao_wfs_cog_on_spots)) include, "tao-wfs.i", 1;

  h = _tao_ncpa_get_private();      // Get the private space.

  // ---- Check that WFS layout is available
  if (!is_void(wfs_layout)) {
    tao_ncpa_set, wfs_layout = wfs_layout;
  }
  wfs_layout = tao_ncpa_get("wfs_layout");

  // ---- Initialize a workspace for _tao_ncpa_wfs_cost
  wfs_ws = h_new(npix = npix);
  h_set, h, wfs_ws = wfs_ws;

  // ---- Connect to WFS camera and save reference
  wfs = tao_attach_remote_camera(h.wfs_name);
  h_set, h, wfs = wfs;

  // ---- Load DM and current command.
  dm = _tao_ncpa_get_dm();
  cmd = _tao_ncpa_get_cmd();

  // ---- Compute the command that nullifies the slopes
  // - compute from current command.
  result = newuoa_minimize(_tao_ncpa_wfs_cost, cmd, h.rhobeg, h.rhoend,
                           all=1n, err=[]);
  cmd = result.xmin;
  _tao_ncpa_info, swrite(format="slopes centered with accuracy %g pix rms",
                         sqrt(result.fmin/(2*wfs_layout.nsubs)));
  _tao_ncpa_info, swrite(format="number of commands sent: %d", result.nevals);

  h_set, h, centercommand = cmd;
  return cmd;
}


func tao_ncpa_cmd_and_capture(cmd, nbimg)
/* DOCUMENT  cube = tao_ncpa_cmd_and_capture(cmd, nbimg)

    sends the command CMD to the deformable mirror, immediately acquires a
    set of NBIMG images, and returns these images as an array of
    NX-by-NY-by-NBIMG.

EXAMPLES:

    This example aims at the estimation of the delay between the sending the
    command and the image that sees the effect of this command.

        > tilt = tao_ncpa_get("tilt");
        > tip = tao_ncpa_get("tip");
        > cmd = tip + tilt;
        > cmd *= 1./max(abs(cmd));
        > cube = tao_ncpa_cmd_and_capture(cmd*0.5, 20);
        > i = 0;
        > _tao_ncpa_plot_img, cube(..,++i), win=1;
        > _tao_ncpa_plot_img, cube(..,++i), win=1;
        > ...

SEE ALSO:
*/
{
  h = _tao_ncpa_get_private();      // Get the private space.

  // ---- Check second argument.
  if (!is_integer(nbimg) || !is_scalar(nbimg) || nbimg <=0) {
    error, "second argument not a positive integer";
  }

  // ---- Ensure DM is opened and check the first argument.
  dm = _tao_ncpa_get_dm();          // Ensure DM is opened (error if failure).
  if (!is_numerical(cmd) || numberof(cmd) != dm.nacts) {
    error, swrite(format="expecting command vector with %d values.", dm.nacts);
  }

  // ---- Get a frame to assess type and dimensions.
  cam = _tao_ncpa_get_cam();        // Issue an error if cannot get cam object.
  frame = tao_wait_image(cam, timeout=h.timeout, noweights=1);

  // ---- Prepare the array to be returned.
  cube = array(structof(frame), dimsof(frame), nbimg);

  // ---- Get the cube.
  // FIXME: why not use tao_capture ?
  _tao_ncpa_send_cmd, cmd;
  for (i=1; i<=nbimg; i++) {
    cube(..,i) = tao_wait_image(cam, timeout=h.timeout, noweights=1);
  }
  return cube;
}


func tao_ncpa_dmshape(cmd, influence_maps)
/* DOCUMENT  map = tao_ncpa_dmshape(cmd)
     - or -  map = tao_ncpa_dmshape(cmd, influence_maps)

    returns the map of the deformable surface (i.e. a 2-D array) for the
    given command CMD. The size of CMD must match the last dimension of the
    influence matrix. If the second argument is not given (first form), the
    default influence matrix defined in _tao_ncpa_private is used.

    In the returned map, any NaN are replaced by zeros.

SEE ALSO: tao_ncpa, _tao_load_influence_maps
*/
{
  if (is_void(influence_maps)) {
    influence_maps = tao_ncpa_get("influence_maps");
  }
  if (numberof(cmd) != dimsof(influence_maps)(0)) {
    error, "command vector size does not match the influence matrix";
  }
  map = influence_maps(..,+)*cmd(+);
  map(where(map != map)) = 0.;
  return map;
}


local tao_ncpa_set;
func tao_ncpa_get(key)
/* DOCUMENT  tao_ncpa_set, <key> = <val>, ...
     - or -  val = tao_ncpa_get("<key>")

    sets/gets the values of the parameters for the NCPA minimization. The
    single argument for tao_ncpa_get is the name of the parameter given as a
    scalar string.

KEYWORDS:

    In the following list, the label (*) marks the keywords that can only used
    with tao_ncpa_get.

    cam=          - sets/gets the reference to the TAO shared camera device.
                    Will be automatically initialized when necessary. In
                    particular getting this value initializes the device.

    cam_name=     - sets/gets the name of the camera server.

(*) bestcommand=  - gets the command corresponding to the best image obtained
                    during the maximization process (all the retries). It is
                    available after the use of tao_ncpa. See refcommand.

(*) bestcost=     - gets the value of the cost function corresponding to the
                    best performance obtained during the maximization process
                    (all the retries).

(*) bestimage=    - gets the best image obtained during the maximization
                    process (all the retries).

(*) centercommand=- gets the command that sets the spots at the center of the
                    sub-apertures in the wavefront sensor. It is available
                    after the use of tao_ncpa_center_wfs.

    command=      - command to be sent to the DM or get from it. The device is
                    opened on the fly if necessary. In particular, setting
                    command=0 will reset the DM to its "flat" (i.e. zero)
                    position.

(*) cmd_to_modes= - gets the command-to-modes matrix.

    cost_fct=     - sets/gets the cost function to be used by the NEWUOA
                    algorithm which minimizes the NCPA.

    delay=        - waiting time in seconds between sending the command and
                    getting the image. Default value is 10 ms.

    dm=           - sets/gets a reference to the deformable mirror. A DM object
                    will be automatically initialized when necessary. In
                    particular getting this value will ensure a DM server is
                    running and will connect to it. If a DM object is already
                    available with reference <dm>, it can be loaded for ncpa
                    with tao_ncpa_set, dm=<dm>.

    dm_name=      - sets/gets the name of the DM server.

    exposuretime= - sets/gets the exposure time to/from the camera. (see
                    tao_configure and tao_attach_remote_camera). This field is
                    directly sent to/gotten from the camera.

(*) influence_maps= - gets the influence matrix of the DM.

    filter_tilt=  - if true, piston, tip and tilt are filtered out from the
                    command, instead of just piston. This allows the image to
                    be localized in the image plane so that the deformation of
                    the DM is minimized. Default is true.

(*) firstcommand= - gets the command used as the starting point of the
                    optimization process.

(*) firstimage=   - gets the image obtained with firstcommand.

(*) focus=        - gets the normalized command vector that corresponds to a
                    focus mode on the DM.

    framerate=    - sets/gets the framerate to/from the camera (see
                    tao_configure and tao_query_framerate). This field is
                    directly sent to/gotten from the camera.

    greedy=       - if true, the optimization is done in the same modal space
                    as with keyword MODAL, but the modes are added step by step
                    starting with the ones with the greatest singular value. At
                    each step, the optimization is done with all the modes of
                    the current subspace. Take care that in this case, one
                    retry will need N optimizations for N modes.

    height=       - sets/gets height to/from the camera (see tao_configure and
                    tao_attach_remote_camera). This field is directly sent
                    to/gotten from the camera.

    maxeval=      - maximum number of iterations of the NEWUOA algorithm.
                    Default value is 30 times the number of actuators.

    modal=        - if true, the optimization is done in a modal space
                    orthogonal to piston, tip, and tilt, so that it has 3
                    degrees of freedom less than the number of actuators. This
                    allows the image to be localized in the image plane so that
                    the deformation of the DM is minimized. Default is false.
                    See also "greedy".

(*) modes_to_cmd= - gets the modes-to-command matrix.

(*) piston=       - gets the normalized command vector that corresponds to a
                    piston mode on the DM.

(*) ptt_filter=   - gets the piston-tip-tilt projection matrix that filters out
                    modes piston, tip and tilt.

    retry=        - number of times the optimization is launched. Default is to
                    launch only once. If launched several times, a new launch
                    starts with the best command found at the previous try.

    rhobeg=       - starting value of rho for NEWUOA method. Typically RHOBEG
                    should be about one tenth of the greatest expected change
                    to a variable. Since the maximum absolute value of the
                    actuators is 1, the default value is set to 0.02.

    rhoend=       - final value of rho for NEWUOA method. RHOEND should
                    indicate the accuracy that is required in the final values
                    of the variables. Assuming the amplitude of the actuators
                    is 10 microns for maximum command equal to 1, default value
                    is set to rhobeg/100, corresponding to 20 nm for the RHOBEG
                    default value.

(*) refcommand=   - gets the reference command (i.e. the offsets to be loaded
                    into the DM) corresponding to the bestcommand and the best
                    image obtained during the maximization process (all the
                    retries). It is obtained by adding bestcommand to the
                    current reference command. It is available after the use of
                    tao_ncpa. See bestcommand.

    roi=          - region of interest as a vector of 4 elements equal to
                    [xoff, yoff, width, height]. This field is directly sent
                    to/gotten from the camera.

    saturation=   - level (ADU) corresponding to the saturation level. On a
                    CMOS camera, this level depends on the incident flux
                    (photons/s). If this level is reached, the function
                    automatically reduces the time exposure for the next try,
                    to avoid saturation. Default value is the 70% of the
                    maximum ADU value.

(*) tip=          - gets the normalized command vector that corresponds to a
                    tip mode on the DM.

(*) tilt=         - gets the normalized command vector that corresponds to a
                    tilt mode on the DM.

    width=        - sets/gets width to/from the camera (see tao_configure and
                    tao_query_height). This field is directly sent to/gotten
                    from the camera.

    windm=        - number of the window where to plot the shape of the DM
                    during the minimization of the NCPA. Setting WINDM to -1
                    cancels this display. Default is not to plot.

    winimg=       - number of the window where to plot the best image obtained
                    so far. Setting WINIMG to -1 cancels this display. Default
                    is not to plot.

    xoff=         - sets/gets xoff to/from the camera (see tao_configure and
                    tao_query_height). This field is directly sent to/gotten
                    from the camera.

    yoff=         - sets/gets yoff to/from the camera (see tao_configure and
                    tao_query_height). This field is directly sent to/gotten
                    from the camera.

SEE ALSO: tao_ncpa
*/
{
  if (!is_string(key) || !is_scalar(key)) {
    error, "first argument must be a scalar string";
  }
  h = _tao_ncpa_get_private();   // Get the private space.

  // ---- keywords always defined
  //
  // If key is an entry of _tao_ncpa_private that is always defined, directly
  // return its value.
  if (h_has(h.always_defined, key)) {
    return h_get(h, key);
  }

  // ---- keywords firstcommand, firstimage, bestcommand, etc.
  //
  if (h_has(h.data_keys, key)) {
    return h_get(h, key);
  }

  // ---- keywords related to cam
  //
  // If key is an entry related to cam, we ensure that cam is opened and all
  // these entries initialized in _tao_ncpa_private.
  if (key == "cam") {
    return _tao_ncpa_get_cam();
  }
  if (key == "cam_name") {
    return _tao_ncpa_get_cam().owner;
  }
  if (h_has(h.cam_keys, key)) {
    return h_get(_tao_ncpa_get_cam_config(), key);
  }
  if (key == "roi") {
    conf = _tao_ncpa_get_cam_config(); // Issue an error if cannot get cam.
    return [conf.xoff, conf.yoff, conf.width, conf.height];
  }

  // ---- cmd_to_modes
  if (key == "cmd_to_modes") {
    _tao_ncpa_load_matrices_and_modes;
    return h.cmd_to_modes;
  }

  // ---- command
  if (key == "command") {
    return _tao_ncpa_get_cmd();  // Get the current command from the dm device.
  }

  // ---- dm
  if (key == "dm") {
    return _tao_ncpa_get_dm();   // Issue an error if cannot get dm object.
  }

  // ---- dm_name
  if (key == "dm_name") {
    return _tao_ncpa_get_dm().owner; // Issue an error if cannot get dm object.
  }

  // ---- focus
  if (key == "focus") {
    _tao_ncpa_load_matrices_and_modes;
    return h.c_focus;
  }

  // ---- influence_maps
  if (key == "influence_maps") {
    return _tao_ncpa_load_influence_maps();
  }

  // ---- maxeval
  if (key == "maxeval") {
    if (is_void(h.maxeval)) {
      // Sets default value of maxeval (30 x number of actuators).
      dm = _tao_ncpa_get_dm();   // Issue an error if cannot get dm object.
      h_set, h, maxeval = 30 * dm.nacts;
    }
    return h.maxeval;
  }

  // ---- piston
  if (key == "piston") {
    _tao_ncpa_load_matrices_and_modes;
    return h.c_piston;
  }

  // ---- modes_to_cmd
  if (key == "modes_to_cmd") {
    _tao_ncpa_load_matrices_and_modes;
    return h.modes_to_cmd;
  }

  // ---- ptt_filter
  if (key == "ptt_filter") {
    _tao_ncpa_load_matrices_and_modes;
    return h.ptt_filter;
  }

  // ---- saturation
  //
  if (key == "saturation") {
    if (is_void(h.saturation)) {
      // Sets default value of saturation level (70% of the maximum ADU value
      // of the camera).
      cam = _tao_ncpa_get_cam();   // Issue an error if cannot get cam object.
      /* In tao.h:
       *
       * Pixel encoding is stored in 32-bit unsigned integer.
       *
       * The pixel encoding is a bitwise combination of various information
       * stored in a 32-bit unsigned integer:
       *
       * | Bits    | Description       |
       * | :-----: | :---------------- |
       * | 1-8     | Bits per pixel    |
       * | 9-16    | Bits per packet   |
       * | 17-24   | Color type        |
       * | 25-32   | Flags             |
       */
      // FIXME: Currently (2019.07.29) returns 12 bits instead of 16.
      // Thus default to 16 bits.
      depth = 16;
      // depth = cam.sensorencoding & 255;

      if (depth == 0) {
        // FIXME: remove particular case of sensorencoding == 0.
        // Not supported yet on andor-server [2019.07.21].
        depth = 16;
      }
      h_set, h, saturation = long((2^depth-1)*0.7);
    }
    return h.saturation;
  }

  // ---- tilt
  if (key == "tilt") {
    _tao_ncpa_load_matrices_and_modes;
    return h.c_tilt;
  }

  // ---- tip
  if (key == "tip") {
    _tao_ncpa_load_matrices_and_modes;
    return h.c_tip;
  }

  // ---- wfs_layout
  if (key == "wfs_layout") {
    return h.wfs_layout;
  }

  error, key + " is unknown";
}

/* Already documented. */

func tao_ncpa_set(cam=, cam_name=, command=, cost_fct=, debug=, delay=, dm=,
                  dm_name=, exposuretime=, filter_tilt=, framerate=, height=,
                  maxeval=, modal=, greedy=, retry=, rhobeg=, rhoend=, roi=,
                  saturation=, wfs_layout=, width=, windm=, winimg=,
                  xoff=, yoff=)
{
  h = _tao_ncpa_get_private();  // Get the private space.
  must_config_cam = 0n;         // Flag to know if cam must be reconfigured.

  // ---- cam
  if (!is_void(cam)) {
    if (tao_is_shared_object(cam) == TAO_REMOTE_CAMERA) {
      h_set, h, cam = cam;
    } else {
      error, "cam is not a TAO remote camera (see tao_attach_remote_camera)";
    }
  }

  // ---- cam_name
  if (!is_void(cam_name)) {
    if (is_string(cam_name) && is_scalar(cam_name)) {
      // Reset of h.cam will trigger a new connection to the new server:
      // see _tao_ncpa_get_cam().
      h_set, h, cam = [], cam_name = cam_name;
    } else {
      error, "cam_name must be a scalar string";
    }
  }

  // ---- command
  if (!is_void(command)) {
    _tao_ncpa_send_cmd, command;
  }

  // ---- cost_fct
  if (!is_void(cost_fct)) {
    if (is_func(cost_fct)) {
      h_set, h, cost_fct = cost_fct;
    } else {
      error, "cost_func is not a function";
    }
  }

  // ---- debug
  h_set, h, debug = debug;

  // ---- delay
  if (!is_void(delay)) {
    if (is_numerical(delay) && is_scalar(delay) && delay >= 0) {
      h_set, h, delay = delay;
    } else {
      error, "delay must be a positive number";
    }
  }

  // ---- dm
  if (!is_void(dm)) {
    if (tao_is_shared_object(dm) == TAO_REMOTE_MIRROR) {
      h_set, h, dm = dm, dm_name = dm.owner;
    } else {
      error, "dm is not a TAO remote mirror (see tao_attach_remote_mirror)";
    }
  }

  // ---- dm_name
  if (!is_void(dm_name)) {
    if (is_string(dm_name) && is_scalar(dm_name)) {
      // Reset of h.dm will trigger a new connection to the new server:
      // see _tao_ncpa_get_dm().
      h_set, h, dm = [], dm_name = dm_name;
    } else {
      error, "dm_name must be a scalar string";
    }
  }

  // ---- filter_tilt
  if (!is_void(filter_tilt)) {
    h_set, h, filter_tilt = (filter_tilt ? 1n : 0n);
  }

  // ---- greedy
  if (!is_void(greedy)) {
    if (is_integer(greedy) && is_scalar(greedy) && greedy >= 0) {
      if (greedy > 0) {
        h_set, h, greedy = greedy, modal = 1n;
      } else {
        h_set, h, greedy = greedy;
      }
    } else {
      error, "greedy must be a positive integer";
    }
  }

  // ---- maxeval
  if (!is_void(maxeval)) {
    if (is_integer(maxeval) && is_scalar(maxeval) && maxeval > 0) {
      h_set, h, maxeval = maxeval;
    } else {
      error, "maxeval must be a positive integer";
    }
  }

  // ---- modal
  if (!is_void(modal)) {
    h_set, h, modal = (modal ? 1n : 0n);
  }

  // ---- retry
  if (!is_void(retry)) {
    if (is_integer(retry) && is_scalar(retry) && retry > 0) {
      h_set, h, retry = retry;
    } else {
      error, "retry must be a positive integer";
    }
  }

  // ---- rhobeg & rhoend
  if (!is_void(rhobeg)) {
    if (is_numerical(rhobeg) && is_scalar(rhobeg) && rhobeg > 0) {
      if (is_void(rhoend)) {
        // Only rhobeg is given
        if (h.rhoend_was_given) {
          if (rhobeg >= h.rhoend) {
            h_set, h, rhobeg = rhobeg;
          } else {
            error, "rhobeg smaller than current rhoend";
          }
        } else {
          rhoend = rhobeg * 0.01;
          h_set, h, rhobeg = rhobeg, rhoend = rhoend;
          _tao_ncpa_warn, swrite(format = "rhoend updated to rhobeg/100 = %g",
                                 rhoend);
        }
      } else {
        // Both rhobeg and rhoend are given
        if (is_numerical(rhoend) && is_scalar(rhoend)
            && rhoend > 0 && rhoend <= rhobeg) {
          h_set, h, rhobeg = rhobeg, rhoend = rhoend, rhoend_was_given = 1n;
        } else {
          error, "rhoend larger than given rhobeg";
        }
      }
    } else {
      error, "rhobeg must be a positive number";
    }
  } else if (!is_void(rhoend)) {
    // Only rhoend is given
    if (is_numerical(rhoend) && is_scalar(rhoend)
        && rhoend > 0 && rhoend <= h.rhobeg) {
      h_set, h, rhoend = rhoend, rhoend_was_given = 1n;
    } else {
      error, "rhoend must be a positive number smaller than current rhobeg";
    }
  }

  // ---- saturation
  if (!is_void(saturation)) {
    if (is_integer(saturation) && is_scalar(saturation) && saturation > 0) {
      h_set, h, saturation = saturation;
    } else {
      error, "saturation must be a positive integer";
    }
  }

  // ---- wfs_layout
  if (!is_void(wfs_layout)) {
    h_set, h, wfs_layout = tao_wfs_get_layout(wfs_layout);
  }

  // ---- windm
  if (!is_void(windm)) {
    if (is_integer(windm) && is_scalar(windm) && windm >= 0 && windm <= 64) {
      h_set, h, windm = windm;
    } else {
      h_delete, h, "windm";
    }
  }

  // ---- winimg
  if (!is_void(winimg)) {
    if (is_integer(winimg) && is_scalar(winimg) && winimg >= 0 && winimg<=64) {
      h_set, h, winimg = winimg;
    } else {
      h_delete, h, "winimg";
    }
  }
  _tao_ncpa_set_cam_config, exposuretime=exposuretime, framerate=framerate,
                            height=height, width=width, roi=roi,
                            xoff=xoff, yoff=yoff;
}


func tao_ncpa_init_exposure(quiet=)
/* DOCUMENT  tao_ncpa_init_exposure

    tunes the exposure time and the frame rate of the camera in order the
    maximum of the image is between half the saturation level and the
    saturation level.

    Keyword QUIET set to true will prevent informations to be displayed.

SEE ALSO: tao_ncpa
*/
{
  h = _tao_ncpa_get_private(); // Get the private space.
  cam = _tao_ncpa_get_cam();   // Issue an error if cannot get cam object.
  high = tao_ncpa_get("saturation");
  low = high * 0.5;

  // FIXME: will ask to the camera when servers will implement that.
  max_exposuretime = 1.;
  min_exposuretime = 10e-6;
  max_framerate = 40;
  min_framerate = 2;

  while (1) {
    exp = tao_ncpa_get("exposuretime");
    fps = tao_ncpa_get("framerate");
    pause, 50;
    // Statistics of the maximum: adds average and 3 times its standard
    // deviation.
    stat = tao_capture(cam, long(fps+1), statistics=1, timeout=h.timeout)(*,);

// FIXME: use mxx.  max_idx = tao_wait_image(h.cam, timeout=h.timeout, noweights=1)(*)(mxx);

    mean = stat(,1);
    stdev = stat(,2);
    maximage = max(mean);
    maximage += 3.0 * stdev( (where(mean == maximage))(1) );

    if (!quiet) {
      write, format="max:%6d, exposure time: %.2g, framerate= %.2g\n",
             long(maximage), double(exp), double(fps);
    }
    if (maximage < low) {
      // exposure time must be increased. First decrease frame rate if
      // necessary.
      _tao_ncpa_debug, "increasing exposure time";
      exp *= 1.3;
      if (exp > 1.02/fps) {
        fps = 0.98/exp;
        if (fps < min_framerate) {
          break;
        } else {
          tao_ncpa_set, framerate = fps;
        }
      }
      tao_ncpa_set, exposuretime = exp;

    } else if (maximage > high) {
      // Exposure time must be decreased.
      _tao_ncpa_debug, "decreasing exposure time";
      tao_ncpa_set, exposuretime = exp * 0.7;

    } else {
      _tao_ncpa_info, swrite(format="max now in range %g - %g",
                             double(low), double(high));
      break;
    }
  }
}


func tao_ncpa_load_ref(cmd)
/* DOCUMENT  tao_ncpa_load_ref, cmd
     - or -  tao_ncpa_load_ref

    loads the reference command CMD into the deformable mirror and ensures that
    the command has been processed by the DM server. Second form loads the best
    reference command obtained by tao_ncpa and available with
    tao_ncpa_get("refcommand").

SEE ALSO: tao_ncpa, tao_ncpa_get
*/
{
  // ---- Initialize
  h = _tao_ncpa_get_private();      // Get the private space.
  dm = _tao_ncpa_get_dm();          // Issue an error if cannot get dm object.

  // ---- Get and check the reference command.
  if (is_void(cmd)) {
    // From _tao_ncpa_private.
    cmd = tao_ncpa_get("refcommand");
    if (is_void(cmd)) {
      _tao_ncpa_warn,"Please use tao_ncpa to compute the reference command\n"+
                     "before to load it in the DM";
      return;
    }
  } else if (! is_numerical(cmd) || numberof(cmd) != dm.nacts) {
    // Check the one given as an argument.
    error, swrite(format="command must be 0 or a %d-element vector", dm.nacts);
  }
  // ---- Load the reference command into the DM
  serial = tao_set_reference(dm, cmd, h.dm_timeout);
  tao_wait_command, dm, serial, h.dm_timeout;
  tao_reset, dm;
}


func tao_ncpa_rm_focus(cmd)
/* DOCUMENT  c = tao_ncpa_rm_focus(cmd)

    removes the focus component from command CMD and returns the result. The
    command that shapes the deformable mirror with a focus mode can be obtained
    with tao_ncpa_get("focus").

SEE ALSO: tao_ncpa_get, tao_ncpa_rm_tip, tao_ncpa_rm_tilt, tao_ncpa_rm_tiptilt
*/
{
  mode = tao_ncpa_get("focus");
  if (numberof(cmd) != numberof(mode)) {
    error, "given command has a wrong number of elements";
  }
  return cmd - sum(cmd*mode)*mode;
}


func tao_ncpa_rm_tilt(cmd)
/* DOCUMENT  c = tao_ncpa_rm_tilt(cmd)

    removes the tilt component from command CMD and returns the result. The
    command that shapes the deformable mirror with a tilt mode can be obtained
    with tao_ncpa_get("tilt").

SEE ALSO: tao_ncpa_get, tao_ncpa_rm_tiptilt, tao_ncpa_rm_tilt,
          tao_ncpa_rm_focus
*/
{
  mode = tao_ncpa_get("tilt");
  if (numberof(cmd) != numberof(mode)) {
    error, "given command has a wrong number of elements";
  }
  return cmd - sum(cmd*mode)*mode;
}


func tao_ncpa_rm_tip(cmd)
/* DOCUMENT  c = tao_ncpa_rm_tip(cmd)

    removes the tip component from command CMD and returns the result. The
    command that shapes the deformable mirror with a tip mode can be obtained
    with tao_ncpa_get("tip").

SEE ALSO: tao_ncpa_get, tao_ncpa_rm_tiptilt, tao_ncpa_rm_tilt,
          tao_ncpa_rm_focus
*/
{
  mode = tao_ncpa_get("tip");
  if (numberof(cmd) != numberof(mode)) {
    error, "given command has a wrong number of elements";
  }
  return cmd - sum(cmd*mode)*mode;
}


func tao_ncpa_rm_tiptilt(cmd)
/* DOCUMENT  c = tao_ncpa_rm_tiptilt(cmd)

    removes the tip and tilt components from command CMD and returns the
    result. The command that shapes the deformable mirror with a tip or a tilt
    mode can be obtained with tao_ncpa_get("tip") or tao_ncpa_get("tilt").

SEE ALSO: tao_ncpa_get, tao_ncpa_rm_tip, tao_ncpa_rm_tilt, tao_ncpa_rm_focus
*/
{
  return tao_ncpa_rm_tilt(tao_ncpa_rm_tip(cmd));
}


func tao_ncpa_roi_to_center(xc, yc, width, height)
/* DOCUMENT  tao_ncpa_roi_to_center, xcenter, ycenter
     - or -  tao_ncpa_roi_to_center, xcenter, ycenter, width, height

    sets the ROI (Region Of Intereset) of the camera to a window
    WIDTH-by-HEIGHT centered on the given XCENTER, YCENTER coordinates. WIDTH
    and HEIGHT default to current values.

    Beware that this operation stop/configure/restart the camera.

SEE ALSO: tao_ncpa, tao_ncpa_roi_to_max
*/
{
  // ---- Check width and height (any argument can be void).
  if (!is_void(width) &&
      !(is_integer(width) && is_scalar(width) && width > 0)) {
    error, "width must be a positive integer";
  }
  if (!is_void(height) &&
      !(is_integer(height) && is_scalar(height) && height > 0)) {
    error, "height must be a positive integer";
  }

  // ---- Get current camera configuration
  conf = _tao_ncpa_get_cam_config(); // Issue an error if cannot get cam.
  xoff_now = conf.xoff;
  yoff_now = conf.yoff;
  width_now = conf.width;
  height_now = conf.height;

  // ---- Check xc and yc and deal with defaults.
  if (is_void(xc)) {
    xc = xoff_now + width_now/2 + 1;
  } else if (!is_integer(xc) || !is_scalar(xc) || xc <= 0) {
    error, "first argument must be a positive integer";
  }
  if (is_void(yc)) {
    yc = yoff_now + height_now/2 + 1;
  } else if (!is_integer(yc) || !is_scalar(yc) || yc <= 0) {
    error, "second argument must be a positive integer";
  }

  // ---- Deal with the default values of width and height.
  // - width defaults to current value.
  // - height defaults to current value or same as width if width is given.
  if (is_void(width)) {
    width = width_now;
    if (is_void(height)) {
      height = height_now;
    }
  } else if (is_void(height)) {
    height = width;
  }

  // ---- Compute new xoff and yoff.
  xoff = max(xc - width/2 - 1, 0);
  yoff = max(yc - height/2 - 1, 0);
  _tao_ncpa_set_cam_config, xoff=xoff, yoff=yoff, width=width, height=height;
}


func tao_ncpa_roi_to_max(width, height)
/* DOCUMENT  tao_ncpa_roi_to_max, width, height

    sets the ROI (Region Of Intereset) of the camera to a window
    WIDTH-by-HEIGHT centered on the maximum of the image. WIDTH and HEIGHT
    default to current values.

    Beware that this operation stop/configure/restart the camera.

SEE ALSO: tao_ncpa, tao_ncpa_roi_to_center
*/
{
  if (!is_void(width)
      && !(is_integer(width) && is_scalar(width) && width > 0)) {
    error, "width must be a positive integer";
  }
  if (!is_void(height)
      && !(is_integer(height) && is_scalar(height) && height > 0)) {
    error, "height must be a positive integer";
  }

  roi = tao_ncpa_get("roi");           // Ensure camera is opened.
  xoff = roi(1);
  yoff = roi(2);
  xsize = roi(3);
  ysize = roi(4);

  h = _tao_ncpa_get_private();         // Get the private space.
  max_idx = tao_wait_image(h.cam, timeout=h.timeout, noweights=1)(*)(mxx);

  yc = (max_idx-1) / xsize;
  xc = max_idx - yc*xsize;
  yc += 1;
  _tao_ncpa_info, swrite(format="max found at (%d, %d)", xc+xoff, yc+yoff);

  if (is_void(width))  width = xsize;
  if (is_void(height)) height = ysize;
  xoff += xc - width/2;
  yoff += yc - height/2;
  _tao_ncpa_set_cam_config, xoff=xoff, yoff=yoff, width=width, height=height;
}


/*--+=^=+----------------------------------------------------------------------
 *                               COST FUNCTIONS
 *---------------------------------------------------------------------------*/

func _tao_ncpa_normalize_var(img, threshold)
/* DOCUMENT  cost = _tao_ncpa_normalize_var(img, threshold)

    computes the cost value to be maximized as the normalized variance of the
    pixel values: Var(img)/Avg(img)^2, where Var(img) is the variance of the
    pixels and Avg(img) is their average. If given, THRESHOLD is a constant
    that corrects for the bias on the average; the computed value is:

                Var(img)
        -----------------------
        (Avg(img) - Treshold)^2

   If not given, THRESHOLD is set equal to entry normalize_var_threshold in
   _tao_ncpa_private, or zero if this entry is not defined. See
   _tao_ncpa_normalize_var_threshold that computes the optimal value.

SEE ALSO: _tao_ncpa_cost, _tao_ncpa_normalize_var_threshold
*/
{
  h = _tao_ncpa_get_private();         // Get the private space.
  if (!is_void(threshold)) {
    c = threshold;
  } else {
    c = h.normalize_var_threshold;
    if (is_void(c)) c = 0.;
  }
  img = double(img(*));
  return (img(rms)/(img(avg)-c))^2;
}


func _tao_ncpa_normalize_var_threshold(cube)
/* DOCUMENT  threshold = _tao_ncpa_normalize_var_threshold(cube)

    returns the threshold that minimizes the dependency of the normalized
    variance criterium. CUBE is an array Nx-by-Ny-by-NIMG, where NIMG is the
    number of images of size Nx-by-Ny. The function first selects 75% of the
    images with the larger average. On this sample, it finds the value of the
    constant X that minimizes the variance of the cost function :

             Var(img)
        ---------------- ,
        (Avg(img) - X)^2

    i.e. the constant that makes the criterium the most indepedent from a
    variation of the flux.

SEE ALSO: _tao_ncpa_normalize_var
*/
{
  // ---- Spatial average and standard deviations of the images
  //
  // - Compute average flux and select the 75% of the images with the greatest
  //   average.
  dims = dimsof(cube(..,1));        // needed after.
  cube = double(cube(*,));
  mean = cube(avg,);
  isel = where(mean > quick_quartile(mean)(1));
  // isel = where(mean > median(mean));
  mean = mean(isel);
  stdev = cube(rms,isel);

  // ---- First estimation of the optimum
  //
  // Consider the median of the pixels of the periphery of the images, 2
  // pixels in width.
  mask = array(0, dims);
  mask(1:2,) = mask(-1:0,) = 1;
  mask(,1:2) = mask(,-1:0) = 1;
  isel = where(mask);
  med = median(cube(isel,)(*));

  // ---- Minimization
  f = closure(_tao_ncpa_normalize_var_threshold_worker, [stdev, mean]);
  a = med - 1.;
  b = med + 1.;
  // fourth argument = 3 means the minimum is searched in the interval (A,B) --
  // i.e. the location of the minimum X is such that: min(A,B) < X < max(A,B).
  // fmin should never evaluates F at the given bounds.

  // fourth argument = 0 means there is no bounds for X: F is first evaluated
  // at A and B, then the interval of search is enlarged until a minimum is
  // bracketed.
  if (catch(0x01)) {
    // 0x01 catcheds math errors (SIGFPE, math library).
    _tao_ncpa_warn, "cannot find min; use median instead";
    return med;
  }
  return fmin(f, a, b, 0, tol=0.01);
}

func _tao_ncpa_normalize_var_threshold_worker(data, x)
{
  return ((data(,1)/(data(,2)-x))^2)(rms)^2;
}



/*--+=^=+----------------------------------------------------------------------
 *                              PRIVATE FUNCTIONS
 *---------------------------------------------------------------------------*/

func _tao_ncpa_cam_configure(cam, exposuretime=, fanspeed=, framerate=,
                             pixeltype=, xoff=, yoff=, width=, height=)
/* DOCUMENT  _tao_ncpa_cam_configure, cam, key=val, ...

    configures the camera cam (a remote camera object) from the given keywords
    and waits until the configuration is actually sent to the camera. Issue an
    error if the timeout is reached while waiting for the camera to configure.

KEYWORDS:
    exposuretime= - exposure duration (in seconds).

    framerate=    - number of frames per second.
    height=       - number of lines in acquired images.
    pixeltype=    - change the pixeltype requested from the camera server.
                    The value can be char, short, int, long, float or double.
    width=        - length of lines in acquired images.
    xoff=         - horizontal offset of ROI (in pixels).
    yoff=         - vertical offset of ROI (in pixels).

FIXME: temporarily removed.
    fanspeed=     - fan speed parameter ("On" or "Off").

SEE ALSO: _tao_ncpa_cam_start, _tao_ncpa_cam_stop, _tao_ncpa_set_cam_config
*/
{
  h = _tao_ncpa_get_private();         // Get the private space.

// FIXME: find a way to control the fan (using keywords ?)
//
//   serial = tao_configure(cam, h.timeout, exposuretime=exposuretime,
//                          fanspeed=fanspeed, framerate=framerate,
//                          xoff=xoff, yoff=yoff, width=width, height=height);

  serial = tao_configure(cam, h.timeout, exposuretime=exposuretime,
                         framerate=framerate, pixeltype=pixeltype,
                         xoff=xoff, yoff=yoff, width=width, height=height);
  if (! tao_wait_command(cam, serial, h.timeout)) {
    error, "timeout while waiting for the camera to configure";
  }
}


func _tao_ncpa_cam_has_changed(void)
/* DOCUMENT  bool = _tao_ncpa_cam_has_changed()

    returns true if any of the critical parameters of the camera configuration
    has changed from the state saved in _tao_ncpa_private.cam_config, and false
    otherwise. These parameters are:

        cam_config
        |- exposuretime
        |- framerate
        |- height
        |- state
        |- width
        |- xoff
        `- yoff

SEE ALSO: tao_ncpa, _tao_ncpa_get_cam_config
*/
{
  // Get reference configuration of the camera in the private space.
  h_ref = _tao_ncpa_get_private().cam_config;

  // Get the current configuration of the camera.
  h = _tao_ncpa_get_cam_config();

  // Check if each entry has changed or not.
  keys = h_keys(h);
  n = numberof(keys);
  for (i=1; i<=n; i++) {
    k = keys(i);
    if (h_get(h,k) != h_get(h_ref,k)) return 1n;
  }
  return 0n;
}


func _tao_ncpa_cam_start(cam)
/* DOCUMENT  _tao_ncpa_cam_start, cam

    starts the camera cam (a remote camera object) and waits until the camera
    is actually started. Issue an error if the timeout is reached while waiting
    for the camera to start.

SEE ALSO: _tao_ncpa_cam_stop, _tao_ncpa_cam_configure
*/
{
  h = _tao_ncpa_get_private();         // Get the private space.
  serial = tao_start(cam, h.timeout);
  if (! tao_wait_command(cam, serial, h.timeout)) {
    error, "timeout while waiting for the camera to start";
  }
}


func _tao_ncpa_cam_stop(obj)
/* DOCUMENT  _tao_ncpa_cam_stop, cam

    stops the camera cam (a remote camera object) and waits until the camera
    is actually stopped. Issue an error if the timeout is reached while waiting
    for the camera to start.

SEE ALSO: _tao_ncpa_cam_start, _tao_ncpa_cam_configure
*/
{
  h = _tao_ncpa_get_private();         // Get the private space.
  serial = tao_stop(cam, h.timeout);
  if (! tao_wait_command(cam, serial, h.timeout)) {
    error, "timeout while waiting for the camera to stop";
  }
}


func _tao_ncpa_check_fan(cam)
/* DOCUMENT  _tao_ncpa_check_fan, cam

    If the camera `cam` (a remote camera object) has a fan, the function
    confirms that the fan is Off (info message), or ask the user if stopping
    the fan is needed. If the answer is "yes", the function tries to stop the
    fan and issues an error if the directive is unsuccessful.

SEE ALSO: tao_ncpa
*/
{
  fan = cam.fanspeed;
  if (is_string(fan) && fan != "" ) {    // Camera has a fan
    if (fan == "Off") {                  // Camera fan is already Off.
      _tao_ncpa_info, "Camera fan is Off";
    } else {
      // ---- Ask if the fan must be stopped and try to do it if "yes".
      if (_tao_ncpa_yesno("Camera fan is On, do you want to stop it?")) {
        // ---- Request to stop the fan.
        need_start = 0n;
        if (cam.state == TAO_STATE_WORKING) {
          _tao_ncpa_cam_stop, cam;
          need_start = 1n;
        }
        _tao_ncpa_cam_configure, cam, fanspeed="Off";
        if (need_start) {
          _tao_ncpa_cam_start, cam;
        }
        if (cam.fanspeed != "Off") {
          error, "Cannot stop the fan"
        }
      }
    }
  }
}


func _tao_ncpa_check_influence(void)
/* DOCUMENT  bool = _tao_ncpa_check_influence()

    returns true if a readable file for the DM influence matrix is specified
    in the configuration and the keyword "modal", "windm", or "filter_tilt" is
    defined. If none of those keywords is set, returns false since the
    influence matrix is not needed. The function issues error messages if no
    file is specified and one of those keywords is set.

EXAMPLES:

        if (_tao_ncpa_check_influence) {
            _tao_ncpa_load_matrices_and_modes;
        }

SEE ALSO: tao_ncpa
*/
{
  h = _tao_ncpa_get_private();      // Get the private space.

  need_influence = h.modal || h.windm || h.filter_tilt;

  if (need_influence) {
    file = h.dm_influence_file;     // File name of the influence matrix

    // ---- Check file is readable if given
    if (is_string(file) && strlen(file) > 0) {
      if (!open(file,"r",1)) {
        error, swrite(format="cannot open file %s for influence matrix", file);
      }
      if (!fits_check_file(file, 1n)) {
        error, swrite(format="file %s not a fits file", file);
      }
      return 1n;      // Influence matrix is available and must be loaded.

    // ---- Issue error if need_influence but no file is given
    } else {
      msg = "keyword \"%s\" is not allowed since\n"+
            "       no file is specified for the DM influence matrix";
      if (h.modal) {
        error, swrite(format=msg, "modal");
      }
      if (h.windm) {
        error, swrite(format=msg, "windm");
      }
      if (h.filter_tilt) {
        error, swrite(format=msg, "filter_tilt");
      }
    }

  } else {
    return 0n;        // Influence matrix is not needed.
  }
}


func _tao_ncpa_check_pixeltype(cam)
/* DOCUMENT  _tao_ncpa_check_pixeltype, cam

    If the camera `cam` (a remote camera object) is configurated to send
    unsigned integers, the function asks the user to configure to camera for
    sending double values instead. Do it if the answer is "yes".

SEE ALSO: tao_ncpa
*/
{
  typ = cam.pixeltype;

  if (typ == TAO_UINT8  || typ == TAO_UINT16 ||
      typ == TAO_UINT32 || typ == TAO_UINT64) {

    msg = "The camera is configurated to send unsigned integer.\n";
    msg += "This may yield wrapping of the pixel values; double is safer.\n";
    msg += "Do you want to configure the camera for sending double?";

    if (_tao_ncpa_yesno(msg)) {

      // ---- Stop the camera if it is running.
      need_start = 0n;
      if (cam.state == TAO_STATE_WORKING) {
        _tao_ncpa_cam_stop, cam;
        need_start = 1n;
      }
      // ---- Configure to send double.
      _tao_ncpa_cam_configure, cam, pixeltype=double;

      // ---- Restart if it was stopped.
      if (need_start) {
        _tao_ncpa_cam_start, cam;
      }

      // ---- Check and report.
      if (tao_type(cam.pixeltype) == double) {
        _tao_ncpa_info, "Camera set to send double values";
      } else {
        error, "Cannot configure the pixel type of the camera server"
      }
    }
  }
}



func _tao_ncpa_cmd_to_param(cmd)
/* DOCUMENT  param = _tao_ncpa_cmd_to_param(cmd)

    returns the vector of parameters PARAM computed from the command vector
    CMD, depending on the selected options (see tao_ncpa_set). The reverse is
    computed with _tao_ncpa_param_to_cmd.

    If "modal" keyword is true in the tao_ncpa configuration, the parameters
    are the eigen modes of the DM vectorial space orthogonal to piston, tip and
    tilt. The function converts CMD to the reduced modal space with 3 degrees
    of freedom less than the number of actuators. If "greedy" keyword is true
    in the tao_ncpa configuration, the returned number of parameters is
    truncated at the number saved in nb_active_modes in the configuration.

    If "filter_tilt" keyword is true in the tao_ncpa configuration, PARAM
    vector is obtained by removing piston, tip and tilt from CMD. The initial
    command is filtered because the optimizer will not be able to add or remove
    the filtered modes since the gradients of the cost function versus filtered
    modes are null.

    Otherwise, the function just remove the average of CMD so that to keep the
    command centered. The initial command is piston removed with ctom because
    the optimizer will not be able to add or remove a piston since the gradient
    of the cost function versus piston mode is null.

SEE ALSO: tao_ncpa, _tao_ncpa_param_to_cmd, _tao_ncpa_compute_matrices_and_modes
*/
{
  h = _tao_ncpa_get_private();         // Get the private space.

  if (h.modal) {
    if (h.greedy) {
      np = dimsof(h.modes_to_cmd)(0);    // Number of params.
      na = h.nb_active_modes;            // Number of active modes.
      if (!is_void(na) && na > 0 && na <= np) {
        return (h.cmd_to_modes(,+) * cmd(+))(1:na);
      } else {
        error, "unexpected number of active modes";
      }
    } else {
      return h.cmd_to_modes(,+) * cmd(+);
    }

  } else if (h.filter_tilt) {
    return h.ptt_filter(,+) * cmd(+);

  } else {
    return cmd - avg(cmd);
  }
}


func _tao_ncpa_compute_matrices_and_modes(void)
/* DOCUMENT  h = _tao_ncpa_compute_matrices_and_modes();

    computes the following matrices and vectors and returns them gathered in
    a hash table with this layout.

      H
      |- c_piston  normalized command for sending a piston mode on the DM.
      |- c_tip     normalized command for sending a tip mode on the DM.
      |- c_tilt    normalized command for sending a tilt mode on the DM.
      |- c_focus   normalized command for sensing a focus mode on the DM
      |- ctom      command-to-modes matrix.
      |- mtoc      modes-to-command matrix.
      |- P         projector in the space out of piston, tip, and tilt.
      |- s         singular values of M.P (matrix product).
      |- U         modes in the space of the DM surface.
      |- Vt        modes in the command space.
      `- ipix      index of the pixels in the pupil.

    The matrix CTOM converts a command vector to its modal decomposition where
    piston, tip and tilt are removed (thus 3 degrees of freedom less than
    command vector), while the matrix MTOC converts from the filtered modal
    decomposition to the corresponding command vector.

SEE ALSO: tao_ncpa, _tao_ncpa_load_influence_maps
*/
{
  // ---- Get M and M^-1.
  M = tao_ncpa_get("influence_maps");
  dims = dimsof(M);
  if (dims(1) != 3) {
    error, "expecting 3 dimensions for influence maps";
  }
  nx = dims(2);                         // nx x ny: size of the maps.
  ny = dims(3);
  nc = dims(4);                         // number of actuators.

  ipix = where(M(..,1) == M(..,1));     // same mask for all the influence fct.
  npix = numberof(ipix);                // number of pixels in the pupil.
  M = M(*,)(ipix,);                     // select pixels in pupil and flatten M.
  M_1 = pseudo_inverse(M);              // in package linalg.i

  // ---- Coordinates in the pupil.
  x = double(indgen(nx));
  x -= avg(x);
  y = double(indgen(ny));
  y -= avg(y);

  // ---- Compute the modes to be removed.
  piston = array(1., npix);
  piston *= 1./sqrt(sum(piston^2));     // normalize.
  tip = x(,-:1:ny)(ipix);
  tip -= avg(tip);                      // center.
  tip *= 1./sqrt(sum(tip^2));           // normalize.
  tilt = y(-:1:nx,)(ipix);
  tilt -= avg(tilt);                    // center.
  tilt *= 1./sqrt(sum(tilt^2));         // normalize.

  // --- Compute corresponding modes in command vectorial space.
  c_piston = M_1(,+)*piston(+);
  c_piston *= 1./sqrt(sum(c_piston^2)); // normalize.
  c_tip = M_1(,+)*tip(+);
  c_tip -= avg(c_tip);                  // center.
  c_tip *= 1./sqrt(sum(c_tip^2));       // normalize.
  c_tilt = M_1(,+)*tilt(+);
  c_tilt -= avg(c_tilt);                // center.
  c_tilt *= 1./sqrt(sum(c_tilt^2));     // normalize.

  // ---- Compute the projector.
  G = [c_piston, c_tip, c_tilt];
  GtG_1 = LUsolve(G(+,)*G(+,));         // inverse of Gt.G.
  P = array(double, nc, nc);
  P(1:nc*nc:nc+1) = 1.                  // identity matrix.
  P -= (G(,+)*GtG_1(+,))(,+)*G(,+);     // P = I - G.(Gt.G)^-1.Gt.

  // ---- Compute focus mode.
  focus = (x^2 + y(-,)^2)(ipix);
  c_focus = P(,+) * (M_1(,+)*focus(+))(+);// remove piston/tip/tilt
  c_focus *= 1./sqrt(sum(c_focus^2));   // normalize.

  // ---- SVD of M.P
  // w = M.P.c = U.S.Vt.c = U.m, with m = S.Vt.c,
  // where w is DM surface shape, c is command, m is modes.
  // ctom = S.Vt; mtoc = V.S^-1.
  s = SVdec(M(,+)*P(+,), U, Vt);
  s = s(1:nc-3);                        // remove the 3 filtered modes.
  Vt = Vt(1:nc-3,);
  ctom = s * Vt;                        // = diag(s)(,+) . Vt(+,)
  mtoc = transpose((1./s)*Vt);          // = V(,+) . diag(1/s)(+,)

  return h_new(c_piston=c_piston, c_tip=c_tip, c_tilt=c_tilt, c_focus=c_focus,
               ctom=ctom, mtoc=mtoc, P=P, s=s, U=U, Vt=Vt, ipix=ipix);
}


func _tao_ncpa_cost(x)
/* DOCUMENT  cost = _tao_ncpa_cost(x)

    computes the value of the cost function to be maximized by NEWUOA
    algorithm. X are the parameters, i.e. the command vector or the modes if
    keyword MODAL is set to true.

    This function is in charge to collect quantities in its workspace
    _tao_ncpa_private.cost_ws. Namely: firstcommand, firstcost, firstimage,
    bestcommand, bestcost, bestimage, allcosts.

SEE ALSO: tao_ncpa
*/
{
  h = _tao_ncpa_get_private();    // Get the private space.
  ws = h.cost_ws;                 // Get my workspace.
  neval = ws.neval + 1;           // Increment neval.
  h_set, ws, neval = neval;       // Save it.

  x = _tao_ncpa_param_to_cmd(x);  // Convert parameters to command.
  full_cmd = h.ground_cmd + x;    // full command.

  if (anyof(abs(full_cmd) > h.maxcmd)) {
    error,"Emergency stop because command out of bounds";
  }

  // Send command to the DM.
  t1 = tao_get_monotonic_time();
  _tao_ncpa_send_cmd, full_cmd;
  t1 = tao_get_monotonic_time() - t1;

  // Wait
  pause, lround(h.delay*1e3);    // delay in ms.

  // Get the image.
  t2 = tao_get_monotonic_time();
  image = tao_wait_image(h.cam, timeout=h.timeout, noweights=1);
  t2 = tao_get_monotonic_time() - t2;

  // Compute the quality of the image.
  t3 = tao_get_monotonic_time();
  cost = h.cost_fct(image);
  t3 = tao_get_monotonic_time() - t3;

  // ---- Collect requested informations
  // Save the first values.
  if (neval == 1) {
    h_set, ws, firstcommand = full_cmd, firstcost = cost, firstimage = image;
    // maxeval must be obtained with tao_ncpa_get to ensure its value is
    // properly defined, with a default value if necessary.
    maxeval = tao_ncpa_get("maxeval");
    h_set, ws, allcosts = array(double, maxeval);
  }
  // Collect all the cost values in ws.allcosts
  allcosts = [];
  eq_nocopy, allcosts, ws.allcosts;
  allcosts(neval) = cost;

  maximage = max(image);
  // ---- If best performance is achieved.
  if (is_void(ws.bestcost) || cost > ws.bestcost) {
    // Save best performance and corresponding quantities.
    h_set, ws, bestcommand = full_cmd, bestcost = cost, bestimage = image;
    // Reporting.
    if (!ws.title_shown) {
      write, format="%s\n",
             "-eval-  --cost--  -max-  (  DM  |  camera  | comp.)";
      h_set, ws, title_shown = 1n;
    }
    write, format= "%6d   %#7.4g  %5d  (%3.0fµs |%7.0fµs |%4.0fµs)\n",
           neval, cost, long(maximage), t1*1e6, t2*1e6, t3*1e6;
    _tao_ncpa_plot_img, image;
  }
  // ---- Advise saturation if it happens.
  if (!ws.saturation_found && maximage > tao_ncpa_get("saturation")) {
    write, format="%6d ------->>  %5d  <<----------------------- SATURATION\n",
                  neval, long(maximage);
    h_set, ws, saturation_found = 1n;
  }
  _tao_ncpa_plot_dm, full_cmd;
  return cost;
}


func _tao_ncpa_get_cam(void)
/* DOCUMENT   cam = _tao_ncpa_get_cam()

    returns the current remote camera. If not yet available, the function
    ensures that the camera server is running, attaches to it, initializes the
    remote camera object and saves it in _tao_ncpa_private. If a remote camera
    is already attached, the function checks the shmid has not changed, and try
    a new connection if it is not the same (it may happen if the server was
    relaunched).

SEE ALSO: tao_ncpa
*/
{
  h = _tao_ncpa_get_private();         // Get the private space.

  // ---- If not yet connected to the camera server
  if (is_void(h.cam)) {

    // ---- Catch if we cannot attach to the camera server
    //
    if (catch(0x08)) {
      // 0x08 catches only errors of type "other compiled errors (YError)".

      write, format="WARNING: cannot connect to camera server \"%s\".\n",
             h.cam_name;
      write, format="Please launch the camera server \"%s\"...\n",
             h.cam_name;
      cmd = "andor-server 0 \""+ h.cam_name + "\"";
      write, format="    $ %s\n", cmd;
    }

//       if (!h.cam_attach_already_attempted) {
//
//         h_set, h, cam_launch_already_attempted = 1n;
//         write, format="WARNING: cannot connect to camera server \"%s\".\n",
//                h.cam_name;
//         write, format="Attempting to launch camera server \"%s\"...\n",
//                h.cam_name;
//         cmd = "andor-server \""+ h.cam_name + "\"";
//         write, format="    $ %s\n", cmd;
//         system, cmd;
//
//       } else {
//
//         h_set, h, cam_launch_already_attempted = 0n;
//         write, format="ERROR: cannot connect to camera server %s\n", h.cam_name;
//         error, catch_message;
//
//       }
//     }

    // ---- Attach to the remote camera
    // - Error from tao_attach_remote_camera will be catched to attempt
    //   launching or relaunching the camera server.
    // - shmid is stored to check later that the camera server is unchanged.
    cam = tao_attach_remote_camera(h.cam_name);
    h_set, h, cam = cam,
              cam_shmid = cam.shmid;
    return cam;

  } else {

    // ---- Check that the shmid is the same.
    if (h.cam.shmid != h.cam_shmid) {

      h_delete, h, "cam", "cam_shmid", "cam_config";
      _tao_ncpa_info, "reconnecting to " + h.cam_name + ".";
      return _tao_ncpa_get_cam();

    } else {

      return h.cam;

    }
  }
}


func _tao_ncpa_get_cam_config(void)
/* DOCUMENT  ht = _tao_ncpa_get_cam_config()

    returns the following hash table with each entry initialized with the
    current value from the camera.

        HT
        |- exposuretime
        |- framerate
        |- height
        |- state
        |- width
        |- xoff
        `- yoff

SEE ALSO: tao_ncpa
*/
{
  cam = _tao_ncpa_get_cam();     // Issue an error if cannot get cam object.

  if (!cam.lock) tao_lock, cam;
  cam_config = h_new(
      exposuretime = cam.exposuretime,
      framerate = cam.framerate,
      height = cam.height,
      state = cam.state,
      width = cam.width,
      xoff = cam.xoff,
      yoff = cam.yoff);
  tao_unlock, cam;
  return cam_config;
}


func _tao_ncpa_get_cmd(void)
/* DOCUMENT  _tao_ncpa_get_cmd()

    gets the current effective command from the deformable mirror. If a DM
    object is not yet available, the function ensures the DM server is running,
    attaches to it, initializes the DM object, saves it in _tao_ncpa_private,
    and finally gets the command.

SEE ALSO: _tao_ncpa_get_dm
*/
{
  h = _tao_ncpa_get_private();      // Get the private space.

  // ---- Get a reference on the DM
  dm = _tao_ncpa_get_dm();          // Issue an error if cannot get dm object.

  frame = tao_wait_frame(dm, dm.serial, h.dm_timeout);
  serial = frame.serial;
  if (serial <= 0) {
    if (serial == 0) {
      error, "DM timeout";
    } else if (serial == -1) {
      error, "DM command has been overwritten";
    } else {
      error, "server is no longer running";
    }
  }
  return frame.effcmds;
}


func _tao_ncpa_get_dm(void)
/* DOCUMENT   dm = _tao_ncpa_get_dm()

    returns the current DM object. If not yet available, the function ensures
    the DM server is running, attaches to it, initializes the DM object and
    saves it in _tao_ncpa_private. The stored DM object is immediately returned
    if it is already available.

SEE ALSO: tao_ncpa
*/
{
  h = _tao_ncpa_get_private();      // Get the private space.

  // ---- If not yet connected to the DM server
  if (is_void(h.dm)) {

    // ---- Catch if we cannot attach to the DM server
    //
    if (catch(0x08)) {
      // 0x08 catches only errors of type "other compiled errors (YError)".

      write, format="WARNING: cannot connect to DM server \"%s\".\n",
             h.dm_name;
      write, format="Please launch the DM server \"%s\" with this command:\n",
             h.dm_name;
      cmd = swrite(format="alpao_server \"%s\" \"%s\"",
                   h.dm_configuration, h.dm_name);
      write, format="    $ %s\n", cmd;
    }
// FIXME: remove this?
//       if (!h.dm_launch_already_attempted) {
//
//         h_set, h, dm_launch_already_attempted = 1n;
//         write, format="WARNING: cannot connect to DM server \"%s\".\n",
//                h.dm_name;
//         write, format="Attempting to launch the DM server \"%s\"...\n",
//                h.dm_name;
//         cmd = swrite(format="alpao-server \"%s\" \"%s\"",
//                      h.dm_configuration, h.dm_name);
//         write, format="    $ %s\n", cmd;
//         system, cmd;
//
//       } else {
//
//         h_set, h, dm_launch_already_attempted = 0n;
//         write, format="ERROR: cannot connect to DM server %s\n", h.dm_name;
//         error, catch_message;
//
//       }
//     }

    // ---- Connect to the DM server
    //
    dm = tao_attach_remote_mirror(h.dm_name);
    tao_reset, dm;       // Ensure the DM shape corresponds to command zero.
    h_set, h, dm = dm;   // Save in private space.
  }
  return h.dm;
}


func _tao_ncpa_get_private(void)
/* DOCUMENT  h = _tao_ncpa_get_private(void)

    returns _tao_ncpa_private private hash table that contains all the
    parameters for the tao_ncpa_* functions, ensuring that the hash table is
    properly defined.

SEE ALSO: tao_ncpa
*/
{
  extern _tao_ncpa_private;

  // Reset _tao_ncpa_private only if it was lost.
  if (!is_hash(_tao_ncpa_private)) include, _TAO_NCPA_SOURCE, 1;
  return _tao_ncpa_private;
}


func _tao_ncpa_load_influence_maps(void)
/* DOCUMENT  influence_maps = _tao_ncpa_load_influence_maps()

    loads the influence matrix in _tao_ncpa_private and returns it. The
    returned array is NX-by-NY-by-NA where NA is the number of actuators and
    NX-by-NY is the size of the map of each influence function, defined on a
    circular pupil. The values outside the pupil are set to NaN.

    The position of the actuators are oriented as they can be seen when we look
    directly at the surface of the mirror set on its support. The influence
    functions are multiplied by two they were obtained with command set to 0.5.
    The sign is chosen so that the influence function is positive where the
    command is positive.

SEE ALSO: tao_ncpa, tao_ncpa_dmshape
*/
{
  h = _tao_ncpa_get_private();                       // Get the private space.

  if (is_void(h.influence_maps)) {
// FIXME: Update for THEMIS
//     infmat = -2.*fits_read(h.dm_influence_file)(::-1,..); // Flip first dim.
    infmat = fits_read(h.dm_influence_file);
    h_set, h, influence_maps = infmat;
    return infmat;
  } else {
    return h.influence_maps;
  }
}


func _tao_ncpa_load_matrices_and_modes(void)
/* DOCUMENT  _tao_ncpa_load_matrices_and_modes

    loads the matrices cmd_to_modes and modes_to_cmd, and the commands c_tip
    and c_tilt (see _tao_ncpa_compute_matrices_and_modes), in
    _tao_ncpa_private.

SEE ALSO: tao_ncpa, _tao_ncpa_compute_matrices_and_modes
*/
{
  h = _tao_ncpa_get_private();                    // Get the private space.
  if (is_void(h.algebra)) {
    alg = _tao_ncpa_compute_matrices_and_modes();
    h_set, h, algebra = alg,
              cmd_to_modes = alg.ctom,
              modes_to_cmd = alg.mtoc,
              ptt_filter = alg.mtoc(,+) * alg.ctom(+,),
              c_focus = alg.c_focus, c_piston = alg.c_piston,
              c_tip = alg.c_tip, c_tilt = alg.c_tilt;
  }
}


func _tao_ncpa_param_to_cmd(param)
/* DOCUMENT  cmd = _tao_ncpa_param_to_cmd(param)

    returns the command vector CMD computed from the vector of parameters
    PARAM, depending on the selected options (see tao_ncpa_set). The reverse is
    computed with _tao_ncpa_cmd_to_param.

    If "modal" keyword is true in the tao_ncpa configuration, the parameters
    are the eigen modes of the DM vectorial space orthogonal to piston, tip and
    tilt. The function converts PARAM from the reduced modal space with 3
    degrees of freedom less than the number of actuators back to a command
    vector. If "greedy" keyword is true in the tao_ncpa configuration, a
    reduced number of parameters is used, fixed at the number saved in
    nb_active_modes in the configuration.

    If "filter_tilt" keyword is true in the tao_ncpa configuration, command
    vector CMD and PARAM vector are in the same vectorial space. CMD is
    obtained by removing piston, tip and tilt from PARAM.

    Otherwise, the function just remove the average of PARAM so that to keep
    the command centered.

SEE ALSO: tao_ncpa, _tao_ncpa_param_to_cmd, _tao_ncpa_compute_matrices_and_modes
*/
{
  h = _tao_ncpa_get_private();         // Get the private space.

  if (h.modal) {
    if (h.greedy) {
      np = numberof(param);            // Number of parameters (i.e. modes)
      return h.modes_to_cmd(,1:np)(,+) * param(+);
    } else {
      return h.modes_to_cmd(,+) * param(+);
    }

  } else if (h.filter_tilt) {
    return h.ptt_filter(,+) * param(+);

  } else {
    return param - avg(param);
  }
}


func _tao_ncpa_send_cmd(cmd)
/* DOCUMENT  _tao_ncpa_send_cmd, cmd

    sends the command CMD to the deformable mirror and waits until the command
    appears in the telemetry so that to be sure the command has been sent to
    the DM. If a DM object is not yet available, the function ensures the DM
    server is running, attaches to it, initializes the DM object, saves it in
    _tao_ncpa_private, and finally sends the command.

SEE ALSO: _tao_ncpa_get_dm
*/
{
  h = _tao_ncpa_get_private();      // Get the private space.

  // ---- Get a reference on the DM
  dm = _tao_ncpa_get_dm();          // Issue an error if cannot get dm object.

  if (is_scalar(cmd) && cmd == 0) {

    // ---- Reset the DM.
    tao_reset, dm;                  // Particular case that resets the DM.

  } else if (is_numerical(cmd) && numberof(cmd) == dm.nacts) {

    // ---- Send the command.
    //
    // Wait until the command appears in telemetry to ensure that it has been
    // sent to the DM.
    serial = tao_send_commands(dm, cmd, h.dm_timeout);
    num = tao_wait_output(dm, serial, h.dm_timeout);
    if (num <= 0) {
      if (num == 0) {
        error, "DM timeout";
      } else if (num == -1) {
        error, "DM command has been overwritten";
      } else {
        error, "server is no longer running";
      }
    }

  } else {
    error, swrite(format="command must be 0 or a %d-element vector", dm.nacts);
  }
}


func _tao_ncpa_set_cam_config(exposuretime=, framerate=, height=, width=,
                              roi=, xoff=, yoff=)
/* DOCUMENT  _tao_ncpa_set_cam_config, <key> = <val>, ...

    reconfigures some parameters of the camera. The allowed parameters are:
    EXPOSURETIME, FRAMERATE, ROI, XOFF, YOFF, HEIGHT, and WIDTH. The new
    parameters are sent to the camera only if they are different to the current
    ones. If the camera is acquiring, it is first stopped and immediately
    restarted after the configuration parameters are sent.

SEE ALSO: tao_ncpa, _tao_ncpa_get_cam_config
*/
{
  given_kwd = h_new();

  // ---- Check the parameters
  if (!is_void(exposuretime)) {
    if (is_numerical(exposuretime) && is_scalar(exposuretime)
        && exposuretime > 0) {
      h_set, given_kwd, exposuretime = exposuretime;
    } else {
      error, "exposuretime must be a positive number";
    }
  }
  if (!is_void(framerate)) {
    if (is_numerical(framerate) && is_scalar(framerate) && framerate > 0) {
      h_set, given_kwd, framerate = framerate;
    } else {
      error, "framerate must be a positive number";
    }
  }
  if (!is_void(height)) {
    if (is_integer(height) && is_scalar(height) && height > 0) {
      h_set, given_kwd, height = height;
    } else {
      error, "height must be a positive integer";
    }
  }
  if (!is_void(width)) {
    if (is_integer(width) && is_scalar(width) && width > 0) {
      h_set, given_kwd, width = width;
    } else {
      error, "width must be a positive integer";
    }
  }
  if (!is_void(xoff)) {
    if (is_integer(xoff) && is_scalar(xoff) && xoff >= 0) {
      h_set, given_kwd, xoff = xoff;
    } else {
      error, "xoff must be a positive integer";
    }
  }
  if (!is_void(yoff)) {
    if (is_integer(yoff) && is_scalar(yoff) && yoff >= 0) {
      h_set, given_kwd, yoff = yoff;
    } else {
      error, "yoff must be a positive integer";
    }
  }
  if (!is_void(roi)) {
    if (is_integer(roi) && numberof(roi) == 4 && allof(roi >= 0)) {
      if (is_void(given_kwd.xoff))   h_set, given_kwd, xoff = roi(1);
      if (is_void(given_kwd.yoff))   h_set, given_kwd, yoff = roi(2);
      if (is_void(given_kwd.width))  h_set, given_kwd, width = roi(3);
      if (is_void(given_kwd.height)) h_set, given_kwd, height = roi(4);
    } else {
      error, "roi must be an array of 4 positive integers";
    }
  }
  // ---- Return if no keyword was given.
  keys = h_keys(given_kwd);
  n = numberof(keys);
  if (n == 0) return;

  // ---- Only keep keywords with values different from current config.
  current_config = _tao_ncpa_get_cam_config(); // Ensure cam is connected.
  for (i=1; i<=n; i++) {
    k = keys(i);
    if (h_get(given_kwd,k) == h_get(current_config,k)) {
      h_delete, given_kwd, k;
    }
  }

  // ---- Return if no keyword left in given_kwd : nothing to send.
  if (numberof(h_keys(given_kwd)) == 0) return;

  // ---- Send the new configuration to the camera.
  cam = _tao_ncpa_get_cam();

  need_start = 0n;
  if (current_config.state == TAO_STATE_WORKING) {
    _tao_ncpa_cam_stop, cam;
    need_start = 1n;
  }
  _tao_ncpa_cam_configure, cam,
        exposuretime = given_kwd.exposuretime,
        framerate = given_kwd.framerate,
        xoff = given_kwd.xoff,
        yoff = given_kwd.yoff,
        width = given_kwd.width,
        height = given_kwd.height;

  if (need_start) {
    _tao_ncpa_cam_start, cam;
  }
}


func _tao_ncpa_wfs_cost(cmd)
/* DOCUMENT  cost = _tao_ncpa_wfs_cost(cmd)

    computes and returns the variance of the absolute slopes, i.e. considering
    the reference positions at the exact center of the subapertures.

SEE ALSO: tao_ncpa_center_wfs.
*/
{
  h = _tao_ncpa_get_private();    // Get the private space.
  ws = h.wfs_ws;

  // ---- Send command to the DM.
  _tao_ncpa_send_cmd, cmd;

  // ---- Wait for the delay.
  pause, lround(h.delay*1e3);    // delay in ms.

  // ---- Get the image.
  frame = tao_wait_image(h.wfs, timeout=h.timeout, noweights=1);

  // ---- Compute the rms of the slopes.
  xy = tao_wfs_cog_on_spots(frame, h.wfs_layout, ws.npix);
  h_set, ws, data = xy;
  x = xy(1,);
  y = xy(2,);
  return sum(x*x) + sum(y*y);
}


/*--+=^=+----------------------------------------------------------------------
 *                     INPUT/OUTPUT PRIVATE FUNCTIONS
 *---------------------------------------------------------------------------*/

func _tao_ncpa_cbar(z, adjust=, cmax=, cmin=, color=, extrema=, font=, format=,
                    height=, nlabs=, opaque=, orient=, thickness=, ticklen=,
                    vert=, vport=, width=)
/* DOCUMENT  _tao_ncpa_cbar, z;
     - or -  ncpa, cmin=CMIN, cmax=CMAX;

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
         > z = random_n(100,100);
         > pli, z, cmin=0;
         > _tao_ncpa_cbar, z, cmin=0, extrema=1;

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
    plg, [y0,y0,y1,y1], [x0,x1,x1,x0], closed=1,
      color=color, width=width, type=linetype, marks=0;
  }

  if (nlabs) {
    ticks = _tao_ncpa_ticks(cmin, cmax, nlabs, extrema=extrema);
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


func _tao_ncpa_debug(text)
/* DOCUMENT  _tao_ncpa_debug, text

    displays the debug message given by TEXT with label "DEBUG: ", only when
    debug mode is set to true.
*/
{
  h = _tao_ncpa_get_private();         // Get the private space.
  if (h.debug) write, format="DEBUG: %s\n", text;
}


func _tao_ncpa_info(text)
/* DOCUMENT  _tao_ncpa_info, text

    displays the informative message given by TEXT with label "INFO: ".
*/
{
  write, format="INFO: %s\n", text;
}


func _tao_ncpa_plot_dm(cmd, win=)
/* DOCUMENT  _tao_ncpa_plot_dm, cmd

    plots the shape of the deformable mirror in the window whose number is
    given by tao_ncpa_get("windm"), unless "windm" is not defined (see
    tao_ncpa_set). If given, heyword WIN specifies the number of the window
    where to plot anyway.

SEE ALSO: tao_ncpa, tao_ncpa_set, tao_ncpa_dmshape
*/
{
  h = _tao_ncpa_get_private();         // Get the private space.
  if (is_void(win)) {
    if (!h.windm) return;
    window, h.windm;
  } else {
    window, win;
  }
  map = tao_ncpa_dmshape(cmd);
  fma;
  pli, map;
  _tao_ncpa_cbar, map, extrema=1;
}


func _tao_ncpa_plot_img(image, win=)
/* DOCUMENT  _tao_ncpa_plot_img, image

    plots the image IMAGE given as the first argument in the window whose
    number is given by tao_ncpa_get("winimg"), unless "winimg" is not defined
    (see tao_ncpa_set). If given, heyword WIN specifies the number of the
    window where to plot anyway.

SEE ALSO: tao_ncpa, tao_ncpa_set
*/
{
  h = _tao_ncpa_get_private();         // Get the private space.
  if (is_void(win)) {
    if (!h.winimg) return;
    window, h.winimg;
  } else {
    window, win;
  }
  x0 = y0 = 0.5;
  dims = dimsof(image);
  width = dims(2);
  height = dims(3);
  cam = _tao_ncpa_get_cam();
  if (!is_void(cam)) {    // FIXME: xoff not already known ?
    x0 += cam.xoff;
    y0 += cam.yoff;
  }
  fma;
  pli, image, x0, y0, x0+width, y0+height;
  _tao_ncpa_cbar, image, extrema=1;
}


func _tao_ncpa_ticks(cmin, cmax, n, extrema=, outside=)
/* DOCUMENT  ticks = _tao_ncpa_ticks(cmin, cmax, n)

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


func _tao_ncpa_warn(text)
/* DOCUMENT  _tao_ncpa_warn, text

    displays a warning message given by TEXT with label "WARNING:"
*/
{
  write, format="WARNING: %s\n", text;
}


func _tao_ncpa_yesno(question)
/* DOCUMENT  bool = _tao_ncpa_yesno(text)

    returns true or false according to the answer at the prompt after the text
    QUESTION is displayed.

EXAMPLES:
        if (_tao_ncpa_yesno("Do you want to stop ?")) {
          return;
        }

SEE ALSO:
*/
{
  answer = "";
  n = read(prompt=question+" (y/n) ", format="%s", answer);
  answer = strpart(answer,1:1);
  if (answer == "Y" || answer == "y") {
    return 1n;
  } else {
    return 0n;
  }
}
