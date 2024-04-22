// tao-encodings.h -
//
// Definitions for dynamic encodings in TAO.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2019-2022, Éric Thiébaut.

#ifndef TAO_ENCODINGS_H_
#define TAO_ENCODINGS_H_ 1

#include <tao-basics.h>
#include <tao-macros.h>

#include <stdint.h>

TAO_BEGIN_DECLS

/**
 * @defgroup Encodings  Encodings
 *
 * @ingroup Arrays
 *
 * @brief Definitions of element types and pixel encodings.
 *
 * @{
 */

union tao_byte_order_mark {
    uint8_t bytes[4];
    uint32_t value;
};

/**
 * @def TAO_NATIVE_ENDIAN_BOM
 *
 * Yield the value of the byte order mark (BOM) of the machine.
 */
#define TAO_NATIVE_ENDIAN_BOM \
    (((union tao_byte_order_mark){.bytes = {1,2,3,4}}).value)

/**
 * @def TAO_BIG_ENDIAN_BOM
 *
 * Byte order mark (BOM) value for big endian byte order.
 */
#define TAO_BIG_ENDIAN_BOM (0x01020304U)

/**
 * @def TAO_LITTLE_ENDIAN_BOM
 *
 * Byte order mark (BOM) value for little endian byte order.
 */
#define TAO_LITTLE_ENDIAN_BOM (0x04030201U)

/**
 * @def TAO_IS_BIG_ENDIAN
 *
 * Yield whether native byte order is big endian.
 */
#define TAO_IS_BIG_ENDIAN (TAO_NATIVE_ENDIAN_BOM == TAO_BIG_ENDIAN_BOM)

/**
 * @def TAO_IS_LITTLE_ENDIAN
 *
 * Yield whether native byte order is little endian.
 */
#define TAO_IS_LITTLE_ENDIAN (TAO_NATIVE_ENDIAN_BOM == TAO_LITTLE_ENDIAN_BOM)

/**
 * Identifier of the type of the elements in an array.
 */
typedef enum tao_eltype {
    TAO_INT8    =   1, ///< Signed 8-bit integer
    TAO_UINT8   =   2, ///< Unsigned 8-bit integer
    TAO_INT16   =   3, ///< Signed 16-bit integer
    TAO_UINT16  =   4, ///< Unsigned 16-bit integer
    TAO_INT32   =   5, ///< Signed 32-bit integer
    TAO_UINT32  =   6, ///< Unsigned 32-bit integer
    TAO_INT64   =   7, ///< Signed 64-bit integer
    TAO_UINT64  =   8, ///< Unsigned 64-bit integer
    TAO_FLOAT   =   9, ///< Single precision floating-point
    TAO_DOUBLE  =  10  ///< Double precision floating-point
} tao_eltype;

/**
 * Get the size of an array element given its type.
 *
 * @param eltype Type of the elements of an array.
 *
 * @return A strictly positive number of bytes if @b eltype is valid; `0` is
 *         @b eltype is not valid.
 *
 * @see tao_eltype.
 */
extern size_t tao_size_of_eltype(
    tao_eltype eltype);

/**
 * Get the name of an element type.
 *
 * @param eltype Type of the elements of an array.
 *
 * @return A name, "unknown_type" if @b eltype is not valid.
 *
 * @see tao_eltype.
 */
extern const char* tao_name_of_eltype(
    tao_eltype eltype);

/**
 * Get a description of an element type.
 *
 * @param eltype Type of the elements of an array.
 *
 * @return A string, `"unknown type"` if @b eltype is not valid.
 *
 * @see tao_eltype.
 */
extern const char* tao_description_of_eltype(
    tao_eltype eltype);

/**
 * Pixel encoding is stored in 32-bit unsigned integer.
 *
 * The pixel encoding is a bitwise combination of various information stored in
 * a 32-bit unsigned integer:
 *
 * | Bits    | Description       |
 * | :-----: | :---------------- |
 * | 1-8     | Bits per pixel    |
 * | 9-16    | Bits per packet   |
 * | 17-24   | Color type        |
 * | 25-32   | Flags             |
 */
typedef uint32_t tao_encoding;

/**
 * @def TAO_ENCODING_2_(col,pxl)
 *
 * @brief Macro to define a simple pixel encoding.
 *
 * @param col    Pixel color type.
 * @param pxl    Bits per pixel.
 */
#define TAO_ENCODING_2_(col,pxl) TAO_ENCODING_3_(col,pxl,pxl)

/**
 * @def TAO_ENCODING_3_(col,pxl,pkt)
 *
 * @brief Macro to define a grouped/packed pixel encoding.
 *
 * @param col    Pixel color type.
 * @param pxl    Bits per pixel.
 * @param pkt    Bits per packet.
 */
#define TAO_ENCODING_3_(col,pxl,pkt) ((((tao_encoding)(col)) << 16) | \
                                      (((tao_encoding)(pkt)) <<  8) | \
                                      ( (tao_encoding)(pxl)))

/**
 * @def TAO_ENCODING_4_(col,pxl,pkt,flg)
 *
 * @brief Macro to define a general pixel encoding.
 *
 * @param col    Pixel color type.
 * @param pxl    Bits per pixel.
 * @param pkt    Bits per packet.
 * @param flg    Flags.
 */
#define TAO_ENCODING_4_(col,pxl,pkt,flg) \
    ((((tao_encoding)(flg)) << 24) | TAO_ENCODING_3_(col,pxl,pkt))

/**
 * @def TAO_ENCODING_FLAGS_MSB_PAD
 *
 * @brief Encoding flag indicating zero padded upper bits.
 */
#define TAO_ENCODING_FLAGS_MSB_PAD  ((tao_encoding)0 << 0)

/**
 * @def TAO_ENCODING_FLAGS_LSB_PAD
 *
 * @brief Encoding flag indicating zero padded lower bits.
 */
#define TAO_ENCODING_FLAGS_LSB_PAD  ((tao_encoding)1 << 0)

/**
 * @def TAO_ENCODING_FLAGS_CODED
 *
 * @brief Encoding flag indicating Andor "Coded" format.
 */
#define TAO_ENCODING_FLAGS_CODED    ((tao_encoding)1 << 1)

/**
 * @def TAO_ENCODING_FLAGS_PARALLEL
 *
 * @brief Encoding flag indicating Andor "Parallel" format.
 */
#define TAO_ENCODING_FLAGS_PARALLEL ((tao_encoding)1 << 2)

/**
 * @def TAO_ENCODING_MASK
 *
 * @brief Encoding mask to select the 8 least significant bits.
 */
#define TAO_ENCODING_MASK     ((tao_encoding)255)

/**
 * @def TAO_ENCODING_UNKNOWN
 *
 * @brief Constant representing unknown encoding.
 */
#define TAO_ENCODING_UNKNOWN  ((tao_encoding)0)

/**
 * @def TAO_ENCODING_BITS_PER_PIXEL(enc)
 *
 * @brief Macro to extract the number of bits per pixel.
 *
 * @param enc    Pixel encoding.
 */
#define TAO_ENCODING_BITS_PER_PIXEL(enc) \
    ((tao_encoding)(enc) & TAO_ENCODING_MASK)

/**
 * @def TAO_ENCODING_BITS_PER_PACKET(enc)
 *
 * @brief Macro to extract the number of bits per packet.
 *
 * @param enc    Pixel encoding.
 */
#define TAO_ENCODING_BITS_PER_PACKET(enc) \
    (((tao_encoding)(enc) >> 8) & TAO_ENCODING_MASK)

/**
 * @def TAO_ENCODING_COLORANT(enc)
 *
 * @brief Macro to extract the color type of a pixel encoding.
 *
 * @param enc    Pixel encoding.
 */
#define TAO_ENCODING_COLORANT(enc) \
    (((tao_encoding)(enc) >> 16) & TAO_ENCODING_MASK)

/**
 * @def TAO_ENCODING_FLAGS(enc)
 *
 * @brief Macro to extract the flags of a pixel encoding.
 *
 * @param enc    Pixel encoding.
 */
#define TAO_ENCODING_FLAGS(enc) \
    (((tao_encoding)(enc) >> 24) & TAO_ENCODING_MASK)

// Colorants.
#define TAO_COLORANT_RAW          1
#define TAO_COLORANT_MONO         2
#define TAO_COLORANT_RGB          3
#define TAO_COLORANT_BGR          4
#define TAO_COLORANT_ARGB         5
#define TAO_COLORANT_RGBA         6
#define TAO_COLORANT_ABGR         7
#define TAO_COLORANT_BGRA         8
#define TAO_COLORANT_BAYER_RGGB   9
#define TAO_COLORANT_BAYER_GRBG  10
#define TAO_COLORANT_BAYER_GBRG  11
#define TAO_COLORANT_BAYER_BGGR  12
#define TAO_COLORANT_YUV444      13
#define TAO_COLORANT_YUV422      14
#define TAO_COLORANT_YUV411      15
#define TAO_COLORANT_YUV420P     16
#define TAO_COLORANT_YUV420SP    17
#define TAO_COLORANT_SIGNED      18
#define TAO_COLORANT_FLOAT       19
#define TAO_COLORANT_UNSIGNED    TAO_COLORANT_MONO

#define TAO_ENCODING_RAW(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_RAW, pxl)

#define TAO_ENCODING_RAW_PKT(pxl, pkt) \
    TAO_ENCODING_3_(TAO_COLORANT_RAW, pxl, pkt)

#define TAO_ENCODING_MONO(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_MONO, pxl)

#define TAO_ENCODING_MONO_PKT(pxl, pkt) \
    TAO_ENCODING_3_(TAO_COLORANT_MONO, pxl, pkt)

#define TAO_ENCODING_RGB(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_RGB, pxl)

#define TAO_ENCODING_RGB_PKT(pxl, pkt) \
    TAO_ENCODING_3_(TAO_COLORANT_RGB, pxl, pkt)

#define TAO_ENCODING_BGR(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_BGR, pxl)

#define TAO_ENCODING_BGR_PKT(pxl, pkt) \
    TAO_ENCODING_3_(TAO_COLORANT_BGR, pxl, pkt)

#define TAO_ENCODING_ARGB(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_ARGB, pxl)

#define TAO_ENCODING_ARGB_PKT(pxl, pkt) \
    TAO_ENCODING_3_(TAO_COLORANT_ARGB, pxl, pkt)

#define TAO_ENCODING_RGBA(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_RGBA, pxl)

#define TAO_ENCODING_RGBA_PKT(pxl, pkt) \
    TAO_ENCODING_3_(TAO_COLORANT_RGBA, pxl, pkt)

#define TAO_ENCODING_ABGR(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_ABGR, pxl)

#define TAO_ENCODING_ABGR_PKT(pxl, pkt) \
    TAO_ENCODING_3_(TAO_COLORANT_ABGR, pxl, pkt)

#define TAO_ENCODING_BGRA(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_BGRA, pxl)

#define TAO_ENCODING_BGRA_PKT(pxl, pkt) \
    TAO_ENCODING_3_(TAO_COLORANT_BGRA, pxl, pkt)

#define TAO_ENCODING_BAYER_RGGB(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_BAYER_RGGB, pxl)

#define TAO_ENCODING_BAYER_GRBG(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_BAYER_GRBG, pxl)

#define TAO_ENCODING_BAYER_GBRG(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_BAYER_GBRG, pxl)

#define TAO_ENCODING_BAYER_BGGR(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_BAYER_BGGR, pxl)

#define TAO_ENCODING_YUV444 \
    TAO_ENCODING_2_(TAO_COLORANT_YUV444,24)

#define TAO_ENCODING_YUV422 \
    TAO_ENCODING_3_(TAO_COLORANT_YUV422,16,32)

#define TAO_ENCODING_YUV411 \
    TAO_ENCODING_3_(TAO_COLORANT_YUV411,12,48)

#define TAO_ENCODING_YUV420P \
    TAO_ENCODING_3_(TAO_COLORANT_YUV420P,12,48)

#define TAO_ENCODING_YUV420SP \
    TAO_ENCODING_3_(TAO_COLORANT_YUV420SP,12,48)

#define TAO_ENCODING_FLOAT(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_FLOAT, pxl)

#define TAO_ENCODING_SIGNED(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_SIGNED, pxl)

#define TAO_ENCODING_UNSIGNED(pxl) \
    TAO_ENCODING_2_(TAO_COLORANT_UNSIGNED, pxl)

/**
 * @def TAO_ENCODING_ANDOR_MONO8
 *
 * @brief Andor Mono8 encoding.
 *
 * Andor Mono8 encoding is monochrome with 8 bits per pixel, stored as 8-bit
 * little-endian.
 */
#define TAO_ENCODING_ANDOR_MONO8 TAO_ENCODING_MONO(8)

/**
 * @def TAO_ENCODING_ANDOR_MONO12
 *
 * @brief Andor Mono12 encoding.
 *
 * Andor Mono12 encoding is monochrome with 12 bits per pixel, stored as 16-bit
 * little-endian with zero padded upper bits.
 */
#define TAO_ENCODING_ANDOR_MONO12 \
    TAO_ENCODING_4_(TAO_COLORANT_MONO, 12, 16, TAO_ENCODING_FLAGS_MSB_PAD)

/**
 * @def TAO_ENCODING_ANDOR_MONO12PACKED
 *
 * @brief Andor Mono12Packed encoding.
 *
 * Andor Mono12Packed encoding is monochrome with 12 bits per pixel, stored by
 * packing two adjacent pixels into three bytes.
 */
#define TAO_ENCODING_ANDOR_MONO12PACKED TAO_ENCODING_MONO_PKT(12, 24)

/**
 * @def TAO_ENCODING_ANDOR_MONO12CODED
 *
 * @brief Andor Mono12Coded encoding.
 *
 * Andor Mono12Coded encoding is assumed to be monochrome with 12 bits per
 * pixel, stored as 16-bit little-endian with zero padded upper bits.
 */
#define TAO_ENCODING_ANDOR_MONO12CODED          \
    TAO_ENCODING_4_(TAO_COLORANT_MONO, 12, 16,  \
                    TAO_ENCODING_FLAGS_CODED |  \
                    TAO_ENCODING_FLAGS_MSB_PAD)

/**
 * @def TAO_ENCODING_ANDOR_MONO12CODEDPACKED
 *
 * @brief Andor Mono12CodedPacked encoding.
 *
 * Andor Mono12CodedPacked encoding is assumed to be monochrome with 12 bits
 * per pixel, stored by packing 2 adjacent pixels into 3 bytes.
 */
#define TAO_ENCODING_ANDOR_MONO12CODEDPACKED \
    TAO_ENCODING_4_(TAO_COLORANT_MONO, 12, 24, TAO_ENCODING_FLAGS_CODED)

/**
 * @def TAO_ENCODING_ANDOR_MONO16
 *
 * @brief Andor Mono16 encoding.
 *
 * Andor Mono16 encoding is monochrome with 16 bits per pixel, stored as 16-bit
 * little-endian.
 */
#define TAO_ENCODING_ANDOR_MONO16 TAO_ENCODING_MONO(16)

/**
 * @def TAO_ENCODING_ANDOR_MONO32
 *
 * @brief Andor Mono32 encoding.
 *
 * Andor Mono32 encoding is monochrome with 32 bits per pixel, stored as 32-bit
 * little-endian.
 */
#define TAO_ENCODING_ANDOR_MONO32 TAO_ENCODING_MONO(32)

/**
 * @def TAO_ENCODING_ANDOR_RGB8PACKED
 *
 * @brief Andor RGB8Packed encoding.
 *
 * Andor RGB8Packed encoding is assumed to be RGB color with 8 bits per pixel,
 * stored as 24-bit data.
 */
#define TAO_ENCODING_ANDOR_RGB8PACKED TAO_ENCODING_RGB(24)

/**
 * @def TAO_ENCODING_ANDOR_MONO22PARALLEL
 *
 * @brief Andor Mono22Parallel encoding.
 *
 * Andor Mono22Parallel encoding is assumed to be monochrome with 22 bits per
 * pixel, stored as 24-bit little-endian with zero padded upper bits.
 */
#define TAO_ENCODING_ANDOR_MONO22PARALLEL               \
    TAO_ENCODING_4_(TAO_COLORANT_MONO, 22, 24,          \
                    TAO_ENCODING_FLAGS_PARALLEL |       \
                    TAO_ENCODING_FLAGS_MSB_PAD)

/**
 * @def TAO_ENCODING_ANDOR_MONO22PACKEDPARALLEL
 *
 * @brief Andor Mono22PackedParallel encoding.
 *
 * Andor Mono22PackedParallel encoding is assumed to be monochrome with 22 bits
 * per pixel, stored by packing 4 adjacent pixels into 11 bytes.
 */
#define TAO_ENCODING_ANDOR_MONO22PACKEDPARALLEL \
    TAO_ENCODING_4_(TAO_COLORANT_MONO, 22, 88, TAO_ENCODING_FLAGS_PARALLEL)

/**
 * Get pixel encoding matching given element type.
 *
 * @param eltype   Element type.
 *
 * @return Encoding value, @ref TAO_ENCODING_UNKNOWN in case of failure.
 */
extern tao_encoding tao_encoding_of_eltype(
    tao_eltype eltype);

/**
 * Get element type matching given pixel encoding.
 *
 * @param enc   Encoding value.
 *
 * @return Element type, `0` if no match exists.
 */
extern tao_eltype tao_eltype_of_encoding(
    tao_encoding enc);

/**
 * Format encoding into a string.
 *
 * @param str   Destination string, must have at least
 *              `TAO_ENCODING_STRING_SIZE` characters, including the final
 *              null.
 * @param enc   Encoding value.
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_format_encoding(
    char* str,
    tao_encoding enc);

/**
 * Get encoding from string.
 *
 * @param str   Encoding name.
 *
 * @return Encoding value, `TAO_ENCODING_UNKNOWN` in case of failures.
 */
extern tao_encoding tao_parse_encoding(
    const char* str);

#define TAO_ENCODING_STRING_SIZE 32

/**
 * @}
 */

//-----------------------------------------------------------------------------

/**
 * @defgroup CopyConvert Copy/convert regions
 *
 * @ingroup Utilities
 *
 * @brief Copy/convert regions of multi-dimensional arrays.
 *
 * @{
 */

/**
 * Copy/convert regions of multi-dimensional arrays.
 *
 * This function copies (and possibly converts) the elements of a rectangular
 * region between two multi-dimensional arrays.
 *
 * @param dstdata  Address of first element in destination array.
 * @param dsttype  Type of elements in destination array.
 * @param dstdims  Dimensions of destination array.
 * @param dstoffs  Offsets of destination region (can be `NULL` if there are
 *                 no offsets).
 * @param srcdata  Address of first element in source array.
 * @param srctype  Type of elements in source array.
 * @param srcdims  Dimensions of source array.
 * @param srcoffs  Offsets of source region (can be `NULL` if there are
 *                 no offsets).
 * @param lens     Dimensions of region to copy.
 * @param ndims    Number of dimensions (length of @a dstdims, @a dstoffs,
 *                 @a srcdims, @a srcoffs and @a lens).
 *
 * @return @ref TAO_OK on success, @ref TAO_ERROR in case of failure.
 *
 * @see tao_copy_checked_args().
 */
extern tao_status tao_copy(
    void*       restrict dstdata,
    tao_eltype           dsttype,
    const long* restrict dstdims,
    const long* restrict dstoffs,
    const void* restrict srcdata,
    tao_eltype           srctype,
    const long* restrict srcdims,
    const long* restrict srcoffs,
    const long* restrict lens,
    int                  ndims);

/**
 * Copy/convert regions of multi-dimensional arrays.
 *
 * This function is the same as tao_copy() except that no checking of the
 * arguments is performed.
 *
 * @param dstdata  Address of first element in destination array.
 * @param dsttype  Type of elements in destination array.
 * @param dstdims  Dimensions of destination array.
 * @param dstoffs  Offsets of destination region (can be `NULL` if there are
 *                 no offsets).
 * @param srcdata  Address of first element in source array.
 * @param srctype  Type of elements in source array.
 * @param srcdims  Dimensions of source array.
 * @param srcoffs  Offsets of source region (can be `NULL` if there are
 *                 no offsets).
 * @param lens     Dimensions of region to copy.
 * @param ndims    Number of dimensions (length of @a dstdims, @a dstoffs,
 *                 @a srcdims, @a srcoffs and @a lens).
 *
 * @see tao_copy().
 */
extern void tao_copy_checked_args(
    void*       restrict dstdata,
    tao_eltype           dsttype,
    const long* restrict dstdims,
    const long* restrict dstoffs,
    const void* restrict srcdata,
    tao_eltype           srctype,
    const long* restrict srcdims,
    const long* restrict srcoffs,
    const long* restrict lens,
    int                  ndims);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_ENCODINGS_H_
