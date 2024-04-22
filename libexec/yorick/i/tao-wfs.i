/*
 * tao-wfs.i --
 *
 * Yorick software for Themis Adaptive Optics.
 *
 *-----------------------------------------------------------------------------
 *
 * This file if part of the TAO software (https://git-cral.univ-lyon1.fr/tao)
 * licensed under the MIT license.
 * Updates can be found in https://git-cral.univ-lyon1.fr/tao/tao-yorick.
 *
 * Copyright (C) 2020, Michel Tallon, Isabelle Tallon-Bosc.
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
 *       - tao_wfs_cog_on_spots
 *       - tao_wfs_stack_data
 *
 * - PRIVATE FUNCTIONS
 *       -
 */

/*--+=^=+----------------------------------------------------------------------
 *                               DEFINITIONS
 *---------------------------------------------------------------------------*/


/*--+=^=+----------------------------------------------------------------------
 *                             PUBLIC FUNCTIONS
 *---------------------------------------------------------------------------*/

func tao_wfs_cog_on_spots(frame, layout, npix, absolute=)
/* DOCUMENT  xy = tao_wfs_cog_on_spots(frame, layout, npix)

    computes the centers of gravity of the subimages found in FRAME (a 2-D
    array). LAYOUT is a TAO_WFS_LAYOUT object describing the geometry of the
    wavefront sensor (see tao_wfs_layout). For each subimage, after removing
    the median of the pixel values, the center of gravity is computed over the
    NPIX brightest pixels. If not given, NPIX is set to 40.

    Default is to compute the coordinates in pixels compared to the center of
    the subimages. Keyword `absolute` returns the positions of the centers of
    gravity compared to the first pixel (bottom-left) of the frame.


SEE ALSO:
*/
{
  // ---- Check arguments
  if (is_void(npix)) npix = 40;
  if (!tao_wfs_is_layout(layout)) {
    error, "second argument is not a TAO_WFS_LAYOUT object";
  }

  // ---- Split the frame into subimages
  subimages = tao_wfs_stack_data(frame, layout);

  dims = dimsof(subimages);
  nx = dims(2);
  ny = dims(3);
  nsub = dims(0);
  x = indgen(nx) - 0.5*(nx+1);  // center of each subimage.
  y = indgen(ny) - 0.5*(ny+1);

  cog = array(double, 2, nsub);
  for (k=1; k<=nsub; k++) {
    img = subimages(,,k);
    idx = sort(img(*))(1-npix:0);
    bias = quick_median(img);
    img = img(idx) - bias;
    i = (idx-1)%nx + 1;
    j = (idx-1)/nx + 1;
    den = max(sum(img), 0.);
    if (den == 0.) {
      cog(,k) = [0.,0.];
    } else {
      cog(1,k) = sum(x(i)*img)/den;
      cog(2,k) = sum(y(j)*img)/den;
    }
  }
  if (absolute) {
    cog(1,) += layout.xmin + (0.5*(nx-1) - 1);
    cog(2,) += layout.ymin + (0.5*(ny-1) - 1);
  }
  return cog;
}


func tao_wfs_stack_data(frame, layout)
/* DOCUMENT  stack = tao_wfs_stack_data(frame, layout)

    yields a stack of wavefront sensor sub-images built from input array FRAME
    and according to the sub-image parameters in wavefront sensor layout (a
    TAO_WFS_LAYOUT object). The dimensions of the returned array will be
    N-by-N-by-NSUBS, N-by-N is the size of each sub-image, and NSUBS is the
    number of sub-images.

SEE ALSO: tao_wfs_layout
*/
{
  // ---- Get bounding boxes and indices of the subimages.
  local xmin; eq_nocopy, xmin, layout.xmin;
  local xmax; eq_nocopy, xmax, layout.xmax;
  local ymin; eq_nocopy, ymin, layout.ymin;
  local ymax; eq_nocopy, ymax, layout.ymax;
  local map;  eq_nocopy, map,  layout.map;

  // ---- Check input array.
  dims = dimsof(frame);
  if (identof(frame) > Y_DOUBLE || dims(1) != 2) {
    error, "FRAME must be an array of 2 dimensions";
  }
  if (dims(2) != layout.xdim || dims(3) != layout.ydim) {
    error, "dimensions of FRAME must match the WFS image size";
  }

  // ---- Define the destination array.
  subsize = layout.subsize;
  dst = array(structof(frame), subsize, subsize, layout.nsubs);

  // ---- Build stack.
  ncols = layout.ncols;
  nrows = layout.nrows;
  for (j = 1; j <= nrows; ++j) {
    for (i = 1; i <= ncols; ++i) {
      if ((k = map(i,j)) != 0) {
        ysrc = ymin(k):ymax(k);
        xsrc = xmin(k):xmax(k);
        dst(,,k) = frame(xsrc, ysrc);
      }
    }
  }
  return dst;
}


/*--+=^=+----------------------------------------------------------------------
 *                             PRIVATE FUNCTIONS
 *---------------------------------------------------------------------------*/
