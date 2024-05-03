/* 
/ readosm.c
/
/ ReadOSM main implementation (externally visible API)
/
/ version  1.1.0, 2017 September 25
/
/ Author: Sandro Furieri a.furieri@lqt.it
/
/ ------------------------------------------------------------------------------
/ 
/ Version: MPL 1.1/GPL 2.0/LGPL 2.1
/ 
/ The contents of this file are subject to the Mozilla Public License Version
/ 1.1 (the "License"); you may not use this file except in compliance with
/ the License. You may obtain a copy of the License at
/ http://www.mozilla.org/MPL/
/ 
/ Software distributed under the License is distributed on an "AS IS" basis,
/ WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
/ for the specific language governing rights and limitations under the
/ License.
/
/ The Original Code is the ReadOSM library
/
/ The Initial Developer of the Original Code is Alessandro Furieri
/ 
/ Portions created by the Initial Developer are Copyright (C) 2012-2017
/ the Initial Developer. All Rights Reserved.
/ 
/ Contributor(s):
/ 
/ Alternatively, the contents of this file may be used under the terms of
/ either the GNU General Public License Version 2 or later (the "GPL"), or
/ the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
/ in which case the provisions of the GPL or the LGPL are applicable instead
/ of those above. If you wish to allow use of your version of this file only
/ under the terms of either the GPL or the LGPL, and not to allow others to
/ use your version of this file under the terms of the MPL, indicate your
/ decision by deleting the provisions above and replace them with the notice
/ and other provisions required by the GPL or the LGPL. If you do not delete
/ the provisions above, a recipient may use your version of this file under
/ the terms of any one of the MPL, the GPL or the LGPL.
/ 
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


unsigned int    cur_uncompressed_buffer_size = 0;
unsigned char  *ptr_uncompressed_buffer      = NULL;
// unsigned char  *ptr_uncompressed_buffer_cur  = NULL;


void wrong_assumption(char* txt) {
   printf("\033[1;31m%s\033[0m\n", txt);
   exit(1);
}


   #define TQ84_USE_PBF_FIELD_HINTS
// #define TQ84_VERBOSE_1

#ifdef TQ84_VERBOSE_1
#define verbose_1(...) printf(__VA_ARGS__)
#else
#define verbose_1(...)
#endif

#include "readosm.h"
#include "protobuf.c"
#include "osm_objects.c"

#include "readosm.h"
#include "readosm_internals.h"

// #include "endian.c"
//

// readosm_node_callback        g_cb_nod;
// readosm_way_callback         g_cb_way;
// readosm_relation_callback    g_cb_rel;
// char                         g_little_endian_cpu;

static int test_endianness () {

/* checks the current CPU endianness */
    four_byte_value endian4;
    endian4.bytes[0] = 0x01;
    endian4.bytes[1] = 0x00;
    endian4.bytes[2] = 0x00;
    endian4.bytes[3] = 0x00;
    if (endian4.uint32_value == 1) return READOSM_LITTLE_ENDIAN;
    return READOSM_BIG_ENDIAN;
}


#if 0
static int read_osm_data_block (unsigned int sz) {
 //
 // expecting to retrieve a valid OSMData header
 //

    int ok_header = 0;
    int hdsz      = 0;
    size_t               rd;
    unsigned char       *buf                = malloc (sz);
    unsigned char       *base               = buf;
    unsigned char       *start              = buf;
    unsigned char       *stop               = buf + sz - 1;
    unsigned char       *zip_ptr            = NULL;
    int                  zip_sz             = 0;
    unsigned char       *raw_ptr            = NULL;
    int                  sz_no_compression  = 0;
    pbf_field            variant;


    verbose_1("  read_osm_data_block sz = %d\n", sz);

    if (buf == NULL)
        goto error;

 // initializing an empty string list
//     init_string_table (&string_table);
//     static void init_string_table (readosm_string_table * string_table) {
   /* initializing an empty PBF StringTable object */
       readosm_string_table string_table;
       string_table.first_string   = NULL;
       string_table.last_string    = NULL;
       string_table.count          =    0;
       string_table.strings        = NULL;
// }


 // initializing an empty variant field
    init_variant      (&variant, g_little_endian_cpu /* input->little_endian_cpu */);
   #ifdef TQ84_USE_PBF_FIELD_HINTS
    add_variant_hints (&variant, READOSM_LEN_BYTES, 1);
    add_variant_hints (&variant, READOSM_LEN_BYTES, 2);
    add_variant_hints (&variant, READOSM_VAR_INT32, 3);
   #endif

    rd = fread (buf, 1, sz, g_pbf_file);
    if (rd != sz)
        goto error;

     //  --------------------------------------------------------------------- Header ---------------------------------------------------------------------------------------------------

 //
 // reading the OSMData header
 //
    while (1) {
          verbose_1("    iterating (read_osm_data_block)\n");

       // resetting an empty variant field
          reset_variant (&variant);

          base = read_pbf_field (start, stop, &variant);

          if (base == NULL && variant.valid == 0)
              goto error;

          start = base;

          if (variant.field_id == 1 && variant.type == READOSM_LEN_BYTES && variant.str_len == 7) {
                verbose_1("       field_id = 1\n");
                if (memcmp (variant.pointer, "OSMData", 7) == 0) ok_header = 1;
          }

          if (variant.field_id == 3 && variant.type == READOSM_VAR_INT32) {
              hdsz = variant.value.int32_value;
              verbose_1("       field_id = 3, hdsz = %d\n", hdsz);
          }

          if (base > stop)
              break;

    }

    free (buf);
    buf = NULL;
    if (!ok_header || !hdsz)
        goto error;

    buf   = malloc (hdsz);
    base  = buf;
    start = buf;
    stop  = buf + hdsz - 1;
    rd    = fread (buf, 1, hdsz, g_pbf_file);
    if ((int) rd != hdsz)
        goto error;

// uncompressing the OSMData zipped */
   #ifdef TQ84_USE_PBF_FIELD_HINTS
    finalize_variant  (&variant);
    add_variant_hints (&variant, READOSM_LEN_BYTES, 1);
    add_variant_hints (&variant, READOSM_VAR_INT32, 2);
    add_variant_hints (&variant, READOSM_LEN_BYTES, 3);
   #endif
    while (1) {
          verbose_1("    iterating again (read_osm_data_block)\n");
       // resetting an empty variant field
          reset_variant (&variant);

          base = read_pbf_field (start, stop, &variant);
          if (base == NULL && variant.valid == 0)
              goto error;

          start = base;
          if (variant.field_id == 1 && variant.type == READOSM_LEN_BYTES) {
             // found an uncompressed block */
                verbose_1("      uncompressed block\n");
                wrong_assumption("uncompressed block don't exist");
                sz_no_compression = variant.str_len;
                raw_ptr = malloc (sz_no_compression);
                memcpy (raw_ptr, variant.pointer, sz_no_compression);
          }

          if (variant.field_id == 2 && variant.type == READOSM_VAR_INT32) {

             // expected size of unZipped block */
                sz_no_compression = variant.value.int32_value;
                verbose_1("      size of uncompressed block %d\n", sz_no_compression);
          }

          if (variant.field_id == 3 && variant.type == READOSM_LEN_BYTES) {
                verbose_1("      zipped block\n");
             // found a ZIP-compressed block
                zip_ptr = variant.pointer;
                zip_sz  = variant.str_len;
          }
          if (base > stop)
              break;
    }

    if (zip_ptr != NULL && zip_sz != 0 && sz_no_compression != 0) {
          /* unZipping a compressed block */
          raw_ptr = malloc (sz_no_compression);
          if (!unzip_compressed_block (zip_ptr, zip_sz, raw_ptr, sz_no_compression))
              goto error;
    }

    free (buf);
    buf = NULL;
    if (raw_ptr == NULL || sz_no_compression == 0)
        goto error;

     //  --------------------------------------------------------------------- PrimitiveBlock ---------------------------------------------------------------------------------------------------

 // parsing the PrimitiveBlock

    base  = raw_ptr;
    start = raw_ptr;
    stop  = raw_ptr + sz_no_compression - 1;
   #ifdef TQ84_USE_PBF_FIELD_HINTS
    finalize_variant (&variant);
    add_variant_hints (&variant, READOSM_LEN_BYTES,  1);
    add_variant_hints (&variant, READOSM_LEN_BYTES,  2);
    add_variant_hints (&variant, READOSM_VAR_INT32, 17);
    add_variant_hints (&variant, READOSM_VAR_INT32, 18);
    add_variant_hints (&variant, READOSM_VAR_INT64, 19);
    add_variant_hints (&variant, READOSM_VAR_INT64, 20);
   #endif

    while (1) {
       // resetting an empty variant field
          reset_variant (&variant);

          base = read_pbf_field (start, stop, &variant);
          if (base == NULL && variant.valid == 0)
              goto error;

          start = base;
          if (variant.field_id == 1 && variant.type == READOSM_LEN_BYTES) {

             // the StringTable
                if (!parse_string_table (
                     &string_table,
                     variant.pointer,
                     variant.pointer + variant.str_len - 1,
                     variant.little_endian_cpu
                   ))
                   goto error;

                array_from_string_table (&string_table);
          }

          if (variant.field_id == 2 && variant.type == READOSM_LEN_BYTES) {

             // the PrimitiveGroup to be parsed
                if (!parse_primitive_group (
                    &string_table, variant.pointer,
                     variant.pointer + variant.str_len - 1,
                     variant.little_endian_cpu
                     // , params
                    ))

                    goto error;
          }

          if (variant.field_id == 17 && variant.type == READOSM_VAR_INT32) {
             // assumed to be a termination marker (???)
                break;
          }

          if (base > stop)
              break;
      }

    if (buf != NULL)
        free (buf);

    if (raw_ptr != NULL)
        free (raw_ptr);

   #ifdef TQ84_USE_PBF_FIELD_HINTS
    finalize_variant (&variant);
   #endif

    finalize_string_table (&string_table);
    return 1;

  error:
    if (buf != NULL)
        free (buf);
    if (raw_ptr != NULL)
        free (raw_ptr);
   #ifdef TQ84_USE_PBF_FIELD_HINTS
    finalize_variant (&variant);
   #endif

    finalize_string_table (&string_table);
    return 0;
}
#endif

static unsigned int blob_size() {
    size_t        rd;
    unsigned char buf_4[4];

    rd = fread (buf_4, 1, 4, g_pbf_file);

    if (rd == 0 && feof(g_pbf_file))
       return 0;

    if (rd != 4) exit(80); // return READOSM_INVALID_PBF_HEADER;

    return get_header_size (buf_4);

}

static int set_uncompressed_buffer(int req_uncompressed_buffer_size) {

    if (req_uncompressed_buffer_size > cur_uncompressed_buffer_size) {
       printf("increase uncompressed buffer from %d to %d\n", cur_uncompressed_buffer_size, req_uncompressed_buffer_size);
       free(ptr_uncompressed_buffer);
       cur_uncompressed_buffer_size = req_uncompressed_buffer_size;
       ptr_uncompressed_buffer      = malloc(cur_uncompressed_buffer_size);
    }
//    ptr_uncompressed_buffer_cur = ptr_uncompressed_buffer;
}



#if 0
static int read_header_block (unsigned int sz) {

//
// expecting to retrieve a valid OSMHeader header 
// there is nothing really interesting here, so we'll
// simply discard the whole block, simply advancing
// the read file-pointer as appropriate
//

    verbose_1("  read_header_block, sz = %d\n", sz);

    if (sz != 14) {
       wrong_assumption("parameter sz of read_header_block was expected to be 14");
    }

    int ok_header = 0;
    int hdsz      = 0;

    size_t rd;
    unsigned char *buf   = malloc (sz);
//  unsigned char *base  = buf;
//  unsigned char *start = buf;
//  unsigned char *stop  = buf + sz - 1;
    unsigned char *cur   = buf;
    unsigned char *end   = buf + sz - 1;
    pbf_field      fld;

    if (buf == NULL)
        goto error;

 // initializing an empty fld field 
    init_variant (&fld, g_little_endian_cpu);
   #ifdef TQ84_USE_PBF_FIELD_HINTS
    add_variant_hints (&fld, READOSM_LEN_BYTES, 1);
    add_variant_hints (&fld, READOSM_LEN_BYTES, 2);
    add_variant_hints (&fld, READOSM_VAR_INT32, 3);
   #endif

    rd = fread (buf, 1, sz, g_pbf_file);
    if (rd != sz)
        goto error;

// reading the OSMHeader header
//
    while (1) {
       verbose_1("    next iteration (read_header_block)\n");
       // resetting an empty fld field
          reset_variant (&fld);

//        base = read_pbf_field (start, stop, &fld);
          cur  = read_pbf_field (cur  , end , &fld);
//        if (base == NULL && fld.valid == 0)
          if (cur  == NULL && fld.valid == 0)
              goto error;

//        start = base;
          if (fld.field_id == 1 && fld.type == READOSM_LEN_BYTES && fld.str_len == 9) {

                verbose_1("      field_id == 1\n");
                if (memcmp (fld.pointer, "OSMHeader", 9) == 0)
                    ok_header = 1;
          }
          else if (fld.field_id == 3 && fld.type == READOSM_VAR_INT32) {
              hdsz = fld.value.int32_value;
              verbose_1("      field_id == 3, hdsz = %d\n", hdsz);
          }
          else {
              wrong_assumption("else");
              verbose_1("      else\n");
          }

//        if (base > stop) {
          if (cur  > end ) {
              verbose_1("      base > stop\n");
              break;
          }
    }

    free (buf);
//  buf = NULL;


    if (!ok_header || !hdsz)
        goto error;

//
//  Just SKIP OVER the rest of the header buffer!
//

    buf   = malloc (hdsz);
//  base  = buf;
//  start = buf;
//  stop  = buf + hdsz - 1;

    rd = fread (buf, 1, hdsz, g_pbf_file);

    if ((int) rd != hdsz)
        goto error;

    if (buf != NULL)
        free (buf);

   #ifdef TQ84_USE_PBF_FIELD_HINTS
    finalize_variant (&fld);
   #endif
   
// exit(100); // TQ84 - remove moe
    return 1;

  error:
    if (buf != NULL)
        free (buf);

   #ifdef TQ84_USE_PBF_FIELD_HINTS
    finalize_variant (&fld);
   #endif
    return 0;
}
#endif



typedef struct {

 // a PBF Variant type wrapper

//  char          little_endian_cpu;     // actual CPU endianness
//
// A PBF field is prefixed by a single byte which 
// stores both the FIELD ID (n bits )and the the FIELD type (8-n bites)
// The function read_pbf_field() splits the byte and assigns it to the following
// two bytes:
// 

//  unsigned char type;          // current type
    unsigned char protobuf_type;
    unsigned char field_id;      // field ID

    union variant_value_v2 {

        int                 int32_value;
        unsigned int       uint32_value;
        long long           int64_value;
        unsigned long long uint64_value;
        float               float_value;
        double             double_value;
    }
    value;                      // The field value

    size_t                str_len;     // length in bytes [for strings]
    unsigned char        *pointer;     // pointer to String value
//  char                  valid;       // valid value
}
pbf_field_v2;

   #define PROTOBUF_TYPE_VARINT 0
// #define PROTOBUF_TYPE_I64    1  // Not currently referenced in this program
   #define PROTOBUF_TYPE_LEN    2
// #define PROTOBUF_TYPE_SGROUP 3  // Not currently referenced in this program / deprecated
// #define PROTOBUF_TYPE_EGROUP 4  // Not currently referenced in this program / deprecated
// #define PROTOBUF_TYPE_I32    5  // Not currently referenced in this program


static unsigned char *read_integer_pbf_field_v2 (unsigned char *start, unsigned char *stop, unsigned char field_type, pbf_field_v2 *variant) {

/* 
 / attempting to read a variable length base128 int 
 /
 / PBF integers are encoded as base128, i.e. using 7 bits
 / for each byte: if the most significant bit is 1, then
 / a further byte is required to get the int value, and so
 / on, until a byte having a 0 most significant bit is found.
 /
 / using this encoding little values simply require few bytes:
 / as a worst case 5 bytes are required to encode int32, and
 / 10 bytes to encode int64
 /
 / there is a further complication: negative value will always 
 / require 5 or 10 bytes: thus SINT32 and SINT64 values are
 / encoded using a "ZigZag" schema.
 /
 / for more details please see:
 / https://developers.google.com/protocol-buffers/docs/encoding
*/
    unsigned char      *ptr = start;
    unsigned char       c;
    unsigned int        v32;
    unsigned long long  v64;
    unsigned int        value32 = 0x00000000;
    unsigned long long  value64 = 0x0000000000000000;
    four_byte_value     endian4;
    eight_byte_value    endian8;
    int next;
    int count = 0;
    int neg;

    while (1) {

          if (ptr > stop) {
              wrong_assumption("read_integer_pbf_field, ptr > stop");
              return NULL;
          }

          c = *ptr++;
          if ((c & 0x80) == 0x80)
              next = 1;
          else
              next = 0;

          c &= 0x7f;

          switch (field_type /* variant->type*/) {

            case READOSM_VAR_INT32:
            case READOSM_VAR_UINT32:
            case READOSM_VAR_SINT32:
            
                switch (count) {
                  case 0: memset (endian4.bytes, 0x00, 4); if (g_little_endian_cpu) endian4.bytes[0] = c; else endian4.bytes[3] = c; v32 = endian4.uint32_value      ; v32 &= READOSM_MASK32_1; value32 |= v32; break;
                  case 1: memset (endian4.bytes, 0x00, 4); if (g_little_endian_cpu) endian4.bytes[0] = c; else endian4.bytes[3] = c; v32 = endian4.uint32_value <<  7; v32 &= READOSM_MASK32_2; value32 |= v32; break;
                  case 2: memset (endian4.bytes, 0x00, 4); if (g_little_endian_cpu) endian4.bytes[0] = c; else endian4.bytes[3] = c; v32 = endian4.uint32_value << 14; v32 &= READOSM_MASK32_3; value32 |= v32; break;
                  case 3: memset (endian4.bytes, 0x00, 4); if (g_little_endian_cpu) endian4.bytes[0] = c; else endian4.bytes[3] = c; v32 = endian4.uint32_value << 21; v32 &= READOSM_MASK32_4; value32 |= v32; break;
                  case 4: memset (endian4.bytes, 0x00, 4); if (g_little_endian_cpu) endian4.bytes[0] = c; else endian4.bytes[3] = c; v32 = endian4.uint32_value << 28; v32 &= READOSM_MASK32_5; value32 |= v32; break;
                  default:
                      return NULL;
                };
                break;

            case READOSM_VAR_INT64:
            case READOSM_VAR_UINT64:
            case READOSM_VAR_SINT64:
                switch (count) {

                  case 0: memset (endian8.bytes, 0x00, 8); if (g_little_endian_cpu) endian8.bytes[0] = c; else endian8.bytes[7] = c; v64 = endian8.uint64_value      ; v64 &= READOSM_MASK64_1; value64 |= v64; break;
                  case 1: memset (endian8.bytes, 0x00, 8); if (g_little_endian_cpu) endian8.bytes[0] = c; else endian8.bytes[7] = c; v64 = endian8.uint64_value <<  7; v64 &= READOSM_MASK64_2; value64 |= v64; break;
                  case 2: memset (endian8.bytes, 0x00, 8); if (g_little_endian_cpu) endian8.bytes[0] = c; else endian8.bytes[7] = c; v64 = endian8.uint64_value << 14; v64 &= READOSM_MASK64_3; value64 |= v64; break;
                  case 3: memset (endian8.bytes, 0x00, 8); if (g_little_endian_cpu) endian8.bytes[0] = c; else endian8.bytes[7] = c; v64 = endian8.uint64_value << 21; v64 &= READOSM_MASK64_4; value64 |= v64; break;
                  case 4: memset (endian8.bytes, 0x00, 8); if (g_little_endian_cpu) endian8.bytes[0] = c; else endian8.bytes[7] = c; v64 = endian8.uint64_value << 28; v64 &= READOSM_MASK64_5; value64 |= v64; break;
                  case 5: memset (endian8.bytes, 0x00, 8); if (g_little_endian_cpu) endian8.bytes[0] = c; else endian8.bytes[7] = c; v64 = endian8.uint64_value << 35; v64 &= READOSM_MASK64_6; value64 |= v64; break;
                  case 6: memset (endian8.bytes, 0x00, 8); if (g_little_endian_cpu) endian8.bytes[0] = c; else endian8.bytes[7] = c; v64 = endian8.uint64_value << 42; v64 &= READOSM_MASK64_7; value64 |= v64; break;
                  case 7: memset (endian8.bytes, 0x00, 8); if (g_little_endian_cpu) endian8.bytes[0] = c; else endian8.bytes[7] = c; v64 = endian8.uint64_value << 49; v64 &= READOSM_MASK64_8; value64 |= v64; break;
                  case 8: memset (endian8.bytes, 0x00, 8); if (g_little_endian_cpu) endian8.bytes[0] = c; else endian8.bytes[7] = c; v64 = endian8.uint64_value << 56; v64 &= READOSM_MASK64_9; value64 |= v64; break;
                  case 9: memset (endian8.bytes, 0x00, 8); if (g_little_endian_cpu) endian8.bytes[0] = c; else endian8.bytes[7] = c; v64 = endian8.uint64_value << 63; v64 &= READOSM_MASK64_A; value64 |= v64; break;
                  default:
                      return NULL;
                };
                break;
            };
          count++;
          if (!next)
              break;
      }

    switch (field_type /* variant->type */) {

      case READOSM_VAR_INT32:
           variant->value.int32_value = (int) value32;
//         variant->valid = 1;
           return ptr;

      case READOSM_VAR_UINT32:
           variant->value.uint32_value = value32;
//         variant->valid = 1;
           return ptr;

      case READOSM_VAR_SINT32:
           if ((value32 & 0x00000001) == 0)
               neg = 1;
           else
               neg = -1;

           v32 = (value32 + 1) / 2;
           variant->value.int32_value = v32 * neg;
//         variant->valid = 1;
           return ptr;

      case READOSM_VAR_INT64:
           variant->value.int64_value = (int) value64;
//         variant->valid = 1;
           return ptr;

      case READOSM_VAR_UINT64:
           variant->value.uint64_value = value64;
//         variant->valid = 1;
           return ptr;

      case READOSM_VAR_SINT64:
           if ((value64 & 0x0000000000000001) == 0)
               neg = 1;
           else
               neg = -1;
           v64 = (value64 + 1) / 2;
           variant->value.int64_value = v64 * neg;
//         variant->valid = 1;
           return ptr;
      };
    wrong_assumption("xyz");
    exit(100);
    return NULL;
}

static unsigned char * read_bytes_pbf_field_v2 (unsigned char *start, unsigned char *stop, pbf_field_v2 *variant) {
 /* 
 / attempting to read some bytes from PBF
 / Strings and alike are encoded in PBF using a two steps approach:
 / - an INT32 field declares the expected length
 / - then the string (no terminating NULL char) follows
*/
    unsigned char *ptr = start;
    pbf_field varlen;
    unsigned int len;

/* initializing an empty variant field (length) */
    init_variant (&varlen, g_little_endian_cpu);
    varlen.type = READOSM_VAR_UINT32;

    ptr = read_integer_pbf_field (ptr, stop, &varlen);

    if (varlen.valid) {
          len = varlen.value.uint32_value;
          if ((ptr + len - 1) > stop)
              return NULL;

          variant->pointer = ptr;
          variant->str_len = len;
//        variant->valid   = 1;
          return ptr + len;
    }
    wrong_assumption("bla bla");
    return NULL;
}
static unsigned char *read_pbf_field_v2_protobuf_type_and_field (
   unsigned char *ptr,
   pbf_field_v2  *fld
)
{


//
// any PBF field is prefixed by a single byte
// a bitwise mask is used so to store both the
// field-id and the field-type on a single byte
//

    fld -> protobuf_type     =  *ptr & 0x07;
    fld -> field_id          = (*ptr & 0xf8) >> 3;

    ptr++;
    return ptr;
}




static int read_header_block_v2() {
    unsigned int  sz;
    size_t        rd;

//
// expecting to retrieve a valid OSMHeader header 
// there is nothing really interesting here, so we'll
// simply discard the whole block, simply advancing
// the read file-pointer as appropriate
//

    verbose_1("  read_header_block, sz = %d\n", sz);

    sz = blob_size();//get_header_size (buf_4);

//  if (sz != 14) {
//     wrong_assumption("parameter sz of read_header_block was expected to be 14");
//  }

    int hdsz      = 0;


    unsigned char *buf   = malloc (sz);
    if (buf == NULL) {
        wrong_assumption("buf");
    }

    rd = fread (buf, 1, sz, g_pbf_file);
    if (rd != sz) {
       wrong_assumption("rd == sz");
    }

    unsigned char *cur   = buf;
    unsigned char *end   = buf + sz - 1;

// -----------------------------------------------------------------------

    pbf_field_v2    fld_block_name;

    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld_block_name);

    if (fld_block_name.field_id != 1) {
       printf("field id = %d\n", fld_block_name.field_id);
       wrong_assumption("field id");
    }
    if (fld_block_name.protobuf_type != PROTOBUF_TYPE_LEN) {
       wrong_assumption("PROTOBUF_TYPE_LEN");
    }
    cur = read_bytes_pbf_field_v2 (cur, end, &fld_block_name);

    if (fld_block_name.str_len == 9) {

          verbose_1("      field_id == 1\n");
          if (memcmp (fld_block_name.pointer, "OSMHeader", 9)) {
              wrong_assumption("block name");
          }
    }

// -----------------------------------------------------------------------
    pbf_field_v2    fld_block_size;

    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld_block_size);
    verbose_1("      read block size\n");

    if (fld_block_size.field_id != 3) {
       printf("field id = %d\n", fld_block_size.field_id);
       wrong_assumption("field id != 3");
    }
    if (fld_block_size.protobuf_type != PROTOBUF_TYPE_VARINT) {
       wrong_assumption("PROTOBUF_TYPE_VARINT");
    }

    cur = read_integer_pbf_field_v2(cur, end, READOSM_VAR_INT32, &fld_block_size);
    verbose_1("      read integer\n");

    hdsz = fld_block_size.value.int32_value;
    verbose_1("      hdsz = %d\n", hdsz);


    free (buf);

    if (!hdsz) {
        wrong_assumption("ok header, hdsz");
    }

//
//  Just SKIP OVER the rest of the header buffer!
//
    unsigned char *rest_of_header_buffer = malloc (hdsz);
    rd = fread (rest_of_header_buffer, 1, hdsz, g_pbf_file);

    if (!rd) {
        wrong_assumption("rd");
    }
    if ((int) rd != hdsz) {
        wrong_assumption("rd != hdsz");
    }

    free (rest_of_header_buffer);

   
// exit(100); // TQ84 - remove moe
    verbose_1("       returning from read_header_block_v2\n");
    return 1;

}


static int read_osm_data_block_v2 () {
 //
 // expecting to retrieve a valid OSMData header
 //

    verbose_1("  read_osm_data_block\n");


// if (sz > max_buffer_size) {
//   max_buffer_size = sz;
//   printf("max_buffer_size = %d\n", max_buffer_size);
// }

    int ok_header = 0;
    int hdsz      = 0;
    size_t               rd;
    unsigned char       *buf                ; // = malloc (sz);

//  unsigned char       *base               ; // = buf;
//  unsigned char       *start              ; // = buf;
//  unsigned char       *stop               ; // = buf + sz - 1;
    unsigned char       *cur                ; // = buf;
    unsigned char       *end                ; // = buf + sz - 1;

    unsigned char       *zip_ptr            = NULL;
    int                  zip_sz             = 0;
    int                  sz_no_compression  = 0;
    pbf_field            variant;


    unsigned int sz;
    sz = blob_size();
    if (!sz) {
       return 0;
    }
    verbose_1("    sz = %d\n", sz);

    buf = malloc(sz);
    if (buf == NULL) {
       wrong_assumption("could not allocated buffer.");
    }

    cur = buf;
    end = buf + sz-1;

 // initializing an empty string list
//     init_string_table (&string_table);
//     static void init_string_table (readosm_string_table * string_table) {
   /* initializing an empty PBF StringTable object */
       readosm_string_table string_table;
       string_table.first_string   = NULL;
       string_table.last_string    = NULL;
       string_table.count          =    0;
       string_table.strings        = NULL;
// }


 // initializing an empty variant field
    init_variant      (&variant, g_little_endian_cpu);
   #ifdef TQ84_USE_PBF_FIELD_HINTS
    add_variant_hints (&variant, READOSM_LEN_BYTES, 1);
    add_variant_hints (&variant, READOSM_LEN_BYTES, 2);
    add_variant_hints (&variant, READOSM_VAR_INT32, 3);
   #endif

    rd = fread (buf, 1, sz, g_pbf_file);
    if (rd != sz) {
       wrong_assumption("fread is ok");
    }

     //  --------------------------------------------------------------------- Header ---------------------------------------------------------------------------------------------------

#if 1
 //
 // reading the OSMData header
 //
    while (1) {
          verbose_1("    iterating (read_osm_data_block)\n");

       // resetting an empty variant field
          reset_variant (&variant);

          cur = read_pbf_field (cur, end, &variant);

          if (cur == NULL && variant.valid == 0) {
             wrong_assumption("xyz");
              goto error;
          }

//        start = base;

          if (variant.field_id == 1 && variant.type == READOSM_LEN_BYTES && variant.str_len == 7) {
                verbose_1("       field_id = 1\n");
                if (memcmp (variant.pointer, "OSMData", 7) == 0) ok_header = 1;
          }

          if (variant.field_id == 3 && variant.type == READOSM_VAR_INT32) {
              hdsz = variant.value.int32_value;
              verbose_1("       field_id = 3, hdsz = %d\n", hdsz);
          }

          if (cur > end)
              break;

    }
#else
 // -------------------------------------------------------------------------

    pbf_field_v2 fld_block_name;
    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld_block_name);

    if (fld_block_name.field_id != 1) {
       printf("field id = %d\n", fld_block_name.field_id);
       wrong_assumption("field id");
    }
    if (fld_block_name.protobuf_type != PROTOBUF_TYPE_LEN) {
       wrong_assumption("PROTOBUF_TYPE_LEN");
    }
    cur = read_bytes_pbf_field_v2 (cur, end, &fld_block_name);

    if (fld_block_name.str_len == 9) {

          verbose_1("      field_id == 1\n");
          if (memcmp (fld_block_name.pointer, "OSMHeader", 9)) {
              wrong_assumption("block name");
          }
    }

#endif
 // -------------------------------------------------------------------------

    free (buf);
    buf = NULL;
    if (!ok_header || !hdsz)
        goto error;

     //  --------------------------------------------------------------------- Data   ---------------------------------------------------------------------------------------------------

    buf   = malloc (hdsz);
    if (!buf) {
       wrong_assumption("buf");
    }
//  base  = buf;
//  start = buf;
//  stop  = buf + hdsz - 1;
    cur   = buf;
    end   = buf+hdsz-1;

    rd    = fread (buf, 1, hdsz, g_pbf_file);
    if ((int) rd != hdsz) {
       wrong_assumption("vbla");
    }

// uncompressing the OSMData zipped */
   #ifdef TQ84_USE_PBF_FIELD_HINTS
    finalize_variant  (&variant);
    add_variant_hints (&variant, READOSM_LEN_BYTES, 1);
    add_variant_hints (&variant, READOSM_VAR_INT32, 2);
    add_variant_hints (&variant, READOSM_LEN_BYTES, 3);
   #endif
    while (1) {
          verbose_1("    iterating again (read_osm_data_block)\n");
       // resetting an empty variant field
          reset_variant (&variant);

          cur = read_pbf_field (cur, end, &variant);
          if (cur == NULL && variant.valid == 0) {
              wrong_assumption("heidi");
          }

//        start = base;
          if (variant.field_id == 1 && variant.type == READOSM_LEN_BYTES) {
             // found an uncompressed block */
                verbose_1("      uncompressed block\n");
                wrong_assumption("uncompressed block don't exist");
                sz_no_compression = variant.str_len;


//              if (sz_no_compression > cur_uncompressed_buffer_size) {
//                 printf("increase uncompressed buffer from %d to %d\n", cur_uncompressed_buffer_size, sz_no_compression);
//                 free(ptr_uncompressed_buffer);
//                 cur_uncompressed_buffer_size = sz_no_compression;
//                 ptr_uncompressed_buffer = malloc(cur_uncompressed_buffer_size);
//              }
                set_uncompressed_buffer(sz_no_compression);

//              raw_ptr = malloc (sz_no_compression);

//              memcpy (raw_ptr, variant.pointer, sz_no_compression);
                memcpy (ptr_uncompressed_buffer, variant.pointer, sz_no_compression);
          }

          if (variant.field_id == 2 && variant.type == READOSM_VAR_INT32) {

             // expected size of unZipped block */
                sz_no_compression = variant.value.int32_value;
                verbose_1("      size of uncompressed block %d\n", sz_no_compression);
          }

          if (variant.field_id == 3 && variant.type == READOSM_LEN_BYTES) {
                verbose_1("      zipped block\n");
             // found a ZIP-compressed block
                zip_ptr = variant.pointer;
                zip_sz  = variant.str_len;
          }
//        if (base > stop)
          if (cur > end)
              break;
    }

    if (zip_ptr != NULL && zip_sz != 0 && sz_no_compression != 0) {
    // unzip a compressed block

//        raw_ptr = malloc (sz_no_compression);
//        if (sz_no_compression > cur_uncompressed_buffer_size) {
//           printf("increase uncompressed buffer from %d to %d\n", cur_uncompressed_buffer_size, sz_no_compression);
//           free(ptr_uncompressed_buffer);
//           cur_uncompressed_buffer_size = sz_no_compression;
//           ptr_uncompressed_buffer = malloc(cur_uncompressed_buffer_size);
//        }
          set_uncompressed_buffer(sz_no_compression);
//        raw_ptr = malloc (sz_no_compression);


                  
//        if (!unzip_compressed_block (zip_ptr, zip_sz, raw_ptr, sz_no_compression))
//            goto error;
          uLongf unc_size = sz_no_compression;
          int unc_ret = uncompress(
//            raw_ptr,   // dest
              ptr_uncompressed_buffer,   // dest
              &unc_size, // dest len: on entry, the value is the size of the dest buffer; on exit, value is the length of uncompressed data.
              zip_ptr,   // src
              zip_sz     // src len
          ); 

          if (unc_ret != Z_OK || unc_size != sz_no_compression) {
              printf("Z_OK = %d, unc_ret = %d / Z_BUF_ERROR = %d, Z_MEM_ERROR = %d, Z_DATA_ERROR = %d\n", Z_OK, unc_ret, Z_BUF_ERROR, Z_MEM_ERROR, Z_DATA_ERROR);
              printf("unc_size = %d, zip_sz = %d\n", unc_size, zip_sz);
              wrong_assumption("uncompress");
              exit(101);
          }

    }

    free (buf);
    buf = NULL;
//  if (raw_ptr == NULL || sz_no_compression == 0)
//      goto error;

     //  --------------------------------------------------------------------- PrimitiveBlock ---------------------------------------------------------------------------------------------------

 // parsing the PrimitiveBlock

//  base  = raw_ptr;
//  start = raw_ptr;
//  stop  = raw_ptr + sz_no_compression - 1;
    cur  = ptr_uncompressed_buffer;
//  start = ptr_uncompressed_buffer;
    end   = ptr_uncompressed_buffer + sz_no_compression - 1;

   #ifdef TQ84_USE_PBF_FIELD_HINTS
    finalize_variant (&variant);
    add_variant_hints (&variant, READOSM_LEN_BYTES,  1);
    add_variant_hints (&variant, READOSM_LEN_BYTES,  2);
    add_variant_hints (&variant, READOSM_VAR_INT32, 17);
    add_variant_hints (&variant, READOSM_VAR_INT32, 18);
    add_variant_hints (&variant, READOSM_VAR_INT64, 19);
    add_variant_hints (&variant, READOSM_VAR_INT64, 20);
   #endif

    while (1) {
       // resetting an empty variant field
          reset_variant (&variant);

          cur = read_pbf_field (cur, end, &variant);
          if (cur == NULL && variant.valid == 0) {
              wrong_assumption("stand");
//            goto error;
          }

//        start = base;
          if (variant.field_id == 1 && variant.type == READOSM_LEN_BYTES) {

             // the StringTable
                if (!parse_string_table ( 
                     &string_table,
                     variant.pointer,
                     variant.pointer + variant.str_len - 1,
                     variant.little_endian_cpu
                   ))
                   wrong_assumption("sta");
//                 goto error;

                array_from_string_table (&string_table);
          }

          if (variant.field_id == 2 && variant.type == READOSM_LEN_BYTES) {

             // the PrimitiveGroup to be parsed
                if (!parse_primitive_group (
                    &string_table, variant.pointer,
                     variant.pointer + variant.str_len - 1,
                     variant.little_endian_cpu
                     // , params
                    ))
                    wrong_assumption("yuh");
//                  goto error;
          }

          if (variant.field_id == 17 && variant.type == READOSM_VAR_INT32) {
             // assumed to be a termination marker (???)
                wrong_assumption("termination marker never reached");
                break;
          }

//        if (base > stop)
          if (cur  > end )
              break;
      }

//  if (buf != NULL)
//      free (buf);

//  if (raw_ptr != NULL)
//      free (raw_ptr);

   #ifdef TQ84_USE_PBF_FIELD_HINTS
    finalize_variant (&variant);
   #endif

    finalize_string_table (&string_table);
    return 1;

  error:
    wrong_assumption("err");
    if (buf != NULL)
        free (buf);

//  if (raw_ptr != NULL)
//      free (raw_ptr);

   #ifdef TQ84_USE_PBF_FIELD_HINTS
    finalize_variant (&variant);
   #endif

    finalize_string_table (&string_table);
    wrong_assumption("neumond");
    return 0;
}

int load_osm_pbf(

    const char* filename_pbf,

    readosm_node_callback     cb_nod,
    readosm_way_callback      cb_way,
    readosm_relation_callback cb_rel
) {

    size_t        rd;
    unsigned char buf[4];
    unsigned int  hdsz;

    verbose_1("load_osm_pbf\n");

    g_cb_nod            = cb_nod;
    g_cb_way            = cb_way;
    g_cb_rel            = cb_rel;
    g_little_endian_cpu = test_endianness();

    g_pbf_file = fopen(filename_pbf, "rb");

    if (g_pbf_file == NULL)
        return READOSM_FILE_NOT_FOUND;

// ----- Header ------------------------------------------------------------------------------------------


//  testing OSMHeader
    if (!read_header_block_v2 ())
        return READOSM_INVALID_PBF_HEADER;

// ----- Data blocks -------------------------------------------------------------------------------------

// 
// the PBF file is internally organized as a collection
// of many subsequent OSMData blocks 

//  cur_uncompressed_buffer_size = 1 * 1000 * 1000;
//  ptr_uncompressed_buffer = malloc(cur_uncompressed_buffer_size);

    set_uncompressed_buffer(1 * 1000 * 1000);

    while(read_osm_data_block_v2()) {
       verbose_1("  iteration (load_osm_pbf)\n");

    }
    free(ptr_uncompressed_buffer);

// -------------------------------------------------------------------------------------------------------


    if (g_pbf_file)
       fclose (g_pbf_file);

    return 0;
}
