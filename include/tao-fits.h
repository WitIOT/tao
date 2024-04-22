// tao-fits.h -
//
// Definitions for using FITS files in TAO (TAO is a library for Adaptive
// Optics software).
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2018-2022, Éric Thiébaut.

#ifndef TAO_FITS_H_
#define TAO_FITS_H_ 1

#include <tao-arrays.h>

// FITSIO header file is only included if the macro _FITSIO2_H is undefined.
// This is to be able to compile TAO ebev though FITSIO is not available.
#ifndef _FITSIO2_H
# include <fitsio2.h>
#endif

#include <stdbool.h>

TAO_BEGIN_DECLS

/**
 * @defgroup FITS  FITS files
 *
 * @ingroup Arrays
 *
 * @brief Reading/writing arrays from/to FITS files.
 * @{
 */

/**
 * Load a multi-dimensional array from a FITS file.
 *
 * This function loads the contents of a FITS IMAGE data into a new
 * multi-dimensional array.  The returned array has a reference count of 1, the
 * caller is responsible for unreferencing the array when no longer needed by
 * calling tao_unreference_array().
 *
 * @param filename  Name of the FITS file.
 * @param extname   Name of the FITS extension to read.  Can be `NULL` to read
 *                  the first FITS IMAGE of the file.
 *
 * @return The address of a new multi-dimensional array; `NULL` in case of
 *         errors.
 *
 * @see tao_create_array(), tao_unreference_array(),
 *      tao_save_array_to_fits_file(), tao_load_array_from_fits_handle().
 */
extern tao_array* tao_load_array_from_fits_file(
    const char* filename,
    char* extname);

/**
 * Load a multi-dimensional array from a provided FITS handle.
 *
 * This function loads the contents of the current FITS HDU into a new
 * multi-dimensional array.  The returned array has a reference count of 1, the
 * caller is responsible for unreferencing the array when no longer needed by
 * calling tao_unreference_array().
 *
 * @param fptr      FITS file handle.
 *
 * @return The address of a new multi-dimensional array; `NULL` in case of
 *         errors.
 *
 * @see tao_create_array(), tao_unreference_array(),
 *      tao_save_array_to_fits_handle(), tao_load_array_from_fits_file().
 */
extern tao_array* tao_load_array_from_fits_handle(
    fitsfile* fptr);

/**
 * Save a multi-dimensional array to a FITS file.
 *
 * This function writes the contents of the supplied array into a new FITS
 * file.
 *
 * @param arr       Pointer to an array referenced by the caller.
 * @param filename  FITS file handle.
 * @param overwrite Non-zero to allow for overwriting the destination.
 *
 * @return `TAO_OK` in case of success, `TAO_ERROR` in case of failure.
 *
 * @see tao_create_array(), tao_unreference_array(),
 *      tao_save_array_to_fits_handle(), tao_load_array_from_fits_file().
 */
extern tao_status tao_save_array_to_fits_file(
    const tao_array* arr,
    const char* filename,
    bool overwrite);

/**
 * Save a multi-dimensional array to a provided FITS handle.
 *
 * This function writes the contents of the supplied array into a new FITS HDU.
 * After having initialized the basic image information in the new HDU, a user
 * supplied callback function is called (if nont `NULL`) to let the caller
 * customize the keywords of the header.
 *
 * @param arr       Pointer to an array referenced by the caller.
 * @param fptr      FITS file handle.
 * @param callback  Function called to update the header of the FITS HDU
 *                  where is written the array.  May be `NULL` to not use it.
 * @param ctx       Context argument supplied for the callback function.
 *
 * @return `TAO_OK` in case of success, `TAO_ERROR` in case of failure.
 *
 * @see tao_create_array(), tao_unreference_array(),
 *      tao_save_array_to_fits_file(), tao_load_array_from_fits_handle().
 */
extern tao_status tao_save_array_to_fits_handle(
    const tao_array* arr,
    fitsfile* fptr,
    int (*callback)(fitsfile*, void*),
    void* ctx);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_FITS_H_
