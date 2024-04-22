// tao-shackhartmann.h -
//
// Definitions for Shack-Hartmann wavefront sensors in TAO.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2022, Éric Thiébaut.

#ifndef TAO_SHACK_HARTMANN_H_
#define TAO_SHACK_HARTMANN_H_ 1

#include <tao-basics.h>

TAO_BEGIN_DECLS

/**
 * @defgroup WavefrontSensors  Shack-Hartmann wavefront sensors
 *
 * @brief Definitions for Shack-Hartmann wavefront sensors.
 *
 * @{
 */

/**
 * Possible wavefront sensing algorithms.
 */
typedef enum tao_algorithm {
    TAO_CENTER_OF_GRAVITY = 0,
    TAO_LINEARIZED_MATCHED_FILTER = 1,
} tao_algorithm;

/**
 * Bounding box.
 */
typedef struct tao_bounding_box {
    int16_t xmin;///< Minimum abscissa (inclusive).
    int16_t xmax;///< Maximum abscissa (inclusive).
    int16_t ymin;///< Minimum ordinate (inclusive).
    int16_t ymax;///< Maximum ordinate (inclusive).
} tao_bounding_box;

/**
 * 2-dimensional position.
 */
typedef struct tao_position {
    double x;///< Abscissa.
    double y;///< Ordinate.
} tao_position;

/**
 * Measured 2-dimensional position.
 */
typedef struct tao_measured_position {
    // The two first member must be the same as for a `tao_position` to allow
    // for a cast.
    double   x;///< Measured abscissa.
    double   y;///< Measured ordinate.
    double wxx;///< Precision of `x`.
    double wxy;///< Joint precision of `x` and `y`;
    double wyy;///< Precision of `y`.
} tao_measured_position;

/**
 * Definition of a wavefront sensor sub-image.
 */
typedef struct tao_subimage {
    tao_bounding_box box;///< Bounding box of the sub-image.
    tao_position     ref;///< Reference position in the sub-image.
} tao_subimage;

/**
 * Wavefront sensor elementary data.
 *
 * This structure represents an elementary data in the output data-frames
 * delivered by a Shack-Hartmann wavefront sensor.
 */
typedef struct tao_shackhartmann_data {
    // The first members must be identical to those of the structure
    // `tao_subimage`.
    tao_bounding_box      box;///< Bounding box of the sub-image.
    tao_position          ref;///< Reference position in the sub-image.

    // Memebers not in `tao_subimage` structure.
    tao_measured_position pos;///< Measured position (relative to the
                              ///  reference).
    double              alpha;///< Intensity factor.
    double                eta;///< Quality factor.
} tao_shackhartmann_data;

/**
 * Shack-Hartmann wavefront sensor configuration.
 *
 * This structure represents the fixed size part of a Shack-Hartmann wavefront
 * sensor configuration.  The variable size parts (the layout of the sub-images
 * grid and the list of sub-images definitions) are provided separately.
 */
typedef struct tao_shackhartmann_config {
    double forgetting_factor;
    double   restoring_force;
    double     max_excursion;
    tao_algorithm  algorithm;
    long             dims[2];///< Dimensions of sub-image grid.
    long               nsubs;///< Number of sub-images.
} tao_shackhartmann_config;

/**
 * Check Shack-Hartmann wavefront sensor configuration.
 *
 * @param cfg     The fixed size part of the wavefront sensor configuration.
 *
 * @param inds    The layout of the sub-images.
 *
 * @param subs    The parameters of the sub-images.
 *
 * @param width   The width of the image.
 *
 * @param height  The height of the image.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_shackhartmann_check_config(
    const tao_shackhartmann_config* cfg,
    const long* inds,
    const tao_subimage* subs,
    long width,
    long height);

/**
 * Tune the run-time parameters of a Shack-Hartmann wavefront sensor.
 *
 * This function tunes the run-time parameters of a Shack-Hartmann wavefront
 * sensor.
 *
 * Only the run-time parameters of the source configuration can be different
 * from those of the destination configuration.
 *
 * @param dst     The destination configuration.
 *
 * @param src     The source configuration.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_shackhartmann_tune_config(
    tao_shackhartmann_config* dst,
    const tao_shackhartmann_config* src);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_SHACK_HARTMANN_H_
