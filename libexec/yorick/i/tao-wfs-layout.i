/*
 * tao-wfs-layout.i --
 *
 * Yorick software for Themis Adaptive Optics.
 *
 *-----------------------------------------------------------------------------
 *
 * This file if part of the TAO software (https://git-cral.univ-lyon1.fr/tao)
 * licensed under the MIT license.
 * Updates can be found in https://git-cral.univ-lyon1.fr/tao/tao-yorick.
 *
 * Copyright (C) 2020, Michel Tallon, Éric Thiébaut.
 */

/* TODO:
 * -
 */

/* SECTIONS
 * --------
 *
 * - DEFINITIONS
 *
 * - PUBLIC FUNCTIONS
 *       - tao_wfs_is_layout
 *       - tao_wfs_get_layout
 *       - tao_wfs_layout
 *       - tao_wfs_layout_boxes
 *
 * - PRIVATE FUNCTIONS
 *       - _tao_is_local_max
 *       - _tao_is_local_min
 *       - _tao_find_most_significant
 *       - _tao_wfs_layout_error
 *       - _tao_wfs_layout_findmaxima
 *       - _tao_wfs_layout_fit
 *       - _tao_wfs_layout_limits
 *       - _tao_wfs_layout_object
 *       - _tao_wfs_layout_subimage_start_index
 */

if (!is_func(h_new)) {
  // For yeti 6.4.0 and earlier.
  // For yeti 6.4.1 and above, h_new is an autoload object and "yeti.i" will
  // not be included here.
  include, "yeti.i", 1;
}

if (!is_func(yhd_check)) {
  // For yeti 6.4.0 and earlier.
  // - From 6.4.1, yhd_check is an autoload, so this include will not be done.
  // - From 6.5.0, "yeti_yhdf.i" is renamed as "yhdf.i".
  include, "yeti_yhdf.i", 1;
}


/*--+=^=+----------------------------------------------------------------------
 *                               DEFINITIONS
 *---------------------------------------------------------------------------*/

TAO_WFS_LAYOUT = "TaoWavefrontSensorLayout";  // class.

TAO_WFS_DEFAULT_MAP = int([[0,0,1,1,1,1,1,1,0,0],
                           [0,1,1,1,1,1,1,1,1,0],
                           [1,1,1,1,1,1,1,1,1,1],
                           [1,1,1,1,0,0,1,1,1,1],
                           [1,1,1,0,0,0,0,1,1,1],
                           [1,1,1,0,0,0,0,1,1,1],
                           [1,1,1,1,0,0,1,1,1,1],
                           [1,1,1,1,1,1,1,1,1,1],
                           [0,1,1,1,1,1,1,1,1,0],
                           [0,0,1,1,1,1,1,1,0,0]]);

TAO_WFS_FULL_MAP = int([[0,0,1,1,1,1,1,1,0,0],
                        [0,1,1,1,1,1,1,1,1,0],
                        [1,1,1,1,1,1,1,1,1,1],
                        [1,1,1,1,1,1,1,1,1,1],
                        [1,1,1,1,1,1,1,1,1,1],
                        [1,1,1,1,1,1,1,1,1,1],
                        [1,1,1,1,1,1,1,1,1,1],
                        [1,1,1,1,1,1,1,1,1,1],
                        [0,1,1,1,1,1,1,1,1,0],
                        [0,0,1,1,1,1,1,1,0,0]]);

/*--+=^=+----------------------------------------------------------------------
 *                             PUBLIC FUNCTIONS
 *---------------------------------------------------------------------------*/


func tao_wfs_is_layout(wfs)
/* DOCUMENT  bool = tao_wfs_is_layout(wfs)

    return true/false if WFS is a hash table of a class corresponding to
    wavefront sensor object.

SEE ALSO: tao_visu
*/
{
  return (is_hash(wfs) && wfs.__class == TAO_WFS_LAYOUT);
}


func tao_wfs_get_layout(wfs_layout)
/* DOCUMENT  layout = tao_wfs_get_layout(wfs_layout)

    returns a checked WFS layout (a TAO_WFS_LAYOUT object) from WFS_LAYOUT
    argument which can be a file name or a hash table.

SEE ALSO: tao_wfs_layout, tao_wfs_is_layout.
*/
{
  if (is_void(wfs_layout)) {
    error, "missing first argument";
  }
  if (is_string(wfs_layout)) {
    file = wfs_layout;
    if (!open(file,"r",1)) {
      error, "file " + basename(file) + " not found";
    } else if (yhd_check(file)) {
      wfs_layout = yhd_restore(file);
    } else {
      error, "no hash table found in " + basename(file);
    }
  }
  if (!tao_wfs_is_layout(wfs_layout)) {
    if (file) {
      error, "no WFS layout found in "+basename(file);
    } else {
      error, "given wfs_layout is not a WFS layout object";
    }
  }
  return wfs_layout;
}


local TAO_WFS_DEFAULT_MAP;
local TAO_WFS_LAYOUT;
func tao_wfs_layout(args)
/* DOCUMENT wfs = tao_wfs_layout(xdim, ydim, x0, y0, x1, y1, sp);
         or wfs = tao_wfs_layout(xdim, ydim, x0, y0, x1, y1, sp, map);
         or wfs = tao_wfs_layout(img);
         or wfs = tao_wfs_layout(img, map);

     yields an object of class TAO_WFS_LAYOUT (i.e. a hash table)
     describing the layout of a Shack-Hartmann wavefront sensor (WFS). The aim
     of the function is essentially to determine the bounding box of each
     active subimage.

     A first possibility is to specify XDIM and YDIM, the dimensions of the
     image provided by the WFS detector, (X0,Y0) and (X1,Y1), the coordinates
     (in fractional pixels and with same indexing rules as Yorick) of the
     lower-left and upper-right corners of the bounding-box of the WFS
     footprint on the detector, and SP, the separation (in fractional pixels)
     between the sub-images formed by the micro-lens array of the WFS.

     The other possibility is to provide an image IMG with the "mean"
     sub-images formed by the wavefront sensor. In this case, the WFS layout is
     computed with function tao_wfs_layout_boxes (see its help) unless keyword
     LEGACY is set to true.

     Optional argument MAP is an NCOLS-by-NROWS array which is true (non-zero)
     for valid WFS sub-pupils.  Here NCOLS and NROWS are the number of WFS
     sub-pupils along the horizontal and vertical dimensions of the detector.

     Keyword QUIET may be set true to suppress information messages.

     The returned hash table has the following "public" read-only members:

         wfs
         |- ncols    The number of columns of the sub-pupil array.
         |- nrows    The number of rows of the sub-pupil array.
         |- nsubs    The number of sub-images (also the number of non-zero
         |           values in WFS.map).
         |- map      A NCOLS-by-NROWS array of integers whose non-zero entries
         |           give the indices of the valid sub-pupils (numbered from 1
         |           to WFS.nsubs).
         |- mask     A XDIM-by-YDIM array of integers whose elements are 1
         |           where a pixel is in a sub-image, and 0 otherwise.
         |- x0
         |- y0       The coordinates (in fractional pixel units) of the
         |- x1       bounding box encompassing all the sub-images formed on
         |- y1       the WFS detector.
         |
         |- xdim     The wavefront sensor image dimensions (integer scalars
         |- ydim     of type long).  As given by arguments XDIM and YDIM.
         |
         |- xmin
         |- ymin     The bounding box of all valid sub-images. Each of these
         |- xmax     members are integer-valued vectors of NSUBS indices.
         |- ymax
         |
         |- subsize  The size of the subimages are SUBSIZE-by-SUBSIZE pixels,
         |           the same for all the subapertures.
         |
         |- mean     [*] A "mean" WFS image showing the subimages and the
         |               vignetting.
         `- proc     [*] An integer indicating the level of processing of the
                         "mean" WFS image (-1 if unknown, 0 for a mean of raw
                         images, 1 for the mean of calibrated images, etc.)

            [*] These fields are defined only if the layout has been computed
                from a given image IMG.

KEYWORDS
    legacy=   - If set to true, the layout is computed from given IMG by using
                function _tao_wfs_layout_fit (see its help).
    margin=   - Default method from a given IMG allows to specify an inner
                margin in pixels for the boxes. A positive inner margin yields
                smaller boxes while a negative inner margin yields large boxes.
                The default value is margin=1.
    no_error= - If set to true, no error will stop the function, and a void
                will be returned in case of error.
    quiet=  -   If set, prevents informative display of layout.

SEE ALSO: tao_wfs_data, tao_wfs_draw_layout, tao_fit_wfs_layout.
 */
{
  /* Parse arguments. */
  local xdim, ydim, x0, y0, x1, y1, sp, map, legacy, margin, no_error, quiet;
  kwds = args(-);
  for (i = numberof(kwds); i >= 1; --i) {
    kwd = kwds(i);
    if (kwd == "legacy") {
      eq_nocopy, legacy, args(kwd);
    } else if (kwd == "margin") {
      eq_nocopy, margin, args(kwd);
    } else if (kwd == "no_error") {
      eq_nocopy, no_error, args(kwd);
    } else if (kwd == "quiet") {
      eq_nocopy, quiet, args(kwd);
    } else {
      error, "unknown keyword "+kwd;
    }
  }
  argc = args(*);
  if (argc == 7 || argc == 8) {
    eq_nocopy, xdim, args(1);
    eq_nocopy, ydim, args(2);
    eq_nocopy, x0,   args(3);
    eq_nocopy, y0,   args(4);
    eq_nocopy, x1,   args(5);
    eq_nocopy, y1,   args(6);
    eq_nocopy, sp,   args(7);
    eq_nocopy, map, (argc >= 8 ? args(8) : TAO_WFS_DEFAULT_MAP);

    return _tao_wfs_layout_object(wfs, xdim, ydim, x0, y0, x1, y1, sp, map,
                                  quiet=quiet);

  } else if (argc == 1 || argc == 2) {
    eq_nocopy, img, args(1);
    eq_nocopy, map, (argc >= 2 ? args(2) : TAO_WFS_DEFAULT_MAP);

    if (legacy) {
      wfs = h_new();
      _tao_wfs_layout_fit, wfs, img, map=map;
      return _tao_wfs_layout_object(wfs, quiet=quiet);

    } else {
      return tao_wfs_layout_boxes(img, map, margin=margin, no_error=no_error,
                                  quiet=quiet);
    }
  }
}
wrap_args, tao_wfs_layout;


func tao_wfs_layout_boxes(img, map, boxsizes=, margin=, quiet=, no_error=)
/* DOCUMENT  h = tao_wfs_layout_boxes(img, map)

    yields an array of aligned bounding boxes found in image IMG according to
    the layout given by the true values in mask MAP. MAP is a 2D array of
    integers where true values mark the selected subimages. Default value for
    MAP is given by TAO_WFS_DEFAULT_MAP. The algorithm tries a range of
    bounding boxes from the maximum size that can fill IMG according to MAP, to
    half this size, and chose the best one. Keyword BOXSIZES can change this
    default.

    The assumptions of the method are:
    - All the bottom-left corners of the subimages are aligned vertically and
      horizontally, but the step can vary along vertical and horizontal axis.
    - All the subimages are squares of the same size.

KEYWORDS:
    boxsizes= - Gives a range of size to try to find the bounding boxes that
                best fit the WFS layout. SIZE can be a range or an array of
                integers.
    margin=   - Keyword MARGIN can be used to specify an inner margin in pixels
                for the boxes. A positive inner margin yields smaller boxes
                while a negative inner margin yields large boxes. The default
                value is margin=1.
    no_error= - If set to true, no error will stop the function, and a void
                will be returned in case of error.
    quiet=    - Suppress information messages if set to true.

SEE ALSO: tao_wfs_layout
*/
{
  // ---- Get and check img, create wfs object.
  if (is_hash(img)) {
    cls = tao_get_class(img);
    if (cls == TAO_IMAGE) {
      wfs = h_new(mean = img.dat, proc = img.proc);
    } else if (cls == TAO_IMAGE_STATISTICS) {
      wfs = h_new(mean = img.mean, proc = img.proc);
    } else {
      error, "unexpected instance of "+cls;
    }
    eq_nocopy, img, wfs.mean;

  } else if (identof(img) <= Y_DOUBLE && (dims = dimsof(img))(1) == 2) {
      wfs = h_new(mean = img, proc = -1);

  } else {
    error, "argument IMG must be an image";
  }

  // ---- Check map.
  if (is_void(map)) {
    map = TAO_WFS_DEFAULT_MAP;
  } else if (! is_integer(map) || (dims = dimsof(map))(1) != 2) {
    _tao_wfs_layout_error, no_error=no_error, quiet=quiet,
                      "argument MAP must be a 2D mask of integers";
    return;
  }
  // ---- Check margin.
  if (is_void(margin)) {
    margin = 0;
  } else if (!is_integer(margin)) {
    _tao_wfs_layout_error, no_error=no_error, quiet=quiet,
                           "expecting an integer for keyword MARGIN.";
    return;
  }
  // ---- Check of boxsizes done after.
  size = [];
  eq_nocopy, size, boxsizes;

  // ---- Compute weights for the matching filters.
  //
  // Also squeeze matching filters and the map: cuts the margins without
  // non-zero values.
  Wx = map(,sum);
  ncols = numberof(Wx);
  i = where(Wx);
  imin = min(i);
  imax = max(i);
  if (imin != 1 || imax != ncols) {
    Wx = Wx(imin:imax);
    map = map(imin:imax,);
  }
  ncols = numberof(Wx);

  Wy = map(sum,);
  nrows = numberof(Wy);
  j = where(Wy);
  jmin = min(j);
  jmax = max(j);
  if (jmin != 1 || jmax != nrows) {
    Wy = Wy(jmin:jmax);
    map = map(,jmin:jmax);
  }
  nrows = numberof(Wy);

  // ----  Integrate gradients along each axis.
  //
  // - Step of finite differences is 2 pixels so that gradients are centered.
  // - On the edge, step is 1 pixel.
  // - Altogether, gradients have the same number of pixels than profiles.

  Px = double(img(,sum));
  Gx = Px;
  Gx(1) = Px(2)-Px(1);
  Gx(0) = Px(0)-Px(-1);
  Gx(2:-1) = Px(3:)-Px(1:-2);
  Py = double(img(sum,));
  Gy = Py;
  Gy(1) = Py(2)-Py(1);
  Gy(0) = Py(0)-Py(-1);
  Gy(2:-1) = Py(3:)-Py(1:-2);

  // ---- Compute a range of sizes to explore.
  //
  // - Default is to try all the sizes between the maximum size to fill the
  //   image with ncols or nrows subimages and half this maximum size.
  dims = dimsof(img);
  xdim = dims(2);
  ydim = dims(3);
  if (is_void(size)) {
    size = int(max(double(xdim)/double(ncols), double(ydim)/double(nrows)));
    size = indgen(size/2:size);
  } else if (identof(size) == Y_RANGE) {
    size = indgen(size);
  } else if (!is_integer(size) || anyof(size<1) || anyof(size>min(xdim,ydim))) {
    _tao_wfs_layout_error, no_error=no_error, quiet=quiet,
        swrite(format="argument BOXSIZES must be integers in the range %d-%d\n",
               1,min(xdim,ydim));
    return;
  }
  nsize = numberof(size);

  // ---- Find best size and corresponding locations of bottom left corners.
  //
  best_score = [];
  for (i=1; i<=nsize; i++) {
    quality_x = Gx;            // reset
    quality_y = Gy;
    shift = size(i);
    quality_x(1:1-shift) = Gx(1:1-shift)-Gx(shift:);
    quality_y(1:1-shift) = Gy(1:1-shift)-Gy(shift:);
    kx = _tao_wfs_layout_findmaxima(quality_x, shift-1);
    ky = _tao_wfs_layout_findmaxima(quality_y, shift-1);
    if (numberof(kx) >= ncols && numberof(ky) >= nrows) {
      kx = kx(1:ncols);           // Select the ncols best values.
      ky = ky(1:nrows);           // Select the nrows best values.
      kx = kx(sort(kx));          // Sort in the same order as Wx.
      ky = ky(sort(ky));          // Sort in the same order as Wy.
      score = sum(Wx * quality_x(kx)) + sum(Wy * quality_y(ky));
      if (is_void(best_score) || score > best_score) {
        best_score = score;
        best_size = size(i);
        xorig = kx;
        yorig = ky;
      }
    }
  }
  if (is_void(best_score)) {
    _tao_wfs_layout_error, no_error=no_error, quiet=quiet,
                           "WFS layout could not be found.";
    return;
  }

  // ---- Make the boxes, the ROI, etc. and return the results.
  //
  sel = where(map);                         // selected subapertures.
  nsubs = numberof(sel);                    // number of selected subap.
  map(sel) = indgen(1:nsubs);               // index of subapertures.
  subsize = best_size - margin*2;           // size of subimages (squares).
  xmin = xorig(,-:1:nrows)(sel) + margin;   // bottom-left corner of subimages.
  ymin = yorig(-:1:ncols,)(sel) + margin;
  xmax = xmin + (subsize - 1);              // top-right corner of subimages.
  ymax = ymin + (subsize - 1);
  x0 = double(min(xmin));                   // general bounding box.
  y0 = double(min(ymin));
  x1 = double(max(xmax));
  y1 = double(max(ymax));
  mask = array(int, xdim, ydim);            // mask on the image.
  for (k = 1; k <= nsubs; ++k) {
    mask(xmin(k):xmax(k), ymin(k):ymax(k)) = 1n;
  }
  sp = 0.5*( ((x1-x0+1.)-ncols*subsize)/(ncols-1.)  // for information only.
            +((y1-y0+1.)-nrows*subsize)/(nrows-1.));

  return h_set(wfs, __class=TAO_WFS_LAYOUT,
               ncols=ncols, nrows=nrows, nsubs=nsubs, subsize=subsize,
               map=map,
               idx=map, // For compatibility. deprecated.
               xmin=xmin, ymin=ymin, xmax=xmax, ymax=ymax,
               x0=x0, y0=y0, x1=x1, y1=y1,
               xdim=xdim, ydim=ydim, mask=mask, sp=sp);
}



/*--+=^=+----------------------------------------------------------------------
 *                             PRIVATE FUNCTIONS
 *---------------------------------------------------------------------------*/


local _tao_is_local_min, _tao_is_local_max;
/* DOCUMENT bool = _tao_is_local_min(arr);
         or bool = _tao_is_local_min(arr, smooth);
         or bool = _tao_is_local_max(arr);
         or bool = _tao_is_local_max(arr, smooth);

     These functions yield an array of same size as ARR which is non-zero at
     local minima/maxima of ARR.  Argument SMOOTH (default is 1) is the radius
     of the local neighborhood to consider.

   SEE ALSO: morph_erosion, morph_dilation.
 */
func _tao_is_local_max(arr, smooth) /* DOCUMENTED */
{
  if (! tao_parse_integer(smooth, 1) || smooth < 1) {
    error, "smoothing radius must be a strictly positive integer";
  }
  return (arr == morph_dilation(arr, smooth));
}

func _tao_is_local_min(arr, smooth) /* DOCUMENTED */
{
  if (! tao_parse_integer(smooth, 1) || smooth < 1) {
    error, "smoothing radius must be a strictly positive integer";
  }
  return (arr == morph_erosion(arr, smooth));
}

func _tao_find_most_significant(dat, sel, num)
/* DOCUMENT i = _tao_find_most_significant(dat, sel, num);

     yields the indices of the most significant values in array DAT as selected
     by SEL.  The values to consider are DAT(J) where J = SEL if SEL is a list
     of indices or J = where(SEL) if SEL is an array of int's with same size as
     DAT.

     Argument NUM specifies the number of indices to return.  If NUM is an
     integer, N = min(NUM, numberof(J)) indices are returned corresponding to
     the N largest values of abs(DAT(J)).  Otherwise, if NUM is "auto", the
     number N of returned indices is automatically guessed assuming that the
     distribution of significant values is much distinct from the distribution
     of other (noisy) values.


   SEE ALSO: where.
 */
{
  id = identof(sel);
  if (id == Y_INT && tao_same_dims(dimsof(dat), dimsof(sel))) {
    /* Assume SEL is a boolean array. */
    sel = where(sel);
  } else if (! ((id == Y_LONG && is_vector(sel)) || id == Y_RANGE)) {
    error, "expecting a boolean array or a selection";
  }
  n = numberof(sel);
  if (n > 1 && ((auto = (num == "auto")) || n > num)) {
    /* Sort values in increasing magnitude order. */
    dat = abs(dat(sel));
    i = sort(abs(dat));
    sel = sel(i);
    dat = dat(i);
    if (auto) {
      /* Find the first index where there is the maximum difference between
         successive sorted values and cut the sequence here. */
      i = dat(dif)(mxx);
    } else {
      i = n - num;
    }
    sel = sel(i + 1 : n);
  }
  return sel;
}


func _tao_wfs_layout_error(msg, no_error=, quiet=)
/* DOCUMENT  _tao_wfs_layout_error, msg

    exits the current interpreted program, printing MSG, unless keyword
    NO_ERROR is set to true. In this case, the caller is not stopped ans MSG
    is only printed, unless keyword QUIET is set to true. Thus the function
    does nothing if both NO_ERROR and QUIET are set to true.

SEE ALSO:
*/
{
  if (no_error) {
    if (!quiet) write, "ERROR %s\n", msg;
    return;
  } else {
    error, msg;
  }
}
errs2caller, _tao_wfs_layout_error;


func _tao_wfs_layout_findmaxima(a, gap, nidx)
/* DOCUMENT  idx = _tao_wfs_layout_findmaxima(a, gap, nidx)

    returns the indices of the maxima in array A (considered as a vector),
    sorted in decreasing order. Each time a maximum is selected at index IMAX,
    the neighborhood is cleared from IMAX-GAP to IMAX+GAP. If NIDX is not
    given, all the maxima are returned until all the array is cleared. If NIDX
    is given, at most NIDX maxima are returned.

SEE ALSO:
*/
{
  n = numberof(a);
  if (is_void(nidx)) {
    nidx = n;
  } else {
    if (!is_integer(nidx) && nidx <= 0) {
      error, "third argument must be a strictly positive integer";
    }
  }
  ap = a;            // copy needed.
  floor = min(ap);
  idx = [];
  for (i=1; i<=nidx; i++) {
    ibest = ap(mxx);
    if (ap(ibest) == floor) break;
    grow, idx, ibest;
    // clear neighborhood
    ap(max(1,ibest-gap):min(n,ibest+gap)) = floor;
  }
  return idx;
}


func _tao_wfs_layout_fit(wfs, img, map=, smooth=)
/* DOCUMENT _tao_wfs_layout_fit, wfs, img;

     Private routine to automatically compute the parameters of the wavefront
     sensor layout given an image IMG of the detector with all sub-images (a
     long exposure for instance).

     Argument WFS is a hash table which is updated with relevant parameters.

     Keyword MAP may be specified to provide the geometry of the valid
     subimages.  It is a NCOLS-by-NROWS array which is non-zero for valid
     subimages and zero elsewhere.  Here NCOLS and NROWS are the number of
     sub-images along the horizontal and vertical dimension.  If unspecified,
     NCOLS and NROWS will be automatically guessed from the most significant
     horizontal and vertical edges detected in image IMG.

     Keyword SMOOTH may be used to specify the radius (an integer ≥ 1) of the
     region where a unique local minimum or maximum of the image gradient along
     a given direction is expected (see _tao_is_local_min and _tao_is_local_max).

SEE ALSO: tao_wfs_layout, _tao_find_most_significant, _tao_is_local_min,
          _tao_is_local_max.
*/
{
  if (is_hash(img)) {
    cls = tao_get_class(img);
    if (cls == TAO_IMAGE) {
      h_set, wfs,
        mean = img.dat,
        proc = img.proc,
        xdim = img.xdim,
        ydim = img.ydim;
    } else if (cls == TAO_IMAGE_STATISTICS) {
      h_set, wfs,
        mean = img.mean,
        proc = img.proc,
        xdim = img.xdim,
        ydim = img.ydim;
    } else {
      error, "unexpected instance of "+cls;
    }
  } else if (identof(img) <= Y_DOUBLE && (dims = dimsof(img))(1) == 2) {
      h_set, wfs,
        mean = img,
        proc = -1,
        xdim = dims(2),
        ydim = dims(3);
  } else {
    error, "argument IMG must be an image";
  }
  if (is_void(map)) {
    /* Number of rows and columns will be automatically guessed. */
    ncols = "auto";
    nrows = "auto";
  } else if (identof(map) <= Y_DOUBLE && (dims = dimsof(map))(1) == 2) {
    ncols = dims(2);
    nrows = dims(3);
  } else {
    error, "keyword MAP must be a 2D array";
  }

  /*
   * To find the edges of the sub-images, we compute the spatial derivatives
   * (with the Sobel filter) of the time averaged WFS image (or equivalent,
   * e.g. flat acquired with the Sun) and average the horizontal gradients
   * vertically (resp. the vertical gradients horizontally), then find the most
   * significant positive and negative gradients which indicate the edges of
   * the sub-images along the horizontal (resp. vertical) direction.
   */
  grd = tao_sobel(wfs.mean);
  gx = grd(,avg,1);
  lmin = (gx < 0)&_tao_is_local_min(gx, smooth);
  lmax = (gx > 0)&_tao_is_local_max(gx, smooth);
  imin = _tao_find_most_significant(gx, lmin, ncols);
  imax = _tao_find_most_significant(gx, lmax, ncols);
  gy = grd(avg,,2);
  lmin = (gy < 0)&_tao_is_local_min(gy, smooth);
  lmax = (gy > 0)&_tao_is_local_max(gy, smooth);
  jmin = _tao_find_most_significant(gy, lmin, nrows);
  jmax = _tao_find_most_significant(gy, lmax, nrows);

  /* Sort indices in increasing order. */
  heapsort, imin;
  heapsort, imax;
  heapsort, jmin;
  heapsort, jmax;

  if (! quiet) {
    write, format=("find %d positive horizontal jumps, " +
                   "separated by %.3f pixels on average\n"),
      numberof(imax), avg(imax(dif));
    write, format=("find %d negative horizontal jumps, " +
                   "separated by %.3f pixels on average\n"),
      numberof(imin), avg(imin(dif));
    write, format=("find %d positive vertical jumps, " +
                   "separated by %.3f pixels on average\n"),
      numberof(jmax), avg(jmax(dif));
    write, format=("find %d negative vertical jumps, " +
                   "separated by %.3f pixels on average\n"),
      numberof(jmin), avg(jmin(dif));
  }

  if (is_void(map)) {
    ncols = max(numberof(imax), numberof(imin));
    nrows = max(numberof(jmax), numberof(jmin));
  }

  s = grow(array(+1, numberof(imax)),
           array(-1, numberof(imin)));
  k = grow(imax, imin);
  j = heapsort(k);
  s = s(j)(dif);
  k = k(j)(dif);
  j = where(s > 0);
  xsep_num = numberof(j);
  xsep_sum = (xsep_num > 0 ? double(sum(k(j))) : 0.0);
  j = where(s < 0);
  xsiz_num = numberof(j);
  xsiz_sum = (xsiz_num > 0 ? double(sum(k(j))) : 0.0);

  s = grow(array(+1, numberof(jmax)),
           array(-1, numberof(jmin)));
  k = grow(jmax, jmin);
  j = heapsort(k);
  s = s(j)(dif);
  k = k(j)(dif);
  j = where(s > 0);
  ysep_num = numberof(j);
  ysep_sum = (ysep_num > 0 ? double(sum(k(j))) : 0.0);
  j = where(s < 0);
  ysiz_num = numberof(j);
  ysiz_sum = (ysiz_num > 0 ? double(sum(k(j))) : 0.0);

  sep = (xsep_num + ysep_num > 0 ?
         double(xsep_sum + ysep_sum)/(xsep_num + ysep_num) : 0.0);
  siz = (xsiz_num + ysiz_num > 0 ?
         double(xsiz_sum + ysiz_sum)/(xsiz_num + ysiz_num) : 0.0);

  if (! quiet) {
    write, format="separation %.3f pixels, size %.3f pixels\n",
      sep, siz + sep;
  }

  /* FIXME: Improve the estimate by a global fit instead of simply using the
     extreme values.  Another possibility is to convolve with a smoothed
     box. */
  return h_set(wfs,
               x0 = double(imax(1)),
               y0 = double(jmax(1)),
               x1 = double(imin(0)),
               y1 = double(jmin(0)),
               sp = double(sep));
}


func _tao_wfs_layout_limits(wfs, &xmin, &xmax, &ymin, &ymax, crop=)
/* DOCUMENT  _tao_wfs_layout_limits, wfs, &xmin, &xmax, &ymin, &ymax

    initializes XMIN, XMAX, YMIN, and YMAX from the values found in object WFS as
    returned by tao_wfs_layout, with the proper offset if keyword CROP is set
    to true.

SEE ALSO: tao_wfs_draw_grid, tao_wfs_draw_labels
*/
{
  if (crop) {
    xoff = int(wfs.x0) - 1.;
    yoff = int(wfs.y0) - 1.;
  } else {
    xoff = 0.;
    yoff = 0.;
  }
  xmin = wfs.xmin - (xoff + 0.5);  // half a pixel for drawing around pixels.
  xmax = wfs.xmax - (xoff - 0.5);
  ymin = wfs.ymin - (yoff + 0.5);
  ymax = wfs.ymax - (yoff - 0.5);
}


func _tao_wfs_layout_object(wfs, xdim, ydim, x0, y0, x1, y1, sp, map, quiet=)
/* DOCUMENT  h = _tao_wfs_layout_object(wfs, xdim, ydim, x0, y0, x1, y1,sp,map)
     - or -  h = _tao_wfs_layout_object(wfs)

    checks the arguments, builds and returns the object returned by
    tao_wfs_layout function (see its help). This worker will create the hash
    table WFS if it is void, or fill it with the required quantities. It is
    called if all the arguments are given, or if tao_wfs_layout is called with
    keyword LEGACY sets to true. In this case, the hash table is filled before
    by function _tao_wfs_layout_fit.

SEE ALSO: tao_wfs_layout
*/
{
  if (is_void(wfs)) {
    wfs = h_new();
  } else {
    if (is_void(xdim)) eq_nocopy, xdim, wfs.xdim;
    if (is_void(ydim)) eq_nocopy, ydim, wfs.ydim;
    if (is_void(x0)) x0 = wfs.x0;
    if (is_void(x1)) x1 = wfs.x1;
    if (is_void(y0)) y0 = wfs.y0;
    if (is_void(y1)) y1 = wfs.y1;
    if (is_void(sp)) sp = wfs.sp;
    if (is_void(map)) eq_nocopy, map, wfs.map;
  }

  /* Get and verify all arguments. */
  if (! tao_parse_integer(xdim) || xdim < 1) {
    error, "invalid XDIM";
  }
  if (! tao_parse_integer(ydim) || ydim < 1) {
    error, "invalid YDIM";
  }
  if (! tao_parse_real(x0) || ! tao_parse_real(y0) ||
      ! tao_parse_real(x1) || ! tao_parse_real(y1) ||
      x0 >= x1 || x0 < 1 || x1 > xdim ||
      y0 >= y1 || y0 < 1 || y1 > ydim) {
    error, "invalid bounding-box (X0,Y0,X1,Y1)";
  }
  if (! tao_parse_real(sp) || sp < 0) {
    error, "invalid spacing SP";
  }
  if (is_void(map)) {
    map = TAO_WFS_DEFAULT_MAP;
  }
  if (! is_integer(map) || (dims = dimsof(map))(1) != 2) {
    error, "MAP must be a 2D array of integers";
  }
  ncols = dims(2);
  nrows = dims(3);

  /* Build array of indices of valid sub-pupils from the given map. */
  idx = array(long, dims);
  sel = where(map != 0);
  nsubs = numberof(sel);
  if (nsubs > 0) {
    idx(sel) = indgen(nsubs);
  }

  /* Display global grid parameters. */
  xlen = double(abs(x1 - x0) - (ncols - 1)*sp)/ncols; // sub-image width
  ylen = double(abs(y1 - y0) - (nrows - 1)*sp)/nrows; // sub-image height
  len = lround(min(xlen, ylen));
  if (! quiet) {
    write, format="grid of %d x %d sub-images\n", ncols, nrows;
    write, format="sub-images size = %.1f x %.1f ~ %d x %d\n",
      xlen, ylen, len, len;
  }

  /* Compute coordinates of sub-image corners in fractional pixel units. */
  xmin = x0 + indgen(0:ncols-1)*(xlen + sp);
  xmax = xmin + xlen;
  ymin = y0 + indgen(0:nrows-1)*(ylen + sp);
  ymax = ymin + ylen;

  /* Compute fractional pixel coordinates in integer indices. */
  xmin = _tao_wfs_layout_subimage_start_index(xmin, xmax, len);
  xmax = xmin + (len - 1);
  ymin = _tao_wfs_layout_subimage_start_index(ymin, ymax, len);
  ymax = ymin + (len - 1);

  /* Bounding boxes of every sub-images. */
  xmin = xmin(,-:1:nrows)(sel);
  xmax = xmax(,-:1:nrows)(sel);
  ymin = ymin(-:1:ncols,)(sel);
  ymax = ymax(-:1:ncols,)(sel);

  /* Computation of the mask. */
  mask = array(int, xdim, ydim);
  for (k = 1; k <= nsubs; ++k) {
    mask(xmin(k):xmax(k), ymin(k):ymax(k)) = 1n;
  }

  /* Build object. */
  return h_set(wfs,
               __class=TAO_WFS_LAYOUT,
               ncols=ncols, nrows=nrows, nsubs=nsubs, map=idx, idx=idx,
               x0=x0, y0=y0, x1=x1, y1=y1, sp=sp, mask=mask, subsize = len,
               xdim=xdim, ydim=ydim,
               xmin=xmin, ymin=ymin,
               xmax=xmax, ymax=ymax);
}
errs2caller, _tao_wfs_layout_object;


func _tao_wfs_layout_subimage_start_index(xmin, xmax, len)
/* DOCUMENT imin = _tao_wfs_layout_subimage_start_index(xmin, xmax, len);

     yields integer IMIN such that integer range IMIN:IMAX (with
     IMAX = IMIN + LEN - 1) is optimally centered in the interval
     [XMIN,XMAX].  LEN is the length of the integer range, XMIN and
     XMAX are the endpoints of the interval.

   SEE ALSO: tao_wfs_layout.
 */
{
  imin = long(floor((xmin + xmax - len)*0.5));
  u = imin - xmin;
  v = imin + (len - 1) - xmax;
  imin += ((abs(u + 1) + abs(v + 1)) < (abs(u) + abs(v)));
  imax = imin + (len - 1);
  return imin;
}


