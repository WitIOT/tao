// tao-test-preprocessing.h -
//
// Encoding of image pre-processing methods.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2020-2021, Éric Thiébaut.

#ifndef TAO_TEST_PREPROCESSING_H_
#define TAO_TEST_PREPROCESSING_H_ 1

/**
 * @defgroup PreprocessingTests Pre-processing tests
 *
 * @ingroup Cameras
 *
 * @brief Functions for testing pre-processing of raw images.
 *
 * @{
 *
 * Preprocessing of acquired images consists in applying an affine correction
 * to the raw pixel values and estimating the precision (inverse of variance)
 * of the resulting pixel value.  The affine correction is to compensate for a
 * non-uniform bias and sensitivity of the camera pixels.
 *
 * Preprocessing is applied to all pixels so it is important to use very
 * efficient code for that.  In particular the code should be organized so that
 * as to best exploit loop vectorization by the compiler.  This is driven by
 * two different settings: (1) the splitting of the preprocessing stages (e.g.,
 * compute pixel value and corresponding weight at the same time or separately
 * for groups of pixels, preprocess the whole image, or proceed row-by-row,
 * etc.) and (2) the formulae used to apply the affine correction and compute
 * the weights.
 *
 * Finally the preprocessing code must be specialized for different raw pixel
 * types and different floating-point types for the result.  As the code
 * remains very similar, meta-programming is used by TAO to help compiling
 * optimized variants of preprocessing functions.
 *
 * All the relevant code is provided by the header file
 * `tao-test-preprocessing.h` which should be included after having defined a
 * few pre-processor macros:
 *
 * - `PREPROC_SCOPE` can be defined to specify the scope of the encoded
 *   function, `extern` is assumed by default.
 *
 * - `PREPROC_FUNC` is the name of the preprocessing function to encode.
 *
 * - `PREPROC_FLOAT` is the floating-point pixel type of resulting data and
 *   weights.
 *
 * - `PREPROC_PIXEL` is the raw pixel type of the acquirred image.
 *
 * - if defined, `PREPROC_PACKED` is the number of packed bits encoding a pixel
 *   in the raw image.
 *
 * - `PREPROC_VARIANT` specifies the formulae to apply the affine correction
 *   and compute the weights and the splitting of the pre-processing stages.
 *   This macro has an integer value given by `i + 10*j` where `i` is in the
 *   range `1:4` and specifies the preprocessing formulae while `j` is in the
 *   range `1:7` and specify how the pixels of the image are grouped for
 *   pre-processing.
 *
 *   The least significant bit of `i-1` specifies which formula to use for
 *   computing the pixel weight `wgt` given the pixel value `dat` and the
 *   preprocessing coefficients `q` and `r`:
 *
 *   - If `i=1` or `i=3`, the pixel weight `wgt` is computed as:
 *     `wgt = q/(max(dat,0) + r)`.
 *
 *   - If `i=2` or `i=4`, the pixel weight `wgt` is computed as:
 *     `wgt = q/max(r + dat, r)`.
 *
 *   This 2 formulae are equivalent (in the absence of NaN).
 *
 *   The second least significant bit of `i-1` specifies which formula to use
 *   for computing the pixel data `dat` by applying the affine pixel correction
 *   to the raw pixel value `raw` given the preprocessing coefficients `a` and
 *   `b`:
 *
 *   - If `i=1` or `i=2`, the pixel data is computed as:
 *     `dat = (raw - b)*a`.
 *
 *   - If `i=3` or `i=4`, the pixel data is computed as:
 *     `dat = raw*a + b`.
 *
 *   Note that these 2 formulae are not equivalent, the second one is intended
 *   to exploit *fused multiply-add* (FMA) instructions of the processor.
 *
 *   The possible values of `j` are:
 *
 *   - If `j=1`, apply all operations to each pixel in turn.
 *
 *   - If `j=2`, convert and apply correction to a row of pixels, then compute
 *     weights for this row of pixels.
 *
 *   - If `j=3`, convert a row of pixels, then apply correction and compute
 *     weights for this row of pixels.
 *
 *   - If `j=4`, convert a row of pixels, then apply correction for this row of
 *     pixels, finally compute weights for this row of pixels.
 *
 *   - If `j=5`, convert and apply correction to the full image, then compute
 *     the weights for the image.
 *
 *   - If `j=6`, convert the full image to floating-point, then apply the
 *     correction and compute the weights for the image.
 *
 *   - If `j=7`, convert the full image to floating-point, then apply the
 *     correction to the image and finally compute the weights for the image.
 *
 * According to benchmark tests, the recommended variant is `22`.
 *
 * The header file `tao-test-preprocessing.h` may be included several times to
 * encode different versions of the pre-processing function with different
 * pixel types, different methods, etc.  The above macros (except
 * `PREPROC_SCOPE`) are automatically undefined by `tao-test-preprocessing.h`.
 *
 * The encoded preprocessing function has the following prototype:
 *
 * ~~~~~{.c}
 * PREPROC_SCOPE void
 * PREPROC_FUNC(long width, long height, long stride,
 *              PREPROC_FLOAT* restrict wgt,
 *              PREPROC_FLOAT* restrict dat,
 *              PREPROC_PIXEL const* restrict img,
 *              PREPROC_FLOAT const* restrict a,
 *              PREPROC_FLOAT const* restrict b,
 *              PREPROC_FLOAT const* restrict q,
 *              PREPROC_FLOAT const* restrict r);
 * ~~~~~
 *
 * where `width` and `height` are the number of pixels along the wo image
 * dimensions, `stride` is the number of xxx per row of the input raw image
 * (all other arrays are assumed to have no padding betweeen rows).
 *
 * Example:
 *
 * ~~~~~{.c}
 * #define PREPROC_SCOPE   static      // to encode static functions
 *
 * #define PREPROC_PIXEL   uint8_t     // type of raw pixel
 * #define PREPROC_FLOAT   float       // type of resulting data and weights
 * #define PREPROC_FUNC    proc_u8_flt // function name
 * #define PREPROC_VARIANT 22          // pre-processing method variant
 * #include "tao-test-preprocessing.h" // include header to encode function
 *
 * #define PREPROC_PIXEL   uint8_t     // type of raw pixel
 * #define PREPROC_FLOAT   double      // type of resulting data and weights
 * #define PREPROC_FUNC    proc_u8_dbl // function name
 * #define PREPROC_VARIANT 22          // pre-processing method variant
 * #include "tao-test-preprocessing.h" // include header to encode function
 * ~~~~~
 */

//-----------------------------------------------------------------------------
// MIN, MAX, NONNEGATIVE

#define _MIN_2(x,y) ((x) < (y) ? (x) : (y))
#define _MAX_2(x,y) ((x) > (y) ? (x) : (y))

/**
 * @def min(x, y)
 *
 * @brief Get the smaller of two values.
 *
 * The call `min(x,y)` yields the smallest value between `x` and `y`.
 * NaN are ignored, `x` and `y` should be floating-point values.
 */
#define min(x, y)                               \
    _Generic((x) + (y),                         \
             float:  min2_flt,                  \
             double: min2_dbl)((x), (y))

/**
 * @def max(x, y)
 *
 * @brief Get the greater of two values.
 *
 * The call `max(x,y)` yields the greater of `x` and `y`.  NaN are
 * ignored, `x` and `y` should be floating-point values.
 */
#define max(x, y)                               \
    _Generic((x) + (y),                         \
             float:  max2_flt,                  \
             double: max2_dbl)((x), (y))

static inline float min2_flt(float x, float y) {
    return _MIN_2(x, y);
}

static inline double min2_dbl(double x, double y) {
    return _MIN_2(x, y);
}

static inline float max2_flt(float x, float y) {
    return _MAX_2(x, y);
}

static inline double max2_dbl(double x, double y) {
    return _MAX_2(x, y);
}

/**
 * @def nonnegative(x)
 *
 * @brief Get a nonnegative value.
 *
 * The call `nonnegative(x)` yields the greater of `x` and zero.  NaN
 * are ignored and `x` should be a floating-point value.
 */
#define nonnegative(x)                          \
    _Generic((x),                               \
             float:  nonnegative_flt,           \
             double: nonnegative_dbl)(x)

static inline float nonnegative_flt(float x) {
    float const zero = 0.0f;
    return _MAX_2(x, zero);
}

static inline float nonnegative_dbl(double x) {
    double const zero = 0.0;
    return _MAX_2(x, zero);
}

//-----------------------------------------------------------------------------
// AFFINE PIXEL CORRECTION

/**
 * @def calc_data_std(x, a, b)
 *
 * @brief Apply affine correction (standard version).
 *
 * The call `calc_data_std(x,a,b)` yields `(x - b)*a`.
 */
#define calc_data_std(x, a, b)                          \
    _Generic(((x) - (b))*(a),                           \
             float:  calc_data_std_flt,                 \
             double: calc_data_std_dbl)((x), (a), (b))

/**
 * @def calc_data_fma(x, a, b)
 *
 * @brief Apply affine correction (FMA version).
 *
 * The call `calc_data_fma(x,a,b)` yields `x*a + b`.  This
 * version is intended to exploit *fused multiply-add* (FMA)
 * instructions of the processor.
 */
#define calc_data_fma(x, a, b)                          \
    _Generic((x)*(a) + (b),                             \
             float:  calc_data_fma_flt,                 \
             double: calc_data_fma_dbl)((x), (a), (b))

#define _CALC_DATA_STD(x, a, b) (((x) - (b))*(a))

#define _CALC_DATA_FMA(x, a, b) ((x)*(a) + (b))

static inline float calc_data_std_flt(float x, float a, float b) {
    return _CALC_DATA_STD(x, a, b);
}

static inline double calc_data_std_dbl(double x, double a, double b) {
    return _CALC_DATA_STD(x, a, b);
}

static inline float calc_data_fma_flt(float x, float a, float b) {
    return _CALC_DATA_FMA(x, a, b);
}

static inline double calc_data_fma_dbl(double x, double a, double b) {
    return _CALC_DATA_FMA(x, a, b);
}

//-----------------------------------------------------------------------------
// COMPUTATION OF WEIGHTS

/**
 * @def calc_weight_std(v, q, r)
 *
 * @brief Apply affine correction (standard version).
 *
 * The call `calc_weight_std(v,q,r)` yields `q/(max(v,0) + r)`.
 */
#define calc_weight_std(v, q, r)                                \
    _Generic((v) + (q) + (r),                                   \
             float:  calc_weight_std_flt,                       \
             double: calc_weight_std_dbl)((v), (q), (r))

static inline float calc_weight_std_flt(float v, float q, float r) {
    float const zero = 0;
    return q/(max(v, zero) + r);
}

static inline double calc_weight_std_dbl(double v, double q, double r) {
    double const zero = 0;
    return q/(max(v, zero) + r);
}

/**
 * @def calc_weight_alt(v, q, r)
 *
 * @brief Apply affine correction (alternative version).
 *
 * The call `calc_weight_alt(v,q,r)` yields `q/max(r + v, r)`.
 */
#define calc_weight_alt(v, q, r)                                \
    _Generic((v) + (q) + (r),                                   \
             float:  calc_weight_alt_flt,                       \
             double: calc_weight_alt_dbl)((v), (q), (r))

static inline float calc_weight_alt_flt(float v, float q, float r) {
    return q/max(r + v, r);
}

static inline double calc_weight_alt_dbl(double v, double q, double r) {
    return q/max(r + v, r);
}

static inline uint16_t unpack_low_p12(uint16_t b0, uint16_t b1)
{
    return (b0 << 4) | (b1 & (uint16_t)0x000F);
}

static inline uint16_t unpack_high_p12(uint16_t b1, uint16_t b2)
{
    return (b2 << 4) | (b1 >> 4);
}

/**
 * @}
 */

#define _PREPROC_ROW(img, y, stride) \
    ((PREPROC_PIXEL const*)((uint8_t*)(img) + (y)*(stride)))

#endif // TAO_TEST_PREPROCESSING_H_

//-----------------------------------------------------------------------------

#ifdef PREPROC_FUNC

#ifndef PREPROC_SCOPE
#  define PREPROC_SCOPE extern
#endif

#ifndef PREPROC_FLOAT
#  error PREPROC_FLOAT must be defined
#endif
#ifndef PREPROC_PIXEL
#  error PREPROC_PIXEL must be defined
#endif

#ifndef PREPROC_VARIANT
#  error PREPROC_VARIANT must be defined
#else
#  if (((PREPROC_VARIANT)%10 - 1) & 1) == 0
#    define CALC_WEIGHT calc_weight_std
#  else
#    define CALC_WEIGHT calc_weight_alt
#  endif
#  if (((PREPROC_VARIANT)%10 - 1) & 2) == 0
#    define CALC_DATA calc_data_std
#  else
#    define CALC_DATA calc_data_fma
#  endif
#  define _PREPROC_SPLIT ((PREPROC_VARIANT)/10)
#endif // PREPROC_VARIANT

// The following macros mimic 2-D array indexing.  In practice with recent
// optimizers and CPUs, this adds no overheads compared to 1-D indexing of the
// C arrays.  These macros however make the code easier to read and write.  For
// now, the macros are not protected by a prefix such as `_PREPROC_`, again to
// keep the code easy to read and write.
#define IMG(x,y) ((PREPROC_FLOAT)img[(x) + stride*(y)])
#define DAT(x,y)                       dat[(x) + width*(y)]
#define WGT(x,y)                       wgt[(x) + width*(y)]
#define A(x,y)                           a[(x) + width*(y)]
#define B(x,y)                           b[(x) + width*(y)]
#define Q(x,y)                           q[(x) + width*(y)]
#define R(x,y)                           r[(x) + width*(y)]

#if _PREPROC_SPLIT == 1
// Version 1.  Apply all operations to each pixel in turn.
PREPROC_SCOPE void PREPROC_FUNC(
    long width,
    long height,
    long stride,
    PREPROC_FLOAT*       restrict wgt,
    PREPROC_FLOAT*       restrict dat,
    PREPROC_PIXEL const* restrict img,
    PREPROC_FLOAT const* restrict a,
    PREPROC_FLOAT const* restrict b,
    PREPROC_FLOAT const* restrict q,
    PREPROC_FLOAT const* restrict r)
{
    for (long y = 0; y < height; ++y) {
        for (long x = 0; x < width; ++x) {
            DAT(x,y) = CALC_DATA(IMG(x,y), A(x,y), B(x,y));
            WGT(x,y) = CALC_WEIGHT(DAT(x,y), Q(x,y), R(x,y));
        }
    }
}
#elif _PREPROC_SPLIT == 2
// Version 2.  Convert and apply correction to a row of pixels, then
//             compute weights for this row of pixels.
PREPROC_SCOPE void PREPROC_FUNC(
    long width,
    long height,
    long stride,
    PREPROC_FLOAT*       restrict wgt,
    PREPROC_FLOAT*       restrict dat,
    PREPROC_PIXEL const* restrict img,
    PREPROC_FLOAT const* restrict a,
    PREPROC_FLOAT const* restrict b,
    PREPROC_FLOAT const* restrict q,
    PREPROC_FLOAT const* restrict r)
{
    for (long y = 0; y < height; ++y) {
        for (long x = 0; x < width; ++x) {
            DAT(x,y) = CALC_DATA(IMG(x,y), A(x,y), B(x,y));
        }
        for (long x = 0; x < width; ++x) {
            WGT(x,y) = CALC_WEIGHT(DAT(x,y), Q(x,y), R(x,y));
        }
    }
}
#elif _PREPROC_SPLIT == 3
// Version 3.  Convert a row of pixels, then apply correction and
//             compute weights for this row of pixels.
PREPROC_SCOPE void PREPROC_FUNC(
    long width,
    long height,
    long stride,
    PREPROC_FLOAT*       restrict wgt,
    PREPROC_FLOAT*       restrict dat,
    PREPROC_PIXEL const* restrict img,
    PREPROC_FLOAT const* restrict a,
    PREPROC_FLOAT const* restrict b,
    PREPROC_FLOAT const* restrict q,
    PREPROC_FLOAT const* restrict r)
{
    for (long y = 0; y < height; ++y) {
        for (long x = 0; x < width; ++x) {
            DAT(x,y) = IMG(x,y);
        }
        for (long x = 0; x < width; ++x) {
            PREPROC_FLOAT val = CALC_DATA(DAT(x,y), A(x,y), B(x,y));
            DAT(x,y) = val;
            WGT(x,y) = CALC_WEIGHT(val, Q(x,y), R(x,y));
        }
    }
}
#elif _PREPROC_SPLIT == 4
// Version 4.  Convert a row of pixels, then apply correction for this
//             row of pixels, finally compute weights for this row of
//             pixels.
PREPROC_SCOPE void PREPROC_FUNC(
    long width,
    long height,
    long stride,
    PREPROC_FLOAT*       restrict wgt,
    PREPROC_FLOAT*       restrict dat,
    PREPROC_PIXEL const* restrict img,
    PREPROC_FLOAT const* restrict a,
    PREPROC_FLOAT const* restrict b,
    PREPROC_FLOAT const* restrict q,
    PREPROC_FLOAT const* restrict r)
{
    for (long y = 0; y < height; ++y) {
        for (long x = 0; x < width; ++x) {
            DAT(x,y) = IMG(x,y);
        }
        for (long x = 0; x < width; ++x) {
            DAT(x,y) = CALC_DATA(DAT(x,y), A(x,y), B(x,y));
        }
        for (long x = 0; x < width; ++x) {
            WGT(x,y) = CALC_WEIGHT(DAT(x,y), Q(x,y), R(x,y));
        }
    }
}
#elif _PREPROC_SPLIT == 5
// Version 5.  Convert and apply correction to the full image, then
//             compute the weights for the image.
PREPROC_SCOPE void PREPROC_FUNC(
    long width,
    long height,
    long stride,
    PREPROC_FLOAT*       restrict wgt,
    PREPROC_FLOAT*       restrict dat,
    PREPROC_PIXEL const* restrict img,
    PREPROC_FLOAT const* restrict a,
    PREPROC_FLOAT const* restrict b,
    PREPROC_FLOAT const* restrict q,
    PREPROC_FLOAT const* restrict r)
{
    for (long y = 0; y < height; ++y) {
        for (long x = 0; x < width; ++x) {
            DAT(x,y) = CALC_DATA(IMG(x,y), A(x,y), B(x,y));
        }
    }
    for (long y = 0; y < height; ++y) {
        for (long x = 0; x < width; ++x) {
            WGT(x,y) = CALC_WEIGHT(DAT(x,y), Q(x,y), R(x,y));
        }
    }
}
#elif _PREPROC_SPLIT == 6
// Version 6.  Convert the full image to floating-point, then apply
//             the correction and compute the weights for the image.
PREPROC_SCOPE void PREPROC_FUNC(
    long width,
    long height,
    long stride,
    PREPROC_FLOAT*       restrict wgt,
    PREPROC_FLOAT*       restrict dat,
    PREPROC_PIXEL const* restrict img,
    PREPROC_FLOAT const* restrict a,
    PREPROC_FLOAT const* restrict b,
    PREPROC_FLOAT const* restrict q,
    PREPROC_FLOAT const* restrict r)
{
    for (long y = 0; y < height; ++y) {
        for (long x = 0; x < width; ++x) {
            DAT(x,y) = IMG(x,y);
        }
    }
    for (long y = 0; y < height; ++y) {
        for (long x = 0; x < width; ++x) {
            PREPROC_FLOAT val = CALC_DATA(DAT(x,y), A(x,y), B(x,y));
            DAT(x,y) = val;
            WGT(x,y) = CALC_WEIGHT(val, Q(x,y), R(x,y));
        }
    }
}
#elif _PREPROC_SPLIT == 7
// Version 7.  Convert the full image to floating-point, then apply
//             the correction to the image and finally compute the
//             weights for the image.
PREPROC_SCOPE void PREPROC_FUNC(
    long width,
    long height,
    long stride,
    PREPROC_FLOAT*       restrict wgt,
    PREPROC_FLOAT*       restrict dat,
    PREPROC_PIXEL const* restrict img,
    PREPROC_FLOAT const* restrict a,
    PREPROC_FLOAT const* restrict b,
    PREPROC_FLOAT const* restrict q,
    PREPROC_FLOAT const* restrict r)
{
    for (long y = 0; y < height; ++y) {
        for (long x = 0; x < width; ++x) {
            DAT(x,y) = IMG(x,y);
        }
    }
    for (long y = 0; y < height; ++y) {
        for (long x = 0; x < width; ++x) {
            DAT(x,y) = CALC_DATA(DAT(x,y), A(x,y), B(x,y));
        }
    }
    for (long y = 0; y < height; ++y) {
        for (long x = 0; x < width; ++x) {
            WGT(x,y) = CALC_WEIGHT(DAT(x,y), Q(x,y), R(x,y));
        }
    }
}
#else
#  error invalid splitting for pre-processing method
#endif // _PREPROC_SPLIT

#undef IMG
#undef DAT
#undef WGT
#undef A
#undef B
#undef Q
#undef R
#undef CALC_DATA
#undef CALC_WEIGHT
#undef _PREPROC_SPLIT
#undef PREPROC_FUNC
#undef PREPROC_FLOAT
#undef PREPROC_PIXEL
#undef PREPROC_VARIANT
#undef PREPROC_PACKED

#endif // PREPROC_FUNC
