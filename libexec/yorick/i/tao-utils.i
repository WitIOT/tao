/*
 * tao-utils.i --
 *
 * Yorick interface to TAO real-time software.  TAO is a library for Adaptive
 * Optics software
 *
 *-----------------------------------------------------------------------------
 *
 * This file if part of the TAO software (https://git-cral.univ-lyon1.fr/tao)
 * licensed under the MIT license.
 * Updates can be found in https://git-cral.univ-lyon1.fr/tao/tao-yorick.
 *
 * Copyright (C) 2018-2022, Michel Tallon, Isabelle Tallon-Bosc, Éric Thiébaut.
 */

/* SECTIONS
 * --------
 *
 * - PUBLIC FUNCTIONS
 *       - tao_pack_influence
 *
 */

/*--+=^=+----------------------------------------------------------------------
 *                             PUBLIC FUNCTIONS
 *---------------------------------------------------------------------------*/

func tao_pack_influence(cube, mask)
/* DOCUMENT  im = tao_pack_influence(cube, mask)

    applies the mask `mask` to the cube `cube` to get the influence matrix `im`
    (an array of double) in a format that can be saved as a simple array.

    `cube` is a 3D array NX-by-NY-by-M, that is a set of M maps NX-by-NY pixels
    in size. Each map is the shape of the deformable mirror when actuator k
    is activated for k=1, ..., M. The deformation is assumed to be given in
    microns. `mask` is an array Nx-by-NY where zero values are false, and
    non-zero values are true.

    The mask is applied to each NX-by-NY map of the given `cube`, replacing the
    false values with NaN (Not-A-Number) values. Then the full lines of NaN
    values at the edge of each map are trimmed out. So the two first dimensions
    of the returned array `im` may be smaller than those of `cube`. Finally,
    piston (i.e. average on the selected pixels) is removed for each influence
    function.

Example:

       cube = fits_read("influence_maps.fits");
       dims = dimsof(cube);
       nx = dims(2);
       ny = dims(3);
       x = indgen(nx); x -= avg(x);
       y = indgen(ny); y -= avg(y);
       mask = abs(x, y(-,)) < nx/2;
       im = tao_pack_influence(cube, mask);
       fits_write, "influence_matrix.fits", im;

SEE ALSO:
*/
{
  // ---- Check the arguments
  dimscube = dimsof(cube);
  dimsmask = dimsof(mask);
  if (dimscube(1) != 3) {
    error, "first argument must have 3 dimensions";
  }
  if (dimsmask(1) != 2) {
    error, "second argument must have 2 dimensions";
  }
  if (is_void(dimsof(cube, mask))) {
    error, "first and second arguments are not conformable";
  }

  // ---- Trim mask and cube.
  ix = where(mask(,sum) > 0);
  iy = where(mask(sum,) > 0);
  mask = mask(ix(1):ix(0),iy(1):iy(0));
  cube = double(cube(ix(1):ix(0),iy(1):iy(0),));

  // ---- Remove piston.
  i = where(mask != 0);
  cube -= cube(*,)(i,)(avg,)(-,-,);

  // ---- Set NaN where mask is false.
  i = where(mask == 0);
  mask(*) = 0;  // means leave unchanged.
  mask(i) = 2;  // means set to qNaN (quiet NaN).
  ieee_set, cube, mask;
  return cube;
}


