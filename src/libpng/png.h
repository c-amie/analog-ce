/*********************************
*      PNG Graphics Library      *
*********************************/

/* This file contains code from the PNG reference library, heavily modified
   by Stephen Turner for use in analog. See the file LICENSE in this directory
   for details and copyright statement. */

#ifndef NOGRAPHICS
#ifdef HAVE_GD
#include <png.h>
#else
#ifndef _PNG_H
#define _PNG_H

#include "../zlib/zlib.h"
#include "pngconf.h"
#if defined(PNG_INTERNAL) && defined(PNG_ASSEMBLER_CODE_SUPPORTED)
#  include "pngasmrd.h"
#endif

#define PNG_LIBPNG_VER_STRING "1.0.6"
#define PNG_LIBPNG_VER    10006  /* 1.0.6 */

#if !defined(PNG_NO_EXTERN) || defined(PNG_ALWAYS_EXTERN)
#ifdef PNG_USE_GLOBAL_ARRAYS
PNG_EXPORT_VAR (char) png_libpng_ver[12];
#else
#define png_libpng_ver png_get_header_ver(NULL)
#endif

#ifdef PNG_USE_GLOBAL_ARRAYS
PNG_EXPORT_VAR (int FARDATA) png_pass_start[7];
PNG_EXPORT_VAR (int FARDATA) png_pass_inc[7];
PNG_EXPORT_VAR (int FARDATA) png_pass_ystart[7];
PNG_EXPORT_VAR (int FARDATA) png_pass_yinc[7];
PNG_EXPORT_VAR (int FARDATA) png_pass_mask[7];
PNG_EXPORT_VAR (int FARDATA) png_pass_dsp_mask[7];
#ifdef PNG_HAVE_ASSEMBLER_COMBINE_ROW
extern int FARDATA png_pass_width[7];
#endif
#endif

#endif /* PNG_NO_EXTERN */

typedef struct png_color_struct
{
   png_byte red;
   png_byte green;
   png_byte blue;
} png_color;
typedef png_color FAR * png_colorp;
typedef png_color FAR * FAR * png_colorpp;

typedef struct png_color_16_struct
{
   png_byte index;   
   png_uint_16 red;  
   png_uint_16 green;
   png_uint_16 blue;
   png_uint_16 gray; 
} png_color_16;
typedef png_color_16 FAR * png_color_16p;
typedef png_color_16 FAR * FAR * png_color_16pp;

typedef struct png_color_8_struct
{
   png_byte red;   
   png_byte green;
   png_byte blue;
   png_byte gray;  
   png_byte alpha; 
} png_color_8;
typedef png_color_8 FAR * png_color_8p;
typedef png_color_8 FAR * FAR * png_color_8pp;

#define PNG_TEXT_COMPRESSION_NONE_WR -3
#define PNG_TEXT_COMPRESSION_zTXt_WR -2
#define PNG_TEXT_COMPRESSION_NONE    -1
#define PNG_TEXT_COMPRESSION_zTXt     0
#define PNG_ITXT_COMPRESSION_NONE     1
#define PNG_ITXT_COMPRESSION_zTXt     2
#define PNG_TEXT_COMPRESSION_LAST     3

typedef struct png_time_struct
{
   png_uint_16 year;
   png_byte month;
   png_byte day;
   png_byte hour;
   png_byte minute;
   png_byte second;
} png_time;
typedef png_time FAR * png_timep;
typedef png_time FAR * FAR * png_timepp;

typedef struct png_info_struct
{
   png_uint_32 width;
   png_uint_32 height;
   png_uint_32 valid;
   png_uint_32 rowbytes;
   png_colorp palette;
   png_uint_16 num_palette;
   png_uint_16 num_trans;
   png_byte bit_depth;
   png_byte color_type;
   png_byte compression_type;
   png_byte filter_type;
   png_byte interlace_type;
   png_byte channels;
   png_byte pixel_depth;
   png_byte spare_byte;
   png_byte signature[8];
   png_uint_32 free_me;

#if defined(PNG_tRNS_SUPPORTED) || defined(PNG_READ_EXPAND_SUPPORTED) || \
defined(PNG_READ_BACKGROUND_SUPPORTED)
   png_bytep trans;
   png_color_16 trans_values;
#endif

#if defined(PNG_INFO_IMAGE_SUPPORTED)
   png_bytepp row_pointers;
#endif
} png_info;

typedef png_info FAR * png_infop;
typedef png_info FAR * FAR * png_infopp;

#define PNG_MAX_UINT ((png_uint_32)0x7fffffffL)

#define PNG_COLOR_MASK_PALETTE    1
#define PNG_COLOR_MASK_COLOR      2
#define PNG_COLOR_MASK_ALPHA      4

#define PNG_COLOR_TYPE_GRAY 0
#define PNG_COLOR_TYPE_PALETTE  (PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_PALETTE)
#define PNG_COLOR_TYPE_RGB        (PNG_COLOR_MASK_COLOR)
#define PNG_COLOR_TYPE_RGB_ALPHA  (PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_ALPHA)
#define PNG_COLOR_TYPE_GRAY_ALPHA (PNG_COLOR_MASK_ALPHA)

#define PNG_COMPRESSION_TYPE_BASE 0
#define PNG_COMPRESSION_TYPE_DEFAULT PNG_COMPRESSION_TYPE_BASE

#define PNG_FILTER_TYPE_BASE      0
#define PNG_FILTER_TYPE_DEFAULT   PNG_FILTER_TYPE_BASE

#define PNG_INTERLACE_NONE        0
#define PNG_INTERLACE_ADAM7       1
#define PNG_INTERLACE_LAST        2

#define PNG_INFO_gAMA 0x0001
#define PNG_INFO_sBIT 0x0002
#define PNG_INFO_cHRM 0x0004
#define PNG_INFO_PLTE 0x0008
#define PNG_INFO_tRNS 0x0010
#define PNG_INFO_bKGD 0x0020
#define PNG_INFO_hIST 0x0040
#define PNG_INFO_pHYs 0x0080
#define PNG_INFO_oFFs 0x0100
#define PNG_INFO_tIME 0x0200
#define PNG_INFO_pCAL 0x0400
#define PNG_INFO_sRGB 0x0800
#define PNG_INFO_iCCP 0x1000
#define PNG_INFO_sPLT 0x2000
#define PNG_INFO_sCAL 0x4000
#define PNG_INFO_IDAT 0x8000L

typedef struct png_row_info_struct
{
   png_uint_32 width;
   png_uint_32 rowbytes;
   png_byte color_type;
   png_byte bit_depth;
   png_byte channels;
   png_byte pixel_depth;
} png_row_info;

typedef png_row_info FAR * png_row_infop;
typedef png_row_info FAR * FAR * png_row_infopp;

typedef struct png_struct_def png_struct;
typedef png_struct FAR * png_structp;

typedef void (*png_error_ptr) PNGARG((png_structp, png_const_charp));
typedef void (*png_rw_ptr) PNGARG((png_structp, png_bytep, png_size_t));
typedef void (*png_flush_ptr) PNGARG((png_structp));
typedef void (*png_read_status_ptr) PNGARG((png_structp, png_uint_32, int));
typedef void (*png_write_status_ptr) PNGARG((png_structp, png_uint_32, int));

#define PNG_TRANSFORM_IDENTITY       0x0000
#define PNG_TRANSFORM_STRIP_16       0x0001
#define PNG_TRANSFORM_STRIP_ALPHA    0x0002
#define PNG_TRANSFORM_PACKING        0x0004
#define PNG_TRANSFORM_PACKSWAP       0x0008
#define PNG_TRANSFORM_EXPAND         0x0010
#define PNG_TRANSFORM_INVERT_MONO    0x0020
#define PNG_TRANSFORM_SHIFT          0x0040
#define PNG_TRANSFORM_BGR            0x0080
#define PNG_TRANSFORM_SWAP_ALPHA     0x0100
#define PNG_TRANSFORM_SWAP_ENDIAN    0x0200
#define PNG_TRANSFORM_INVERT_ALPHA   0x0200
#define PNG_TRANSFORM_STRIP_FILLER   0x0800

typedef png_voidp (*png_malloc_ptr) PNGARG((png_structp, png_size_t));
typedef void (*png_free_ptr) PNGARG((png_structp, png_voidp));

struct png_struct_def
{
#ifdef PNG_SETJMP_SUPPORTED
   jmp_buf jmpbuf;
#endif
   png_error_ptr error_fn;
   png_error_ptr warning_fn;
   png_voidp error_ptr;
   png_rw_ptr write_data_fn;
   png_rw_ptr read_data_fn;
   png_voidp io_ptr;

   png_uint_32 mode;
   png_uint_32 flags;
   png_uint_32 free_me;
   png_uint_32 transformations;

   z_stream zstream;
   png_bytep zbuf;
   png_size_t zbuf_size;
   int zlib_level;
   int zlib_method;
   int zlib_window_bits;
   int zlib_mem_level;
   int zlib_strategy;

   png_uint_32 width;
   png_uint_32 height;
   png_uint_32 num_rows;
   png_uint_32 usr_width;
   png_uint_32 rowbytes;
   png_uint_32 irowbytes;
   png_uint_32 iwidth;
   png_uint_32 row_number;
   png_bytep prev_row;
   png_bytep row_buf;
   png_bytep sub_row;
   png_bytep up_row;
   png_bytep avg_row;
   png_bytep paeth_row;
   png_row_info row_info;
   png_uint_32 idat_size;
   png_uint_32 crc;
   png_colorp palette;
   png_uint_16 num_palette;
   png_uint_16 num_trans;
   png_byte chunk_name[5];
   png_byte compression;
   png_byte filter;
   png_byte interlaced;
   png_byte pass;
   png_byte do_filter;
   png_byte color_type;
   png_byte bit_depth;
   png_byte usr_bit_depth;
   png_byte pixel_depth;
   png_byte channels;
   png_byte usr_channels;
   png_byte sig_bytes;
#if defined(PNG_WRITE_FLUSH_SUPPORTED)
   png_flush_ptr output_flush_fn;
   png_uint_32 flush_dist;
   png_uint_32 flush_rows;
#endif
   png_read_status_ptr read_row_fn;
   png_write_status_ptr write_row_fn;
#if defined(__TURBOC__) && !defined(_Windows) && !defined(__FLAT__)
   png_bytepp offset_table_ptr;
   png_bytep offset_table;
   png_uint_16 offset_table_number;
   png_uint_16 offset_table_count;
   png_uint_16 offset_table_count_free;
#endif
#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
   png_byte heuristic_method;
   png_byte num_prev_filters;
   png_bytep prev_filters;
   png_uint_16p filter_weights;
   png_uint_16p inv_filter_weights;
   png_uint_16p filter_costs;
   png_uint_16p inv_filter_costs;
#endif
#if defined(PNG_TIME_RFC1123_SUPPORTED)
   png_charp time_buffer;
#endif
#if defined(PNG_READ_EMPTY_PLTE_SUPPORTED) || \
    defined(PNG_WRITE_EMPTY_PLTE_SUPPORTED)
   png_byte empty_plte_permitted;
#endif
};

typedef png_structp version_1_0_6;

typedef png_struct FAR * FAR * png_structpp;

extern PNG_EXPORT(png_structp,png_create_write_struct)
   PNGARG((png_const_charp user_png_ver, png_voidp error_ptr,
   png_error_ptr error_fn, png_error_ptr warn_fn));
extern PNG_EXPORT(void,png_write_chunk) PNGARG((png_structp png_ptr,
   png_bytep chunk_name, png_bytep data, png_size_t length));
extern PNG_EXPORT(void,png_write_chunk_start) PNGARG((png_structp png_ptr,
   png_bytep chunk_name, png_uint_32 length));
extern PNG_EXPORT(void,png_write_chunk_data) PNGARG((png_structp png_ptr,
   png_bytep data, png_size_t length));
extern PNG_EXPORT(void,png_write_chunk_end) PNGARG((png_structp png_ptr));
extern PNG_EXPORT(png_infop,png_create_info_struct)
   PNGARG((png_structp png_ptr));
extern void png_info_init PNGARG((png_infop info_ptr));
extern PNG_EXPORT(void,png_write_info_before_PLTE) PNGARG((png_structp png_ptr,
   png_infop info_ptr));
extern PNG_EXPORT(void,png_write_info) PNGARG((png_structp png_ptr,
   png_infop info_ptr));
#if defined(PNG_READ_PACK_SUPPORTED) || defined(PNG_WRITE_PACK_SUPPORTED)
extern PNG_EXPORT(void,png_set_packing) PNGARG((png_structp png_ptr));
#endif
#if defined(PNG_WRITE_FLUSH_SUPPORTED)
extern PNG_EXPORT(void,png_write_flush) PNGARG((png_structp png_ptr));
#endif
extern PNG_EXPORT(void,png_write_row) PNGARG((png_structp png_ptr,
   png_bytep row));
extern PNG_EXPORT(void,png_write_image) PNGARG((png_structp png_ptr,
   png_bytepp image));
extern PNG_EXPORT(void,png_write_end) PNGARG((png_structp png_ptr,
   png_infop info_ptr));
extern PNG_EXPORT(void,png_destroy_write_struct)
   PNGARG((png_structpp png_ptr_ptr, png_infopp info_ptr_ptr));
extern void png_write_destroy PNGARG((png_structp png_ptr));

#define PNG_CRC_DEFAULT       0
#define PNG_CRC_ERROR_QUIT    1
#define PNG_CRC_WARN_DISCARD  2
#define PNG_CRC_WARN_USE      3
#define PNG_CRC_QUIET_USE     4
#define PNG_CRC_NO_CHANGE     5

#define PNG_NO_FILTERS     0x00
#define PNG_FILTER_NONE    0x08
#define PNG_FILTER_SUB     0x10
#define PNG_FILTER_UP      0x20
#define PNG_FILTER_AVG     0x40
#define PNG_FILTER_PAETH   0x80
#define PNG_ALL_FILTERS (PNG_FILTER_NONE | PNG_FILTER_SUB | PNG_FILTER_UP | \
                         PNG_FILTER_AVG | PNG_FILTER_PAETH)

#define PNG_FILTER_VALUE_NONE  0
#define PNG_FILTER_VALUE_SUB   1
#define PNG_FILTER_VALUE_UP    2
#define PNG_FILTER_VALUE_AVG   3
#define PNG_FILTER_VALUE_PAETH 4
#define PNG_FILTER_VALUE_LAST  5

#if defined(PNG_WRITE_WEIGHTED_FILTER_SUPPORTED)
#ifdef PNG_FLOATING_POINT_SUPPORTED
extern PNG_EXPORT(void,png_set_filter_heuristics) PNGARG((png_structp png_ptr,
   int heuristic_method, int num_weights, png_doublep filter_weights,
   png_doublep filter_costs));
#endif
#endif

#define PNG_FILTER_HEURISTIC_DEFAULT    0
#define PNG_FILTER_HEURISTIC_UNWEIGHTED 1
#define PNG_FILTER_HEURISTIC_WEIGHTED   2
#define PNG_FILTER_HEURISTIC_LAST       3

extern PNG_EXPORT(void,png_set_error_fn) PNGARG((png_structp png_ptr,
   png_voidp error_ptr, png_error_ptr error_fn, png_error_ptr warning_fn));
extern PNG_EXPORT(png_voidp,png_get_error_ptr) PNGARG((png_structp png_ptr));
extern PNG_EXPORT(void,png_set_write_fn) PNGARG((png_structp png_ptr,
   png_voidp io_ptr, png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn));
extern PNG_EXPORT(png_voidp,png_get_io_ptr) PNGARG((png_structp png_ptr));
extern PNG_EXPORT(png_voidp,png_malloc) PNGARG((png_structp png_ptr,
   png_uint_32 size));
extern PNG_EXPORT(void,png_free) PNGARG((png_structp png_ptr, png_voidp ptr));
extern PNG_EXPORT(void,png_free_data) PNGARG((png_structp png_ptr,
   png_infop info_ptr, png_uint_32 free_me, int num));
#define PNG_FREE_PLTE 0x0001
#define PNG_FREE_TRNS 0x0002
#define PNG_FREE_TEXT 0x0004
#define PNG_FREE_HIST 0x0008
#define PNG_FREE_ICCP 0x0010
#define PNG_FREE_SPLT 0x0020
#define PNG_FREE_ROWS 0x0040
#define PNG_FREE_PCAL 0x0080
#define PNG_FREE_SCAL 0x0100
#define PNG_FREE_UNKN 0x0200
#define PNG_FREE_LIST 0x0400
#define PNG_FREE_ALL  0x07ff

extern PNG_EXPORT(png_voidp,png_memcpy_check) PNGARG((png_structp png_ptr,
   png_voidp s1, png_voidp s2, png_uint_32 size));
#if defined(USE_FAR_KEYWORD)
extern void *png_far_to_near PNGARG((png_structp png_ptr,png_voidp ptr,
   int check));
#endif
extern PNG_EXPORT(void,png_error) PNGARG((png_structp png_ptr,
   png_const_charp error));
extern PNG_EXPORT(void,png_chunk_error) PNGARG((png_structp png_ptr,
   png_const_charp error));
extern PNG_EXPORT(void,png_warning) PNGARG((png_structp png_ptr,
   png_const_charp message));
extern PNG_EXPORT(void,png_chunk_warning) PNGARG((png_structp png_ptr,
   png_const_charp message));
extern PNG_EXPORT(void,png_set_IHDR) PNGARG((png_structp png_ptr,
   png_infop info_ptr, png_uint_32 width, png_uint_32 height, int bit_depth,
   int color_type, int interlace_type, int compression_type, int filter_type));
extern PNG_EXPORT(void,png_set_PLTE) PNGARG((png_structp png_ptr,
   png_infop info_ptr, png_colorp palette, int num_palette));
#if defined(PNG_tRNS_SUPPORTED)
extern PNG_EXPORT(void,png_set_tRNS) PNGARG((png_structp png_ptr,
   png_infop info_ptr, png_bytep trans, int num_trans,
   png_color_16p trans_values));
#endif
#if defined(PNG_INFO_IMAGE_SUPPORTED)
extern PNG_EXPORT(void, png_write_png) PNGARG((png_structp png_ptr,
                        png_infop info_ptr,
                        int transforms,
                        voidp params));
#endif

#define png_debug(l, m)
#define png_debug1(l, m, p1)
#define png_debug2(l, m, p1, p2)

extern PNG_EXPORT(png_bytep,png_sig_bytes) PNGARG((png_structp png_ptr));
extern PNG_EXPORT(png_charp,png_get_header_ver) PNGARG((png_structp png_ptr));
extern PNG_EXPORT(png_charp,png_get_libpng_ver) PNGARG((png_structp png_ptr));

#define PNG_HEADER_VERSION_STRING \
   " libpng version 1.0.6 - March 21, 2000 (header)\n"

#if defined(PNG_INTERNAL)

#define PNG_HAVE_IHDR               0x01
#define PNG_HAVE_PLTE               0x02
#define PNG_HAVE_IDAT               0x04
#define PNG_AFTER_IDAT              0x08
#define PNG_HAVE_IEND               0x10
#define PNG_HAVE_gAMA               0x20
#define PNG_HAVE_cHRM               0x40
#define PNG_HAVE_sRGB               0x80
#define PNG_HAVE_CHUNK_HEADER      0x100
#define PNG_WROTE_tIME             0x200
#define PNG_WROTE_INFO_BEFORE_PLTE 0x400
#define PNG_BACKGROUND_IS_GRAY     0x800

#define PNG_BGR                0x0001
#define PNG_INTERLACE          0x0002
#define PNG_PACK               0x0004
#define PNG_SHIFT              0x0008
#define PNG_SWAP_BYTES         0x0010
#define PNG_INVERT_MONO        0x0020
#define PNG_DITHER             0x0040
#define PNG_BACKGROUND         0x0080
#define PNG_BACKGROUND_EXPAND  0x0100

#define PNG_16_TO_8            0x0400
#define PNG_RGBA               0x0800
#define PNG_EXPAND             0x1000
#define PNG_GAMMA              0x2000
#define PNG_GRAY_TO_RGB        0x4000
#define PNG_FILLER             0x8000L
#define PNG_PACKSWAP          0x10000L
#define PNG_SWAP_ALPHA        0x20000L
#define PNG_STRIP_ALPHA       0x40000L
#define PNG_INVERT_ALPHA      0x80000L
#define PNG_USER_TRANSFORM   0x100000L
#define PNG_RGB_TO_GRAY_ERR  0x200000L
#define PNG_RGB_TO_GRAY_WARN 0x400000L
#define PNG_RGB_TO_GRAY      0x600000L

#define PNG_STRUCT_PNG   0x0001
#define PNG_STRUCT_INFO  0x0002

#define PNG_WEIGHT_SHIFT 8
#define PNG_WEIGHT_FACTOR (1<<(PNG_WEIGHT_SHIFT))
#define PNG_COST_SHIFT 3
#define PNG_COST_FACTOR (1<<(PNG_COST_SHIFT))

#define PNG_FLAG_ZLIB_CUSTOM_STRATEGY     0x0001
#define PNG_FLAG_ZLIB_CUSTOM_LEVEL        0x0002
#define PNG_FLAG_ZLIB_CUSTOM_MEM_LEVEL    0x0004
#define PNG_FLAG_ZLIB_CUSTOM_WINDOW_BITS  0x0008
#define PNG_FLAG_ZLIB_CUSTOM_METHOD       0x0010
#define PNG_FLAG_ZLIB_FINISHED            0x0020
#define PNG_FLAG_ROW_INIT                 0x0040
#define PNG_FLAG_FILLER_AFTER             0x0080
#define PNG_FLAG_CRC_ANCILLARY_USE        0x0100
#define PNG_FLAG_CRC_ANCILLARY_NOWARN     0x0200
#define PNG_FLAG_CRC_CRITICAL_USE         0x0400
#define PNG_FLAG_CRC_CRITICAL_IGNORE      0x0800
#define PNG_FLAG_KEEP_UNKNOWN_CHUNKS      0x1000 
#define PNG_FLAG_KEEP_UNSAFE_CHUNKS       0x2000

#define HANDLE_CHUNK_AS_DEFAULT   0
#define HANDLE_CHUNK_NEVER        1
#define HANDLE_CHUNK_IF_SAFE      2
#define HANDLE_CHUNK_ALWAYS       3

#define PNG_FLAG_CRC_ANCILLARY_MASK (PNG_FLAG_CRC_ANCILLARY_USE | \
                                     PNG_FLAG_CRC_ANCILLARY_NOWARN)

#define PNG_FLAG_CRC_CRITICAL_MASK  (PNG_FLAG_CRC_CRITICAL_USE | \
                                     PNG_FLAG_CRC_CRITICAL_IGNORE)

#define PNG_FLAG_CRC_MASK           (PNG_FLAG_CRC_ANCILLARY_MASK | \
                                     PNG_FLAG_CRC_CRITICAL_MASK)

#define PNG_COLOR_DIST(c1, c2) (abs((int)((c1).red) - (int)((c2).red)) + \
   abs((int)((c1).green) - (int)((c2).green)) + \
   abs((int)((c1).blue) - (int)((c2).blue)))

#if !defined(PNG_NO_EXTERN) || defined(PNG_ALWAYS_EXTERN)
#ifdef PNG_USE_GLOBAL_ARRAYS
   PNG_EXPORT_VAR (png_byte FARDATA) png_sig[8];
#else
#define png_sig png_sig_bytes(NULL)
#endif
#endif

#define PNG_IHDR const png_byte png_IHDR[5] = { 73,  72,  68,  82, '\0'}
#define PNG_IDAT const png_byte png_IDAT[5] = { 73,  68,  65,  84, '\0'}
#define PNG_IEND const png_byte png_IEND[5] = { 73,  69,  78,  68, '\0'}
#define PNG_PLTE const png_byte png_PLTE[5] = { 80,  76,  84,  69, '\0'}
#define PNG_bKGD const png_byte png_bKGD[5] = { 98,  75,  71,  68, '\0'}
#define PNG_cHRM const png_byte png_cHRM[5] = { 99,  72,  82,  77, '\0'}
#define PNG_gAMA const png_byte png_gAMA[5] = {103,  65,  77,  65, '\0'}
#define PNG_hIST const png_byte png_hIST[5] = {104,  73,  83,  84, '\0'}
#define PNG_iCCP const png_byte png_iCCP[5] = {105,  67,  67,  80, '\0'}
#define PNG_iTXt const png_byte png_iTXt[5] = {105,  84,  88, 116, '\0'}
#define PNG_oFFs const png_byte png_oFFs[5] = {111,  70,  70, 115, '\0'}
#define PNG_pCAL const png_byte png_pCAL[5] = {112,  67,  65,  76, '\0'}
#define PNG_sCAL const png_byte png_sCAL[5] = {115,  67,  65,  76, '\0'}
#define PNG_pHYs const png_byte png_pHYs[5] = {112,  72,  89, 115, '\0'}
#define PNG_sBIT const png_byte png_sBIT[5] = {115,  66,  73,  84, '\0'}
#define PNG_sPLT const png_byte png_sPLT[5] = {115,  80,  76,  84, '\0'}
#define PNG_sRGB const png_byte png_sRGB[5] = {115,  82,  71,  66, '\0'}
#define PNG_tEXt const png_byte png_tEXt[5] = {116,  69,  88, 116, '\0'}
#define PNG_tIME const png_byte png_tIME[5] = {116,  73,  77,  69, '\0'}
#define PNG_tRNS const png_byte png_tRNS[5] = {116,  82,  78,  83, '\0'}
#define PNG_zTXt const png_byte png_zTXt[5] = {122,  84,  88, 116, '\0'}

#ifdef PNG_USE_GLOBAL_ARRAYS
PNG_EXPORT_VAR (const png_byte FARDATA) png_IHDR[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_IDAT[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_IEND[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_PLTE[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_bKGD[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_cHRM[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_gAMA[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_hIST[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_iCCP[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_iTXt[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_oFFs[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_pCAL[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_sCAL[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_pHYs[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_sBIT[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_sPLT[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_sRGB[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_tEXt[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_tIME[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_tRNS[5];
PNG_EXPORT_VAR (const png_byte FARDATA) png_zTXt[5];
#endif /* PNG_USE_GLOBAL_ARRAYS */

PNG_EXTERN png_voidp png_create_struct PNGARG((int type));
PNG_EXTERN void png_destroy_struct PNGARG((png_voidp struct_ptr));
PNG_EXTERN voidpf png_zalloc PNGARG((voidpf png_ptr, uInt items, uInt size));
PNG_EXTERN void png_zfree PNGARG((voidpf png_ptr, voidpf ptr));
PNG_EXTERN void png_reset_crc PNGARG((png_structp png_ptr));
PNG_EXTERN void png_write_data PNGARG((png_structp png_ptr, png_bytep data,
   png_size_t length));
PNG_EXTERN void png_calculate_crc PNGARG((png_structp png_ptr, png_bytep ptr,
   png_size_t length));
#if defined(PNG_WRITE_FLUSH_SUPPORTED)
PNG_EXTERN void png_flush PNGARG((png_structp png_ptr));
#endif
PNG_EXTERN void png_save_uint_32 PNGARG((png_bytep buf, png_uint_32 i));
PNG_EXTERN void png_write_sig PNGARG((png_structp png_ptr));
PNG_EXTERN void png_write_IHDR PNGARG((png_structp png_ptr, png_uint_32 width,
   png_uint_32 height,
   int bit_depth, int color_type, int compression_type, int filter_type,
   int interlace_type));
PNG_EXTERN void png_write_PLTE PNGARG((png_structp png_ptr, png_colorp palette,
   png_uint_32 num_pal));
PNG_EXTERN void png_write_IDAT PNGARG((png_structp png_ptr, png_bytep data,
   png_size_t length));
PNG_EXTERN void png_write_IEND PNGARG((png_structp png_ptr));
#if defined(PNG_WRITE_tRNS_SUPPORTED)
PNG_EXTERN void png_write_tRNS PNGARG((png_structp png_ptr, png_bytep trans,
   png_color_16p values, int number, int color_type));
#endif
PNG_EXTERN void png_write_finish_row PNGARG((png_structp png_ptr));
PNG_EXTERN void png_write_start_row PNGARG((png_structp png_ptr));
PNG_EXTERN void png_write_find_filter PNGARG((png_structp png_ptr,
   png_row_infop row_info));
PNG_EXTERN void png_write_filtered_row PNGARG((png_structp png_ptr,
   png_bytep filtered_row));
#if defined(PNG_WRITE_PACK_SUPPORTED)
PNG_EXTERN void png_do_pack PNGARG((png_row_infop row_info,
   png_bytep row, png_uint_32 bit_depth));
#endif
PNG_EXTERN void png_do_write_transformations PNGARG((png_structp png_ptr));
#endif  /* PNG_INTERNAL */
#endif  /* _PNG_H */
#endif  /* HAVE_GD */
#endif  /* NOGRAPHICS */
