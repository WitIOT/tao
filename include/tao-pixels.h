// tao-pixels.h -
//
// Encoding of image pre-processing methods.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2020-2022, Éric Thiébaut.

#ifndef TAO_PIXELS_H_
#define TAO_PIXELS_H_ 1

#include <stdint.h>

/**
 * @defgroup Pixels  Copy, conversion, and pre-processing of pixels
 *
 * @ingroup Cameras
 *
 * @brief Functions for copying, converting, or pre-processing raw images.
 *
 * @{
 *
 */

//-----------------------------------------------------------------------------
// COPY PIXELS

/**
 * @brief Copy raw pixels.
 *
 * This function copies raw pixel values.  The input and output images have
 * 8-bit unsigned integer pixels.  Lines of pixels may be strided in the
 * acquisition buffer but pixels are assumed contiguous in the output arrays.
 *
 * @param dat     Output array of pre-processed pixels.
 * @param width   Number of pixels per line of the image.
 * @param height  Number of lines of pixels in the image.
 * @param raw     Input buffer of raw pixels.
 * @param stride  Number of bytes between successive lines in input image
 *                buffer @a raw.
 */
extern void tao_pixels_copy_u8_to_u8(
    uint8_t*        restrict dat,
    long                     width,
    long                     height,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Copy raw pixels.
 *
 * This function behaves as tao_pixels_copy_u8_to_u8() except that the input
 * and output images have 16-bit unsigned integer pixels.
 */
extern void tao_pixels_copy_u16_to_u16(
    uint16_t*       restrict dat,
    long                     width,
    long                     height,
    const uint16_t* restrict raw,
    long                     stride);

/**
 * @brief Copy raw pixels.
 *
 * This function behaves as tao_pixels_copy_u8_to_u8() except that the input
 * and output images have 32-bit unsigned integer pixels.
 */
extern void tao_pixels_copy_u32_to_u32(
    uint32_t*       restrict dat,
    long                     width,
    long                     height,
    const uint32_t* restrict raw,
    long                     stride);

//-----------------------------------------------------------------------------
// Convert PIXELS

/**
 * @brief Convert raw pixels.
 *
 * This function converts raw pixel values.  The input image has 8-bit unsigned
 * integer pixels while the output images has 16-bit unsigned integer pixels.
 * Lines of pixels may be strided in the acquisition buffer but pixels are
 * assumed contiguous in the output arrays.
 *
 * @param dat     Output array of pre-processed pixels.
 * @param width   Number of pixels per line of the image.
 * @param height  Number of lines of pixels in the image.
 * @param raw     Input buffer of raw pixels.
 * @param stride  Number of bytes between successive lines in input image
 *                buffer @a raw.
 */
extern void tao_pixels_convert_u8_to_u16(
    uint16_t*       restrict dat,
    long                     width,
    long                     height,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Convert raw pixels.
 *
 * This function behaves as tao_pixels_convert_u8_to_u16() except that the
 * input image has 8-bit unsigned integer pixels while the output images has
 * 32-bit unsigned integer pixels.
 */
extern void tao_pixels_convert_u8_to_u32(
    uint32_t*       restrict dat,
    long                     width,
    long                     height,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Convert raw pixels.
 *
 * This function behaves as tao_pixels_convert_u8_to_u16() except that the
 * input image has 8-bit unsigned integer pixels while the output images has
 * single precision floating-point pixels.
 */
extern void tao_pixels_convert_u8_to_flt(
    float*          restrict dat,
    long                     width,
    long                     height,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Convert raw pixels.
 *
 * This function behaves as tao_pixels_convert_u8_to_u16() except that the
 * input image has 8-bit unsigned integer pixels while the output images has
 * double precision floating-point pixels.
 */
extern void tao_pixels_convert_u8_to_dbl(
    double*         restrict dat,
    long                     width,
    long                     height,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Convert raw pixels.
 *
 * This function behaves as tao_pixels_convert_u8_to_u16() except that the
 * input image has packed 12-bit unsigned integer pixels while the output
 * images has 16-bit unsigned integer pixels.
 */
extern void tao_pixels_convert_p12_to_u16(
    uint16_t*       restrict dat,
    long                     width,
    long                     height,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Convert raw pixels.
 *
 * This function behaves as tao_pixels_convert_u8_to_u16() except that the
 * input image has packed 12-bit unsigned integer pixels while the output
 * images has 32-bit unsigned integer pixels.
 */
extern void tao_pixels_convert_p12_to_u32(
    uint32_t*       restrict dat,
    long                     width,
    long                     height,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Convert raw pixels.
 *
 * This function behaves as tao_pixels_convert_u8_to_u16() except that the
 * input image has 8-bit unsigned integer pixels while the output images has
 * single precision floating-point pixels.
 */
extern void tao_pixels_convert_p12_to_flt(
    float*          restrict dat,
    long                     width,
    long                     height,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Convert raw pixels.
 *
 * This function behaves as tao_pixels_convert_u8_to_u16() except that the
 * input image has 8-bit unsigned integer pixels while the output images has
 * single precision floating-point pixels.
 */
extern void tao_pixels_convert_p12_to_dbl(
    double*         restrict dat,
    long                     width,
    long                     height,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Convert raw pixels.
 *
 * This function behaves as tao_pixels_convert_u8_to_u16() except that the
 * input image has 16-bit unsigned integer pixels while the output images has
 * 32-bit unsigned integer pixels.
 */
extern void tao_pixels_convert_u16_to_u32(
    uint32_t*       restrict dat,
    long                     width,
    long                     height,
    const uint16_t* restrict raw,
    long                     stride);

/**
 * @brief Convert raw pixels.
 *
 * This function behaves as tao_pixels_convert_u8_to_u16() except that the
 * input image has 16-bit unsigned integer pixels while the output images has
 * single precision floating-point pixels.
 */
extern void tao_pixels_convert_u16_to_flt(
    float*          restrict dat,
    long                     width,
    long                     height,
    const uint16_t* restrict raw,
    long                     stride);

/**
 * @brief Convert raw pixels.
 *
 * This function behaves as tao_pixels_convert_u8_to_u16() except that the
 * input image has 16-bit unsigned integer pixels while the output images has
 * double precision floating-point pixels.
 */
extern void tao_pixels_convert_u16_to_dbl(
    double*         restrict dat,
    long                     width,
    long                     height,
    const uint16_t* restrict raw,
    long                     stride);

/**
 * @brief Convert raw pixels.
 *
 * This function behaves as tao_pixels_convert_u8_to_u16() except that the
 * input image has 32-bit unsigned integer pixels while the output images has
 * single precision floating-point pixels.
 */
extern void tao_pixels_convert_u32_to_flt(
    float*          restrict dat,
    long                     width,
    long                     height,
    const uint32_t* restrict raw,
    long                     stride);

/**
 * @brief Convert raw pixels.
 *
 * This function behaves as tao_pixels_convert_u8_to_u16() except that the
 * input image has 32-bit unsigned integer pixels while the output images has
 * double precision floating-point pixels.
 */
extern void tao_pixels_convert_u32_to_dbl(
    double*         restrict dat,
    long                     width,
    long                     height,
    const uint32_t* restrict raw,
    long                     stride);

//-----------------------------------------------------------------------------
// APPLY AFFINE CORRECTION

/**
 * @brief Apply affine correction to raw pixels.
 *
 * This function applies an affine pixel correction.  Output image has single
 * precision floating-point pixels.  Input image has 8-bit unsigned integer
 * pixels.
 *
 * Affine pixel correction is expressed by the following pseudo-code:
 *
 *     dat[i] = (raw[i] - b[i])*a[i]
 *     wgt[i] = q[i]/(max(dat[i],0) + r[i])
 *
 * @param dat     Output array of pre-processed pixels.
 * @param width   Number of pixels per line of the image.
 * @param height  Number of lines of pixels in the image.
 * @param a       Pixelwise linear correction.
 * @param b       Pixelwise bias correction
 * @param raw     Input buffer of raw pixels.
 * @param stride  Number of bytes between successive lines in input image
 *                buffer @a raw.
 */
extern void tao_pixels_preprocess_affine_u8_to_flt(
    float*          restrict dat,
    long                     width,
    long                     height,
    const float*    restrict a,
    const float*    restrict b,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels.
 *
 * This function behaves as tao_pixels_preprocess_affine_u8_to_flt() except
 * that the input image has 8-bit unsigned integer pixels while the output
 * images has double precision floating-point pixels.
 */
extern void tao_pixels_preprocess_affine_u8_to_dbl(
    double*         restrict dat,
    long                     width,
    long                     height,
    const double*   restrict a,
    const double*   restrict b,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels.
 *
 * This function behaves as tao_pixels_preprocess_affine_u8_to_flt() except
 * that the input image has packed 12-bit unsigned integer pixels while the
 * output images has single precision floating-point pixels.
 */
extern void tao_pixels_preprocess_affine_p12_to_flt(
    float*          restrict dat,
    long                     width,
    long                     height,
    const float*    restrict a,
    const float*    restrict b,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels.
 *
 * This function behaves as tao_pixels_preprocess_affine_u8_to_flt() except
 * that the input image has packed 12-bit unsigned integer pixels while the
 * output images has double precision floating-point pixels.
 */
extern void tao_pixels_preprocess_affine_p12_to_dbl(
    double*         restrict dat,
    long                     width,
    long                     height,
    const double*   restrict a,
    const double*   restrict b,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels.
 *
 * This function behaves as tao_pixels_preprocess_affine_u8_to_flt() except
 * that the input image has 16-bit unsigned integer pixels while the output
 * images has single precision floating-point pixels.
 */
extern void tao_pixels_preprocess_affine_u16_to_flt(
    float*          restrict dat,
    long                     width,
    long                     height,
    const float*    restrict a,
    const float*    restrict b,
    const uint16_t* restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels.
 *
 * This function behaves as tao_pixels_preprocess_affine_u8_to_flt() except
 * that the input image has 16-bit unsigned integer pixels while the
 * output images has double precision floating-point pixels.
 */
extern void tao_pixels_preprocess_affine_u16_to_dbl(
    double*         restrict dat,
    long                     width,
    long                     height,
    const double*   restrict a,
    const double*   restrict b,
    const uint16_t* restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels.
 *
 * This function behaves as tao_pixels_preprocess_affine_u8_to_flt() except
 * that the input image has 32-bit unsigned integer pixels while the output
 * images has single precision floating-point pixels.
 */
extern void tao_pixels_preprocess_affine_u32_to_flt(
    float*          restrict dat,
    long                     width,
    long                     height,
    const float*    restrict a,
    const float*    restrict b,
    const uint32_t* restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels.
 *
 * This function behaves as tao_pixels_preprocess_affine_u8_to_flt() except
 * that the input image has 32-bit unsigned integer pixels while the
 * output images has double precision floating-point pixels.
 */
extern void tao_pixels_preprocess_affine_u32_to_dbl(
    double*         restrict dat,
    long                     width,
    long                     height,
    const double*   restrict a,
    const double*   restrict b,
    const uint32_t* restrict raw,
    long                     stride);

//-----------------------------------------------------------------------------
// APPLY FULL PRE-PROCESSING

/**
 * @brief Apply affine correction to raw pixels and compute weights.
 *
 * This function applies an affine pixel correction and compute the
 * corresponding weights.  Output image has single precision floating-point
 * pixels.  Input image has 8-bit unsigned integer pixels.
 *
 * Affine pixel correction and corresponding weights are expressed by the
 * following pseudo-code:
 *
 *     dat[i] = (raw[i] - b[i])*a[i]
 *     wgt[i] = q[i]/(max(dat[i],0) + r[i])
 *
 * @param dat     Output array of pre-processed pixels.
 * @param wgt     Output array of weights.
 * @param width   Number of pixels per line of the image.
 * @param height  Number of lines of pixels in the image.
 * @param a       Pixelwise linear correction.
 * @param b       Pixelwise bias correction
 * @param q       Pixelwise numerator value in the weight expression.
 * @param r       Pixelwise denominator offset in the weight expression.
 * @param raw     Input buffer of raw pixels.
 * @param stride  Number of bytes between successive lines in input image
 *                buffer @a raw.
 */
extern void tao_pixels_preprocess_full_u8_to_flt(
    float*          restrict dat,
    float*          restrict wgt,
    long                     width,
    long                     height,
    const float*    restrict a,
    const float*    restrict b,
    const float*    restrict q,
    const float*    restrict r,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels and compute weights.
 *
 * This function behaves as tao_pixels_preprocess_full_u8_to_flt() except
 * that the input image has 8-bit unsigned integer pixels while the output
 * images has double precision floating-point pixels.
 */
extern void tao_pixels_preprocess_full_u8_to_dbl(
    double*         restrict dat,
    double*         restrict wgt,
    long                     width,
    long                     height,
    const double*   restrict a,
    const double*   restrict b,
    const double*   restrict q,
    const double*   restrict r,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels and compute weights.
 *
 * This function behaves as tao_pixels_preprocess_full_u8_to_flt() except
 * that the input image has packed 12-bit unsigned integer pixels while the
 * output images has single precision floating-point pixels.
 */
extern void tao_pixels_preprocess_full_p12_to_flt(
    float*          restrict dat,
    float*          restrict wgt,
    long                     width,
    long                     height,
    const float*    restrict a,
    const float*    restrict b,
    const float*    restrict q,
    const float*    restrict r,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels and compute weights.
 *
 * This function behaves as tao_pixels_preprocess_full_u8_to_flt() except
 * that the input image has packed 12-bit unsigned integer pixels while the
 * output images has double precision floating-point pixels.
 */
extern void tao_pixels_preprocess_full_p12_to_dbl(
    double*         restrict dat,
    double*         restrict wgt,
    long                     width,
    long                     height,
    const double*   restrict a,
    const double*   restrict b,
    const double*   restrict q,
    const double*   restrict r,
    const uint8_t*  restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels and compute weights.
 *
 * This function behaves as tao_pixels_preprocess_full_u8_to_flt() except
 * that the input image has 16-bit unsigned integer pixels while the output
 * images has single precision floating-point pixels.
 */
extern void tao_pixels_preprocess_full_u16_to_flt(
    float*          restrict dat,
    float*          restrict wgt,
    long                     width,
    long                     height,
    const float*    restrict a,
    const float*    restrict b,
    const float*    restrict q,
    const float*    restrict r,
    const uint16_t* restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels and compute weights.
 *
 * This function behaves as tao_pixels_preprocess_full_u8_to_flt() except
 * that the input image has 16-bit unsigned integer pixels while the
 * output images has double precision floating-point pixels.
 */
extern void tao_pixels_preprocess_full_u16_to_dbl(
    double*         restrict dat,
    double*         restrict wgt,
    long                     width,
    long                     height,
    const double*   restrict a,
    const double*   restrict b,
    const double*   restrict q,
    const double*   restrict r,
    const uint16_t* restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels and compute weights.
 *
 * This function behaves as tao_pixels_preprocess_full_u8_to_flt() except
 * that the input image has 32-bit unsigned integer pixels while the output
 * images has single precision floating-point pixels.
 */
extern void tao_pixels_preprocess_full_u32_to_flt(
    float*          restrict dat,
    float*          restrict wgt,
    long                     width,
    long                     height,
    const float*    restrict a,
    const float*    restrict b,
    const float*    restrict q,
    const float*    restrict r,
    const uint32_t* restrict raw,
    long                     stride);

/**
 * @brief Apply affine correction to raw pixels and compute weights.
 *
 * This function behaves as tao_pixels_preprocess_full_u8_to_flt() except
 * that the input image has 32-bit unsigned integer pixels while the
 * output images has double precision floating-point pixels.
 */
extern void tao_pixels_preprocess_full_u32_to_dbl(
    double*         restrict dat,
    double*         restrict wgt,
    long                     width,
    long                     height,
    const double*   restrict a,
    const double*   restrict b,
    const double*   restrict q,
    const double*   restrict r,
    const uint32_t* restrict raw,
    long                     stride);

//-----------------------------------------------------------------------------

/**
 * @}
 */

#endif // TAO_PREPROCESSING_H_
