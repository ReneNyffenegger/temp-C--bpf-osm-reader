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





typedef struct {

 // a PBF Variant type wrapper

//
// A PBF field is prefixed by a single byte which 
// stores both the FIELD ID (n bits )and the the FIELD type (8-n bites)
// The function read_pbf_field() splits the byte and assigns it to the following
// two bytes:
// 

    unsigned char protobuf_type;
    unsigned char field_id;

    union variant_value_v2 {

        int                 int32_value;
        unsigned int       uint32_value;
        long long           int64_value;
        unsigned long long uint64_value;
        float               float_value;
        double             double_value;
    }
    value;                      // The field value

    size_t                str_len;     // length in bytes [for strings]                --- TODO Should be named nof_bytes or something.
    unsigned char        *pointer;     // pointer to String value
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
           return ptr;

      case READOSM_VAR_UINT32:
           variant->value.uint32_value = value32;
           return ptr;

      case READOSM_VAR_SINT32:
           if ((value32 & 0x00000001) == 0)
               neg = 1;
           else
               neg = -1;

           v32 = (value32 + 1) / 2;
           variant->value.int32_value = v32 * neg;
           return ptr;

      case READOSM_VAR_INT64:
           variant->value.int64_value = (int) value64; // TODO: Should value64 not be cast to signed long long ?
           return ptr;

      case READOSM_VAR_UINT64:
           variant->value.uint64_value = value64;
           return ptr;

      case READOSM_VAR_SINT64:
           if ((value64 & 0x0000000000000001) == 0)
               neg = 1;
           else
               neg = -1;
           v64 = (value64 + 1) / 2;
           variant->value.int64_value = v64 * neg;
           return ptr;
      };
    wrong_assumption("xyz");
    exit(100);
    return NULL;
}

static unsigned char * read_bytes_pbf_field_v2 (unsigned char *start, unsigned char *stop, pbf_field_v2 *variant) {
// 
// attempting to read some bytes from PBF
// Strings and alike are encoded in PBF using a two steps approach:
// - an INT32 field declares the expected length
// - then the string (no terminating NULL char) follows

    unsigned char *ptr = start;
    pbf_field varlen;
    unsigned int len;

 /* initializing an empty variant field (length) */
    init_variant (&varlen, g_little_endian_cpu);
    varlen.type = READOSM_VAR_UINT32;

    ptr = read_integer_pbf_field (ptr, stop, &varlen);

    if (varlen.valid) {
          len = varlen.value.uint32_value;
          if ((ptr + len - 1) > stop) {
              wrong_assumption("ptr+len-1 > stop");
              return NULL;
          }

          variant->pointer = ptr;
          variant->str_len = len;
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



int block_size_v2(char* name) {

    unsigned int sz = blob_size();
    if (!sz) return 0;

    int hdsz      = 0;
    size_t        rd;


    unsigned char *buf   = malloc (sz);
    unsigned char *cur   = buf;
    unsigned char *end   = buf + sz - 1;

    if (buf == NULL) {
        wrong_assumption("buf");
    }

    rd = fread (buf, 1, sz, g_pbf_file);
    if (rd != sz) {
       wrong_assumption("rd == sz");
    }

    pbf_field_v2    fld_block_name;

    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld_block_name);

    if (fld_block_name.field_id != 1) {
       printf("field id = %d\n", fld_block_name.field_id);
       wrong_assumption("field id in block_size");
    }
    if (fld_block_name.protobuf_type != PROTOBUF_TYPE_LEN) {
       wrong_assumption("PROTOBUF_TYPE_LEN");
    }
    cur = read_bytes_pbf_field_v2 (cur, end, &fld_block_name);

    if (fld_block_name.str_len == 9) {

          verbose_1("      field_id == 1\n");
          if (memcmp (fld_block_name.pointer, name, strlen(name))) {
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
    verbose_1("      ret = %d\n", read);

    free(buf);

    return hdsz;
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

    int hdsz = block_size_v2("OSMHeader");
    if (!hdsz) {
        wrong_assumption("ok header, hdsz 2");
    }
 //
 // Just SKIP OVER the rest of the header buffer!
 //
    fseek(g_pbf_file, hdsz, SEEK_CUR);
    return 1;
 //
 // alternatively, create a buffer and parse it and deallocate it.
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
   
// exit(100); // TQ84 - remove me
    verbose_1("       returning from read_header_block_v2\n");
    return 1;

}


#define NOF_PACKED_ELEMS 8000

typedef struct {
   int count;
   unsigned int data[NOF_PACKED_ELEMS];
} packed_uint32_v2;

typedef struct {
   int count;
   unsigned long long data[NOF_PACKED_ELEMS];
} packed_uint64_v2;

typedef struct {
   int count;
   signed long long data[NOF_PACKED_ELEMS];
} packed_sint64_v2;


static void parse_uint32_packed_v2(
// readosm_uint32_packed * packed,
   packed_uint32_v2 * packed,
   unsigned char *start,
   unsigned char *stop
) {

 // parsing a uint32 packed object
    unsigned char *ptr = start;
//q pbf_field variant;
    pbf_field_v2 fld;

 // initializing an empty variant field (length)
//q init_variant (&variant, little_endian_cpu);
//q variant.type = READOSM_VAR_UINT32;

//q while (1) {
    while (packed->count < NOF_PACKED_ELEMS) {

//q    ptr = read_integer_pbf_field    (start, stop, &variant);
       ptr = read_integer_pbf_field_v2 (start, stop, READOSM_VAR_UINT32, &fld);
//     if (variant.valid) {
//q          append_uint32_packed (packed, variant.value.uint32_value);
             packed->data[packed->count] = fld.value.uint32_value;
             packed->count++;
             if (ptr > stop) {
                 return;
//               break;
             }

             start = ptr;
//           continue;
//q    }
//q    return 0;
    }
//q return 1;
}


static void parse_sint64_packed_v2 (
//  readosm_int64_packed   *packed,
    packed_sint64_v2       *packed,
    unsigned char          *start,
    unsigned char          *stop
)
{
 //
 // parsing a sint64 packed object
 //
    unsigned char *ptr = start;
    pbf_field_v2 fld;

 //
 // initializing an empty variant field (length) */
 //
//  init_variant (&variant, little_endian_cpu);
//  variant.type = READOSM_VAR_SINT64;

//  int pos = 0;
  
    packed->count = 0;
    while (packed->count < NOF_PACKED_ELEMS) {

       ptr = read_integer_pbf_field_v2 (ptr, stop, READOSM_VAR_SINT64, &fld);

//q    ptr = read_integer_pbf_field (start, stop, &variant);

//     if (variant.valid) {
//q       append_int64_packed (packed, variant.value.int64_value);
          packed->data[packed->count] = fld.value.int64_value;
          packed->count ++;

          if (ptr > stop) {
              return;
//            break;
          }

//        start = ptr;
//        continue;
//     }
//     return 0;

    }
//  return 1;
}

static void parse_uint64_packed_v2 (
//  readosm_int64_packed   *packed,
    packed_uint64_v2       *packed,
    unsigned char          *start,
    unsigned char          *stop
)
{
 //
 // parsing a sint64 packed object
 //
    unsigned char *ptr = start;
    pbf_field_v2 fld;

 //
 // initializing an empty variant field (length) */
 //
//  init_variant (&variant, little_endian_cpu);
//  variant.type = READOSM_VAR_SINT64;

//  int pos = 0;
  
    packed->count = 0;
    while (packed->count < NOF_PACKED_ELEMS) {

       ptr = read_integer_pbf_field_v2 (ptr, stop, READOSM_VAR_UINT64, &fld);

//q    ptr = read_integer_pbf_field (start, stop, &variant);

//     if (variant.valid) {
//q       append_int64_packed (packed, variant.value.int64_value);
          packed->data[packed->count] = fld.value.uint64_value;
          packed->count ++;

          if (ptr > stop) {
              return;
//            break;
          }

//        start = ptr;
//        continue;
//     }
//     return 0;

    }
//  return 1;
}


static int parse_pbf_nodes_v2 (
                 readosm_string_table * strings,
                 unsigned char *start,
                 unsigned char *end,
                 char           little_endian_cpu
)
{
/* 
 / Attempting to parse a valid PBF DenseNodes 
 /
 / Remark: a PBF DenseNodes block consists of five byte blocks:
 / - ids
 / - DenseInfos
 / - longitudes
 / - latitudes
 / - packed-keys (*)
 /
 / Each block in turn contains an array of INT values;
 / and individual values are usually encoded as DELTAs,
 / i.e. differences respect the immediately preceding value.
 /
 / (*) packed keys actually are encoded as arrays of index
 / to StringTable entries.
 / alternatively we have a key-index and then a value-index;
 / any 0 value means that the current Node stops: next index
 / will be a key-index for the next Node item
*/

    verbose_1("      parse_pbf_nodes_v2\n");

//  pbf_field       variant;
    pbf_field_v2    fld;

    unsigned char *cur = start;

    readosm_uint32_packed  packed_keys;
//  packed_uint32_v2       packed_keys_v2;
//q readosm_int64_packed   packed_ids;
    packed_sint64_v2       packed_ids_v2;   // Nodes are signed!
    readosm_int64_packed   packed_lats;
    readosm_int64_packed   packed_lons;
    readosm_packed_infos   packed_infos;

    readosm_internal_node *nodes = NULL;
    int nd_count                 = 0;
//  int valid                    = 0;
    int fromPackedInfos          = 0;

 // initializing empty packed objects
    init_uint32_packed (&packed_keys);
//q init_int64_packed  (&packed_ids);
    init_int64_packed  (&packed_lats);
    init_int64_packed  (&packed_lons);
    init_packed_infos  (&packed_infos);



    while (1) {

          cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld);
      //
      //  It seems that each 'dense node' has the field ids 1, 5, 8, 9  and 10 exactly once (and in this order).
      //  
//q printf("        field_id = %d\n", fld.field_id);
          verbose_1("        field_id = %d\n", fld.field_id);


//q       if      (fld.field_id ==  1 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* NODE IDs    */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_sint64_packed    (&packed_ids    , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error; array_from_int64_packed (&packed_ids);  }
          if      (fld.field_id ==  1 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* NODE IDs    */ cur = read_bytes_pbf_field_v2 (cur, end, &fld);      parse_sint64_packed_v2 (&packed_ids_v2 , fld.pointer, fld.pointer + fld.str_len - 1                     );                                                     }
          else if (fld.field_id ==  5 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* DenseInfos  */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_pbf_node_infos   (&packed_infos  , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error;                                         }
          else if (fld.field_id ==  8 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* latitudes   */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_sint64_packed    (&packed_lats   , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error; array_from_int64_packed (&packed_lats); }
          else if (fld.field_id ==  9 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* longitudes  */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_sint64_packed    (&packed_lons   , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error; array_from_int64_packed (&packed_lons); }
          else if (fld.field_id == 10 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* packes-keys */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_uint32_packed    (&packed_keys   , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error; array_from_uint32_packed(&packed_keys); }
//q       else if (fld.field_id == 10 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* packes-keys */ cur = read_bytes_pbf_field_v2 (cur, end, &fld);      parse_uint32_packed_v2 (&packed_keys_v2, fld.pointer, fld.pointer + fld.str_len - 1                     );                                                     }
          else wrong_assumption("dense node");

          if (cur > end)
              break;
    }

    printf("packed_ids.count = %d, packed_keys count = %d\n", packed_ids_v2.count, packed_keys.count);

//
//  https://wiki.openstreetmap.org/wiki/PBF_Format (2024-05-07):
//       A block may contain any number of entities, as long as the size limits for
//       a block are obeyed. It will result in small file sizes if you pack as many
//       entities as possible into each block. However, for simplicity, certain
//       programs (e.g. osmosis 0.38) limit the number of entities in each block to
//       8000 when writing PBF format.
//
// printf("packed_ids.count = %d\n", packed_ids.count);

//q if (packed_ids.count == packed_lats.count &&
//q     packed_ids.count == packed_lons.count
    if (packed_ids_v2.count == packed_lats.count &&
        packed_ids_v2.count == packed_lons.count
       )
    {
       // not using PackedInfos
//        valid = 1;
    }
    else {
       wrong_assumption("count of lats <> count of longs");
    }
//qif (   packed_ids.count == packed_lats.count
//q    && packed_ids.count == packed_lons.count
//q    && packed_ids.count == packed_infos.ver_count
//q    && packed_ids.count == packed_infos.tim_count
//q    && packed_ids.count == packed_infos.cng_count
//q    && packed_ids.count == packed_infos.uid_count
//q    && packed_ids.count == packed_infos.usr_count)
   if (   packed_ids_v2.count == packed_lats.count
       && packed_ids_v2.count == packed_lons.count
       && packed_ids_v2.count == packed_infos.ver_count
       && packed_ids_v2.count == packed_infos.tim_count
       && packed_ids_v2.count == packed_infos.cng_count
       && packed_ids_v2.count == packed_infos.uid_count
       && packed_ids_v2.count == packed_infos.usr_count)
      {
       // from PackedInfos
//        valid           = 1;
          fromPackedInfos = 1;
      }
      else {
          wrong_assumption("counts");
      }

//  if (!valid) {
//      wrong_assumption("valid count");
//      goto error;
//  }

//  else {
    //
    //  all right, we now have the same item count anywhere
    //  we can now go further away attempting to reassemble
    //  individual Nodes 
    //
    readosm_internal_node *nd;
    int i;
    int i_keys = 0;
    long long delta_id = 0;
    long long delta_lat = 0;
    long long delta_lon = 0;
    int max_nodes;
    int base = 0;
//q nd_count = packed_ids.count;
    nd_count = packed_ids_v2.count;

    while (base < nd_count) {

       // processing about 1024 nodes at each time
          max_nodes = MAX_NODES;

          if ((nd_count - base) < MAX_NODES)
              max_nodes = nd_count - base;

          nodes = malloc (sizeof (readosm_internal_node) * max_nodes);

          for (i = 0; i < max_nodes; i++) {
             // initializing an array of empty internal Nodes
                nd = nodes + i;
                init_internal_node (nd);
          }
          for (i = 0; i < max_nodes; i++) {
                /* reassembling internal Nodes */
                const char *key   = NULL;
                const char *value = NULL;
                time_t xtime;
                struct tm *times;
                int s_id;
                nd = nodes + i;
//q             delta_id  += *(packed_ids.values     + base + i);
                delta_id  += *(packed_ids_v2.data    + base + i);
                delta_lat += *(packed_lats.values    + base + i);
                delta_lon += *(packed_lons.values    + base + i);
                nd->id = delta_id;
                printf("  nd->id = %d\n", nd->id);
            /* latitudes and longitudes require to be rescaled as DOUBLEs */
                nd->latitude  = delta_lat / 10000000.0;
                nd->longitude = delta_lon / 10000000.0;

                if (fromPackedInfos) {
                      nd->version = *(packed_infos.versions   + base + i);
                      printf("  nd->version = %d\n", nd->version);
                      xtime       = *(packed_infos.timestamps + base + i);
                      times       = gmtime (&xtime);

                      if (times) {
// printf("times\n");
                         // formatting Timestamps
                            char buf[64];
                            int len;
                            sprintf (buf,
                                     "%04d-%02d-%02dT%02d:%02d:%02dZ",
                                     times->tm_year + 1900,
                                     times->tm_mon  +    1,
                                     times->tm_mday,
                                     times->tm_hour,
                                     times->tm_min,
                                     times->tm_sec);

                            if (nd->timestamp)
                                free (nd->timestamp);

                            len = strlen (buf);
                            nd->timestamp = malloc (len + 1);
                            strcpy (nd->timestamp, buf);
                        }

                      nd->changeset = *(packed_infos.changesets + base + i);

                      if (*(packed_infos.uids + base + i) >= 0)
                          nd->uid = *(packed_infos.uids + base + i);

                      s_id = *(packed_infos.users + base + i);

                      if (s_id > 0) {
                         /* retrieving user-names as strings (by index) */
                            pbf_string_table_elem *s_ptr =
                                *(strings->strings + s_id);
                            int len = strlen (s_ptr->string);
                            if (nd->user != NULL)
                                free (nd->user);

                            if (len > 0) {
                                  nd->user = malloc (len + 1);
                                  strcpy (nd->user, s_ptr->string);
                              }
                        }
                }
                else {
                }

                for (; i_keys < packed_keys.count; i_keys++) {
//              for (int i_keys = 0; i_keys < packed_keys_v2.count; i_keys++) {
//                 printf("i_keys = %d\n", i_keys);
                   // decoding packed-keys
                      int is = *(packed_keys.values  + i_keys);
//                    int is = *(packed_keys_v2.data + i_keys);

                      if (is == 0) {
                            /* next Node */
                            i_keys++;
// printf("break\n");
                            break;
                      }

                      if (key == NULL) {
                            pbf_string_table_elem *s_ptr =
                                *(strings->strings + is);

                            key = s_ptr->string;
// printf("key = %s\n", key);
                      }
                      else {
                            pbf_string_table_elem *s_ptr = *(strings->strings + is);

                            value = s_ptr->string;
// printf("value = %s\n", value);
                            append_tag_to_node (nd, key, value);
                            key = NULL;
                            value = NULL;
                        }
                  }
            }
          base += max_nodes;

          /* processing each Node in the block */
//        if (params->node_callback != NULL && params->stop == 0) {
                int ret;
                readosm_internal_node *nd;
                int i;
                for (i = 0; i < max_nodes; i++) {
                      nd = nodes + i;
                      ret = call_node_callback (g_cb_nod, nd);

                      if (ret != READOSM_OK) {
                            exit(42);
                            break;
                        }
                  }
//          }

          /* memory cleanup: destroying Nodes */
          if (nodes != NULL) {

                readosm_internal_node *nd;
                int i;
                for (i = 0; i < max_nodes; i++)
                  {
                      nd = nodes + i;
                      destroy_internal_node (nd);
                  }
                free (nodes);
            }
      }
//    }

/* memory cleanup */
    finalize_uint32_packed(&packed_keys);
//
//q finalize_int64_packed (&packed_ids);
    finalize_int64_packed (&packed_lats);
    finalize_int64_packed (&packed_lons);
    finalize_packed_infos (&packed_infos);
    return 1;

  error:
    finalize_uint32_packed(&packed_keys);
//q finalize_int64_packed (&packed_ids);
    finalize_int64_packed (&packed_lats);
    finalize_int64_packed (&packed_lons);
    finalize_packed_infos (&packed_infos);

    if (nodes != NULL) {
          readosm_internal_node *nd;
          int i;
          for (i = 0; i < nd_count; i++) {
                nd = nodes + i;
                destroy_internal_node (nd);
          }
          free (nodes);
    }
    return 0;
}

static int parse_pbf_nodes_v3 (
                 readosm_string_table * strings,
                 unsigned char * const start,
                 unsigned char * const end
//               char           little_endian_cpu
)
{
/* 
 / Attempting to parse a valid PBF DenseNodes 
 /
 / Remark: a PBF DenseNodes block consists of five byte blocks:
 / - ids
 / - DenseInfos
 / - longitudes
 / - latitudes
 / - packed-keys (*)
 /
 / Each block in turn contains an array of INT values;
 / and individual values are usually encoded as DELTAs,
 / i.e. differences respect the immediately preceding value.
 /
 / (*) packed keys actually are encoded as arrays of index
 / to StringTable entries.
 / alternatively we have a key-index and then a value-index;
 / any 0 value means that the current Node stops: next index
 / will be a key-index for the next Node item
*/

    verbose_1("      parse_pbf_nodes_v2\n");

//  pbf_field       variant;
    pbf_field_v2    fld;


    readosm_uint32_packed  packed_keys;
//  packed_uint32_v2       packed_keys_v2;
//q readosm_int64_packed   packed_ids;
    packed_sint64_v2       packed_ids_v2;   // Nodes are signed!
    readosm_int64_packed   packed_lats;
    readosm_int64_packed   packed_lons;
    readosm_packed_infos   packed_infos;

    readosm_internal_node *nodes = NULL;
    int nd_count                 = 0;
//  int valid                    = 0;
    int fromPackedInfos          = 0;

 // initializing empty packed objects
    init_uint32_packed (&packed_keys);
//q init_int64_packed  (&packed_ids);
    init_int64_packed  (&packed_lats);
    init_int64_packed  (&packed_lons);
    init_packed_infos  (&packed_infos);

    unsigned char *cur = start;

    unsigned char *cur_node_ids;     unsigned char* end_node_ids;
    unsigned char *cur_dense_infos;  unsigned char* end_dense_infos;
    unsigned char *cur_latitudes;    unsigned char* end_latitudes;
    unsigned char *cur_longitudes;   unsigned char* end_longitudes;
    unsigned char *cur_packed_keys;  unsigned char* end_packed_keys;

    printf("start = %p\n", start);
    
    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld); if (fld.field_id !=  1 || fld.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld");} cur = read_bytes_pbf_field_v2 (cur, end, &fld); cur_node_ids     = fld.pointer; end_node_ids      = cur_node_ids    + fld.str_len -1;
    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld); if (fld.field_id !=  5 || fld.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld");} cur = read_bytes_pbf_field_v2 (cur, end, &fld); cur_dense_infos  = fld.pointer; end_dense_infos   = cur_dense_infos + fld.str_len -1;
    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld); if (fld.field_id !=  8 || fld.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld");} cur = read_bytes_pbf_field_v2 (cur, end, &fld); cur_latitudes    = fld.pointer; end_latitudes     = cur_latitudes   + fld.str_len -1;
    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld); if (fld.field_id !=  9 || fld.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld");} cur = read_bytes_pbf_field_v2 (cur, end, &fld); cur_longitudes   = fld.pointer; end_longitudes    = cur_longitudes  + fld.str_len -1;
    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld); if (fld.field_id != 10 || fld.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld");} cur = read_bytes_pbf_field_v2 (cur, end, &fld); cur_packed_keys  = fld.pointer; end_packed_keys   = cur_packed_keys + fld.str_len -1;

    unsigned char *cur_versions   , *end_versions;
    unsigned char *cur_timestamps , *end_timestamps;
    unsigned char *cur_changesets , *end_changesets;
    unsigned char *cur_uids       , *end_uids;
    unsigned char *cur_unames     , *end_unames;

    cur_dense_infos = read_pbf_field_v2_protobuf_type_and_field(cur_dense_infos, &fld); if (fld.field_id != 1 || fld.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld");} cur_dense_infos = read_bytes_pbf_field_v2 (cur_dense_infos, end_dense_infos, &fld); cur_versions    = fld.pointer; end_versions    = cur_versions   + fld.str_len -1;
    cur_dense_infos = read_pbf_field_v2_protobuf_type_and_field(cur_dense_infos, &fld); if (fld.field_id != 2 || fld.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld");} cur_dense_infos = read_bytes_pbf_field_v2 (cur_dense_infos, end_dense_infos, &fld); cur_timestamps  = fld.pointer; end_timestamps  = cur_timestamps + fld.str_len -1;
    cur_dense_infos = read_pbf_field_v2_protobuf_type_and_field(cur_dense_infos, &fld); if (fld.field_id != 3 || fld.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld");} cur_dense_infos = read_bytes_pbf_field_v2 (cur_dense_infos, end_dense_infos, &fld); cur_changesets  = fld.pointer; end_changesets  = cur_changesets + fld.str_len -1;
    cur_dense_infos = read_pbf_field_v2_protobuf_type_and_field(cur_dense_infos, &fld); if (fld.field_id != 4 || fld.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld");} cur_dense_infos = read_bytes_pbf_field_v2 (cur_dense_infos, end_dense_infos, &fld); cur_uids        = fld.pointer; end_uids        = cur_uids       + fld.str_len -1;
    cur_dense_infos = read_pbf_field_v2_protobuf_type_and_field(cur_dense_infos, &fld); if (fld.field_id != 5 || fld.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld");} cur_dense_infos = read_bytes_pbf_field_v2 (cur_dense_infos, end_dense_infos, &fld); cur_unames      = fld.pointer; end_unames      = cur_unames     + fld.str_len -1;



//  iterate over each node

    int cnt_nodes = 0;
    signed long long cur_node_id = 0;

    double lat = 0.0;
    double lon = 0.0;
    int    tim = 0;
    long long changeset = 0;
    int       uid       = 0;
    int       uname     = 0;

    while (cur_node_ids <= end_node_ids) {

       cur_node_ids = read_integer_pbf_field_v2 (cur_node_ids, end_node_ids, READOSM_VAR_SINT64, &fld);
       signed long long δ_node_id = fld.value.int64_value;

       cur_node_id   += δ_node_id;

       double δ_lat;
       double δ_lon;

       cur_latitudes  = read_integer_pbf_field_v2 (cur_latitudes , end_latitudes , READOSM_VAR_SINT64, &fld);
       δ_lat = fld.value.int64_value / 10000000.0;
       lat += δ_lat;

       cur_longitudes = read_integer_pbf_field_v2 (cur_longitudes, end_longitudes, READOSM_VAR_SINT64, &fld);
       δ_lon = fld.value.int64_value / 10000000.0;
       lon += δ_lon;

       cur_versions = read_integer_pbf_field_v2(cur_versions, end_versions, READOSM_VAR_UINT32, &fld);
       unsigned int version = fld.value.uint32_value;


   //  -----------------------------------------------------------------------------------------------------

       cur_timestamps = read_integer_pbf_field_v2(cur_timestamps, end_timestamps, READOSM_VAR_SINT32, &fld);
       int    δ_tim = fld.value.int32_value;
//     printf("δ_tim = %11d\n", δ_tim);
       tim += δ_tim;

  //   printf("tim=%d, δ_tim = %d\n", tim, δ_tim);

       const time_t tim_ = tim;
//     printf("xtime = %d\n", xtime);
       char ts_buf[64];
       struct tm *times = gmtime (&tim_);
//     printf("times = %p\n", times);
                if (times) {
                      int len;
                      sprintf (ts_buf, "%04d-%02d-%02dT%02d:%02d:%02dZ",
                               times->tm_year + 1900, times->tm_mon + 1,
                               times->tm_mday, times->tm_hour, times->tm_min,
                               times->tm_sec);
//                    if (way->timestamp)
//                        free (way->timestamp);
//                    len = strlen (ts_buf);
//                    way->timestamp = malloc (len + 1);
//                    strcpy (way->timestamp, buf);
                  }
                  else {

                     sprintf(ts_buf, "%s", "?");
                  }

   //  -----------------------------------------------------------------------------------------------------
   //
   //      Changesets
   //         Note: Changesets need not be monotonously increasing vis a vis a node's history.
   //               It's also possible for different versions to have the same changeset.
   //               See for example node 854251
   //             


       cur_changesets = read_integer_pbf_field_v2(cur_changesets, end_changesets, READOSM_VAR_SINT64, &fld);
       signed long long    δ_changeset = fld.value.int64_value;
       changeset += δ_changeset;

   //  -----------------------------------------------------------------------------------------------------

       cur_uids = read_integer_pbf_field_v2(cur_uids, end_uids, READOSM_VAR_SINT32, &fld);
       signed int δ_uid = fld.value.int32_value;
       uid += δ_uid;

       cur_unames = read_integer_pbf_field_v2(cur_unames, end_unames, READOSM_VAR_SINT32, &fld);
       signed int δ_uname = fld.value.int32_value;
       uname += δ_uname;

       char *uname_str = (*(strings -> strings + uname))->string;

       printf("node_id = %llu of %s @ %f, %f [%3d | %13llu] by %30s (%10d)\n", cur_node_id, ts_buf, lat, lon, version, changeset, uname_str, uid);
//     printf("node_id = %llu       @ %f, %f [%3d]\n", cur_node_id        , lat, lon, version);


    //
    // Find key value pairs
    //
       while (1) {
          cur_packed_keys = read_integer_pbf_field_v2(cur_packed_keys, end_packed_keys, READOSM_VAR_UINT32, &fld);
          unsigned int str_id_key = fld.value.int32_value;

          if (!str_id_key ) { break; }

          cur_packed_keys = read_integer_pbf_field_v2(cur_packed_keys, end_packed_keys, READOSM_VAR_UINT32, &fld);
          unsigned int str_id_val = fld.value.int32_value;

          char *key, *val;

          key = (*(strings -> strings + str_id_key))->string;
          val = (*(strings -> strings + str_id_val))->string;

//        printf("  %s = %s\n", key, val);
       }

// HERE!       while (1) {
// HERE!
// HERE!          cur_dense_infos = read_pbf_field_v2_protobuf_type_and_field(cur_dense_infos,  &fld);
// HERE!          if (fld.field_id != 1 || fld.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("xxx"); }
// HERE!
// HERE!          unsigned int version;
// HERE!          read_integer_pbf_field_v2(cur_dense_infos, end_dense_infos, READOSM_VAR_UINT32, &fld);
// HERE!          printf("version = %d\n", fld.value.uint32_value);
// HERE!
// HERE!
// HERE!
// HERE!// -        if (variant.field_id == 1 && variant.type == READOSM_LEN_BYTES) { /* versions: *not* delta encoded */                                         if (!parse_uint32_packed (&packed_u32, variant.pointer, variant.pointer + variant.str_len - 1, variant.little_endian_cpu)) goto error; count = 0; pu32 = packed_u32.first; while (pu32) { count++; pu32 = pu32->next; } packed_infos->ver_count = count; if (packed_infos->versions   != NULL) { free (packed_infos->versions  ); packed_infos->versions   = NULL; } if (count > 0) { packed_infos->versions   = malloc (sizeof (int      ) * count); count = 0; pu32 = packed_u32.first; while (pu32) {              *(packed_infos->versions + count) = pu32->value;     count++; pu32 = pu32->next;} } reset_uint32_packed (&packed_u32); }
// HERE!// -        if (variant.field_id == 2 && variant.type == READOSM_LEN_BYTES) { /* timestamps: delta encoded */                        int       delta = 0; if (!parse_sint32_packed (&packed_32 , variant.pointer, variant.pointer + variant.str_len - 1, variant.little_endian_cpu)) goto error; count = 0; p32  = packed_32.first ; while (p32 ) { count++; p32  = p32->next;  } packed_infos->tim_count = count; if (packed_infos->timestamps != NULL) { free (packed_infos->timestamps); packed_infos->timestamps = NULL; } if (count > 0) { packed_infos->timestamps = malloc (sizeof (int      ) * count); count = 0; p32  = packed_32.first ; while (p32)  { delta += p32->value; *(packed_infos->timestamps + count) = delta; count++; p32  = p32->next; } } reset_int32_packed  (&packed_32);  }
// HERE!// -        if (variant.field_id == 3 && variant.type == READOSM_LEN_BYTES) { /* changesets: delta encoded */                        long long delta = 0; if (!parse_sint64_packed (&packed_64 , variant.pointer, variant.pointer + variant.str_len - 1, variant.little_endian_cpu)) goto error; count = 0; p64  = packed_64.first ; while (p64 ) { count++; p64  = p64->next;  } packed_infos->cng_count = count; if (packed_infos->changesets != NULL) { free (packed_infos->changesets); packed_infos->changesets = NULL; } if (count > 0) { packed_infos->changesets = malloc (sizeof (long long) * count); count = 0; p64  = packed_64.first ; while (p64)  { delta += p64->value; *(packed_infos->changesets + count) = delta; count++; p64  = p64->next; } } reset_int64_packed  (&packed_64);  }
// HERE!// -        if (variant.field_id == 4 && variant.type == READOSM_LEN_BYTES) { /* uids: delta encoded */                              int       delta = 0; if (!parse_sint32_packed (&packed_32 , variant.pointer, variant.pointer + variant.str_len - 1, variant.little_endian_cpu)) goto error; count = 0; p32  = packed_32.first ; while (p32 ) { count++; p32  = p32->next;  } packed_infos->uid_count = count; if (packed_infos->uids       != NULL) { free (packed_infos->uids      ); packed_infos->uids       = NULL; } if (count > 0) { packed_infos->uids       = malloc (sizeof (int      ) * count); count = 0; p32  = packed_32.first ; while (p32)  { delta += p32->value; *(packed_infos->uids       + count) = delta; count++; p32  = p32->next; } } reset_int32_packed  (&packed_32);  }
// HERE!// -        if (variant.field_id == 5 && variant.type == READOSM_LEN_BYTES) { /* user-names: delta encoded (index to StringTable) */ int       delta = 0; if (!parse_sint32_packed (&packed_32 , variant.pointer, variant.pointer + variant.str_len - 1, variant.little_endian_cpu)) goto error; count = 0; p32  = packed_32.first ; while (p32 ) { count++; p32  = p32->next;  } packed_infos->usr_count = count; if (packed_infos->users      != NULL) { free (packed_infos->users     ); packed_infos->users      = NULL; } if (count > 0) { packed_infos->users      = malloc (sizeof (int      ) * count); count = 0; p32  = packed_32.first ; while (p32)  { delta += p32->value; *(packed_infos->users      + count) = delta; count++; p32  = p32->next; } } reset_int32_packed  (&packed_32);  }
// HERE!// -        if (base > stop)
// HERE!
// HERE!
// HERE!//        printf("   fld.field_id = %d\n", fld.field_id);
// HERE!//        cur_dense_infos = read_integer_pbf_field_v2(cur_dense_infos, end_dense_infos, READOSM_VAR_UINT32, &fld);
// HERE!          
// HERE!          break;
// HERE!
// HERE!       }


       

       cnt_nodes ++;
    }

    printf("count_nodes = %d\n", cnt_nodes);




//  parse_sint64_packed_v2 (&packed_ids_v2 , fld.pointer, fld.pointer + fld.str_len - 1                     );                                                     }

//r          else if (fld.field_id ==  5 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* DenseInfos  */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_pbf_node_infos   (&packed_infos  , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error;                                         }
//r          else if (fld.field_id ==  8 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* latitudes   */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_sint64_packed    (&packed_lats   , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error; array_from_int64_packed (&packed_lats); }
//r          else if (fld.field_id ==  9 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* longitudes  */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_sint64_packed    (&packed_lons   , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error; array_from_int64_packed (&packed_lons); }
//r          else if (fld.field_id == 10 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* packes-keys */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_uint32_packed    (&packed_keys   , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error; array_from_uint32_packed(&packed_keys); }
//r//q       else if (fld.field_id == 10 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* packes-keys */ cur = read_bytes_pbf_field_v2 (cur, end, &fld);      parse_uint32_packed_v2 (&packed_keys_v2, fld.pointer, fld.pointer + fld.str_len - 1                     );                                                     }
//r
//r    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld_dense_infos);
//r    if (fld_dense_infos.field_id !=  5 || fld_dense_infos.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld_dense_infos");}
//r
//r    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld_latitudes);
//r    if (fld_latitudes.field_id   !=  8 || fld_latitudes.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld_latitudes");}
//r
//r    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld_longitudes);
//r    if (fld_longitudes.field_id  !=  9 || fld_longitudes.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld_longitudes");}
//r
//r    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld_packed_keys);
//r    if (fld_packed_keys.field_id != 10 || fld_packed_keys.protobuf_type != PROTOBUF_TYPE_LEN) { wrong_assumption("fld_packed_keys");}
//r
//r
//r    return 0;
    
//r    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld);
//r    
//r
//r    while (1) {
//r
//r          cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld);
//r      //
//r      //  It seems that each 'dense node' has the field ids 1, 5, 8, 9  and 10 exactly once (and in this order).
//r      //  
//r          verbose_1("        field_id = %d\n", fld.field_id);
//r
//r
//r//q       if      (fld.field_id ==  1 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* NODE IDs    */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_sint64_packed    (&packed_ids    , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error; array_from_int64_packed (&packed_ids);  }
//r          if      (fld.field_id ==  1 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* NODE IDs    */ cur = read_bytes_pbf_field_v2 (cur, end, &fld);      parse_sint64_packed_v2 (&packed_ids_v2 , fld.pointer, fld.pointer + fld.str_len - 1                     );                                                     }
//r          else if (fld.field_id ==  5 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* DenseInfos  */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_pbf_node_infos   (&packed_infos  , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error;                                         }
//r          else if (fld.field_id ==  8 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* latitudes   */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_sint64_packed    (&packed_lats   , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error; array_from_int64_packed (&packed_lats); }
//r          else if (fld.field_id ==  9 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* longitudes  */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_sint64_packed    (&packed_lons   , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error; array_from_int64_packed (&packed_lons); }
//r          else if (fld.field_id == 10 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* packes-keys */ cur = read_bytes_pbf_field_v2 (cur, end, &fld); if (!parse_uint32_packed    (&packed_keys   , fld.pointer, fld.pointer + fld.str_len - 1, g_little_endian_cpu)) goto error; array_from_uint32_packed(&packed_keys); }
//r//q       else if (fld.field_id == 10 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { /* packes-keys */ cur = read_bytes_pbf_field_v2 (cur, end, &fld);      parse_uint32_packed_v2 (&packed_keys_v2, fld.pointer, fld.pointer + fld.str_len - 1                     );                                                     }
//r          else wrong_assumption("dense node");
//r
//r          if (cur > end)
//r              break;
//r    }
//r
//r  //  printf("packed_ids.count = %d, packed_keys count = %d\n", packed_ids_v2.count, packed_keys.count);
//r
//r//
//r//  https://wiki.openstreetmap.org/wiki/PBF_Format (2024-05-07):
//r//       A block may contain any number of entities, as long as the size limits for
//r//       a block are obeyed. It will result in small file sizes if you pack as many
//r//       entities as possible into each block. However, for simplicity, certain
//r//       programs (e.g. osmosis 0.38) limit the number of entities in each block to
//r//       8000 when writing PBF format.
//r//
//r// printf("packed_ids.count = %d\n", packed_ids.count);
//r
//r//q if (packed_ids.count == packed_lats.count &&
//r//q     packed_ids.count == packed_lons.count
//r    if (packed_ids_v2.count == packed_lats.count &&
//r        packed_ids_v2.count == packed_lons.count
//r       )
//r    {
//r       // not using PackedInfos
//r//        valid = 1;
//r    }
//r    else {
//r       wrong_assumption("count of lats <> count of longs");
//r    }
//r//qif (   packed_ids.count == packed_lats.count
//r//q    && packed_ids.count == packed_lons.count
//r//q    && packed_ids.count == packed_infos.ver_count
//r//q    && packed_ids.count == packed_infos.tim_count
//r//q    && packed_ids.count == packed_infos.cng_count
//r//q    && packed_ids.count == packed_infos.uid_count
//r//q    && packed_ids.count == packed_infos.usr_count)
//r   if (   packed_ids_v2.count == packed_lats.count
//r       && packed_ids_v2.count == packed_lons.count
//r       && packed_ids_v2.count == packed_infos.ver_count
//r       && packed_ids_v2.count == packed_infos.tim_count
//r       && packed_ids_v2.count == packed_infos.cng_count
//r       && packed_ids_v2.count == packed_infos.uid_count
//r       && packed_ids_v2.count == packed_infos.usr_count)
//r      {
//r       // from PackedInfos
//r//        valid           = 1;
//r          fromPackedInfos = 1;
//r      }
//r      else {
//r          wrong_assumption("counts");
//r      }
//r
//r//  if (!valid) {
//r//      wrong_assumption("valid count");
//r//      goto error;
//r//  }
//r
//r//  else {
//r    //
//r    //  all right, we now have the same item count anywhere
//r    //  we can now go further away attempting to reassemble
//r    //  individual Nodes 
//r    //
//r    readosm_internal_node *nd;
//r    int i;
//r    int i_keys = 0;
//r    long long delta_id = 0;
//r    long long delta_lat = 0;
//r    long long delta_lon = 0;
//r    int max_nodes;
//r    int base = 0;
//r//q nd_count = packed_ids.count;
//r    nd_count = packed_ids_v2.count;
//r
//r    while (base < nd_count) {
//r
//r       // processing about 1024 nodes at each time
//r          max_nodes = MAX_NODES;
//r
//r          if ((nd_count - base) < MAX_NODES)
//r              max_nodes = nd_count - base;
//r
//r          nodes = malloc (sizeof (readosm_internal_node) * max_nodes);
//r
//r          for (i = 0; i < max_nodes; i++) {
//r             // initializing an array of empty internal Nodes
//r                nd = nodes + i;
//r                init_internal_node (nd);
//r          }
//r          for (i = 0; i < max_nodes; i++) {
//r                /* reassembling internal Nodes */
//r                const char *key   = NULL;
//r                const char *value = NULL;
//r                time_t xtime;
//r                struct tm *times;
//r                int s_id;
//r                nd = nodes + i;
//r//q             delta_id  += *(packed_ids.values     + base + i);
//r                delta_id  += *(packed_ids_v2.data    + base + i);
//r                delta_lat += *(packed_lats.values    + base + i);
//r                delta_lon += *(packed_lons.values    + base + i);
//r                nd->id = delta_id;
//r            /* latitudes and longitudes require to be rescaled as DOUBLEs */
//r                nd->latitude  = delta_lat / 10000000.0;
//r                nd->longitude = delta_lon / 10000000.0;
//r
//r                if (fromPackedInfos) {
//r                      nd->version = *(packed_infos.versions   + base + i);
//r                      xtime       = *(packed_infos.timestamps + base + i);
//r                      times       = gmtime (&xtime);
//r
//r                      if (times) {
//r// printf("times\n");
//r                         // formatting Timestamps
//r                            char buf[64];
//r                            int len;
//r                            sprintf (buf,
//r                                     "%04d-%02d-%02dT%02d:%02d:%02dZ",
//r                                     times->tm_year + 1900,
//r                                     times->tm_mon  +    1,
//r                                     times->tm_mday,
//r                                     times->tm_hour,
//r                                     times->tm_min,
//r                                     times->tm_sec);
//r
//r                            if (nd->timestamp)
//r                                free (nd->timestamp);
//r
//r                            len = strlen (buf);
//r                            nd->timestamp = malloc (len + 1);
//r                            strcpy (nd->timestamp, buf);
//r                        }
//r
//r                      nd->changeset = *(packed_infos.changesets + base + i);
//r
//r                      if (*(packed_infos.uids + base + i) >= 0)
//r                          nd->uid = *(packed_infos.uids + base + i);
//r
//r                      s_id = *(packed_infos.users + base + i);
//r
//r                      if (s_id > 0) {
//r                         /* retrieving user-names as strings (by index) */
//r                            pbf_string_table_elem *s_ptr =
//r                                *(strings->strings + s_id);
//r                            int len = strlen (s_ptr->string);
//r                            if (nd->user != NULL)
//r                                free (nd->user);
//r
//r                            if (len > 0) {
//r                                  nd->user = malloc (len + 1);
//r                                  strcpy (nd->user, s_ptr->string);
//r                              }
//r                        }
//r                }
//r                else {
//r                }
//r
//r                for (; i_keys < packed_keys.count; i_keys++) {
//r//              for (int i_keys = 0; i_keys < packed_keys_v2.count; i_keys++) {
//r//                 printf("i_keys = %d\n", i_keys);
//r                   // decoding packed-keys
//r                      int is = *(packed_keys.values  + i_keys);
//r//                    int is = *(packed_keys_v2.data + i_keys);
//r
//r                      if (is == 0) {
//r                            /* next Node */
//r                            i_keys++;
//rprintf("break\n");
//r                            break;
//r                      }
//r
//r                      if (key == NULL) {
//r                            pbf_string_table_elem *s_ptr =
//r                                *(strings->strings + is);
//r
//r                            key = s_ptr->string;
//rprintf("key = %s\n", key);
//r                      }
//r                      else {
//r                            pbf_string_table_elem *s_ptr = *(strings->strings + is);
//r
//r                            value = s_ptr->string;
//rprintf("value = %s\n", value);
//r                            append_tag_to_node (nd, key, value);
//r                            key = NULL;
//r                            value = NULL;
//r                        }
//r                  }
//r            }
//r          base += max_nodes;
//r
//r          /* processing each Node in the block */
//r//        if (params->node_callback != NULL && params->stop == 0) {
//r                int ret;
//r                readosm_internal_node *nd;
//r                int i;
//r                for (i = 0; i < max_nodes; i++) {
//r                      nd = nodes + i;
//r                      ret = call_node_callback (g_cb_nod, nd);
//r
//r                      if (ret != READOSM_OK) {
//r                            exit(42);
//r                            break;
//r                        }
//r                  }
//r//          }
//r
//r          /* memory cleanup: destroying Nodes */
//r          if (nodes != NULL) {
//r
//r                readosm_internal_node *nd;
//r                int i;
//r                for (i = 0; i < max_nodes; i++)
//r                  {
//r                      nd = nodes + i;
//r                      destroy_internal_node (nd);
//r                  }
//r                free (nodes);
//r            }
//r      }
//r//    }
//r
//r/* memory cleanup */
//r    finalize_uint32_packed(&packed_keys);
//r//
//r//q finalize_int64_packed (&packed_ids);
//r    finalize_int64_packed (&packed_lats);
//r    finalize_int64_packed (&packed_lons);
//r    finalize_packed_infos (&packed_infos);
//r    return 1;
//r
//r  error:
//r    finalize_uint32_packed(&packed_keys);
//r//q finalize_int64_packed (&packed_ids);
//r    finalize_int64_packed (&packed_lats);
//r    finalize_int64_packed (&packed_lons);
//r    finalize_packed_infos (&packed_infos);
//r
//r    if (nodes != NULL) {
//r          readosm_internal_node *nd;
//r          int i;
//r          for (i = 0; i < nd_count; i++) {
//r                nd = nodes + i;
//r                destroy_internal_node (nd);
//r          }
//r          free (nodes);
//r    }
//r    return 0;
}

static int parse_primitive_group_v2 (
   readosm_string_table * strings,
   unsigned char *start,
   unsigned char *end,
            char little_endian_cpu
)
{

// Parse a so-called «Primitive Group» 
// A primitive group stores elements of one OSM type, i. e.
//    - nodes
//    - ways, or
//    - relations

    pbf_field_v2 fld;
    unsigned char *cur = start;


    verbose_1("    parse_primitive_group\n");
    while (1) {

          cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld);
          if (fld.field_id == 2 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { // Dense nodes

               cur = read_bytes_pbf_field_v2 (cur, end, &fld);

               if (!parse_pbf_nodes_v3 (
                     strings,
                     fld.pointer,
                     fld.pointer + fld.str_len - 1
//                ,  g_little_endian_cpu
                   ))
                       wrong_assumption("parse_pbf_nodes");
          }
          else if (fld.field_id == 3 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { // Way

               cur = read_bytes_pbf_field_v2 (cur, end, &fld);

                if (!parse_pbf_way (
                     strings,
                     fld.pointer,
                     fld.pointer + fld.str_len - 1,
                     g_little_endian_cpu
                ))
                     wrong_assumption("parse_pbf_way");
          }
          else if (fld.field_id == 4 && fld.protobuf_type == PROTOBUF_TYPE_LEN) { // Relation

                cur = read_bytes_pbf_field_v2 (cur, end, &fld);

                if (!parse_pbf_relation (
                     strings, fld.pointer,
                     fld.pointer + fld.str_len - 1,
                     g_little_endian_cpu
                ))
                     wrong_assumption("parse_pbf_relation");

            //    goto error;
          }
          else {
               wrong_assumption("node, way or relation");
          }


          if (cur > end)
              break;
      }

    return 1;

  error:

    return 0;
}

static int read_osm_data_block_v3 () {
 //
 // expecting to retrieve a valid OSMData header
 //

    verbose_1("  read_osm_data_block\n");

    int hdsz = block_size_v2( "OSMData");
    if (!hdsz) return 0;


 // -------------------------------

    unsigned char       *buf;
    unsigned char       *cur;
    unsigned char       *end;

    unsigned char       *zip_ptr            = NULL;
    int                  zip_sz             = 0;
    int                  sz_no_compression  = 0;

//  --------------------------------------------------------------------- Data   ---------------------------------------------------------------------------------------------------

    buf   = malloc (hdsz);
    if (!buf) {
       wrong_assumption("buf");
    }

    cur   = buf;
    end   = buf+hdsz-1;

    size_t  rd;
    rd    = fread (buf, 1, hdsz, g_pbf_file);
    if ((int) rd != hdsz) {
       wrong_assumption("vbla");
    }

// uncompressing the OSMData zipped */


 //
 // Determine size of 'primitive block'.
 // The primitive block can be uncompressed or compressed.
 // 

    pbf_field_v2    size_primitive_block;
    pbf_field_v2    zipped_block;

    cur = read_pbf_field_v2_protobuf_type_and_field(cur, &size_primitive_block);

    if (size_primitive_block.field_id == 1) {
    //
    // The block is not compressed
    //
       cur = read_bytes_pbf_field_v2 (cur, end, &size_primitive_block);
       sz_no_compression = size_primitive_block.str_len;
    }
    else if (size_primitive_block.field_id == 2) {

       cur = read_integer_pbf_field_v2(cur, end, READOSM_VAR_INT32, &size_primitive_block);
       
       sz_no_compression = size_primitive_block.value.int32_value;

       cur = read_pbf_field_v2_protobuf_type_and_field(cur, &zipped_block);
       cur = read_bytes_pbf_field_v2 (cur, end, &zipped_block);

       if (zipped_block.field_id != 3) {
          printf("field_id = %d\n", zipped_block.field_id);
          wrong_assumption("zipped_block.field_id == 3");
       }
       zip_ptr = zipped_block.pointer;
       zip_sz  = zipped_block.str_len;
    }
    else {
       wrong_assumption("expected field id 1 or 2");
    }

    if (cur <= end) {
       wrong_assumption("cur<=end");
    }



//  -------------------------------------------------------------------------------------

    if (zip_ptr != NULL && zip_sz != 0 && sz_no_compression != 0) {
    //
    // The primitive block is zipped, we need to unzip it.
    // 
    // According to https://github.com/paulmach/osm?tab=readme-ov-file :
    // 
    //  OSM PBF data comes in blocks, each block is zlib compressed.
    //  Decompressing this data takes about 33% of the total read time.
    //  DataDog/czlib ( https://github.com/DataDog/czlib ) is used to speed this process.
    //  See osmpbf/README.md ( https://github.com/paulmach/osm/blob/master/osmpbf#using-cgoczlib-for-decompression )
    //  for more details.
    //    

          set_uncompressed_buffer(sz_no_compression);
                  
          uLongf unc_size = sz_no_compression;
          int unc_ret = uncompress(
              ptr_uncompressed_buffer,// dest
              &unc_size,              // dest len: on entry, the value is the size of the dest buffer; on exit, value is the length of uncompressed data.
              zip_ptr,                // src
              zip_sz                  // src len
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

     //  --------------------------------------------------------------------- PrimitiveBlock ---------------------------------------------------------------------------------------------------

    pbf_field_v2         fld_data;
 // parsing the PrimitiveBlock

    cur  = ptr_uncompressed_buffer;
    end  = ptr_uncompressed_buffer + sz_no_compression - 1;



 // initializing an empty string list
 // initializing an empty PBF StringTable object
    readosm_string_table string_table;
    string_table.first_string   = NULL;
    string_table.last_string    = NULL;
    string_table.count          =    0;
    string_table.strings        = NULL;


    while (1) {
       // resetting an empty variant field

          cur = read_pbf_field_v2_protobuf_type_and_field(cur, &fld_data);
          
          if (fld_data.field_id == 1 && fld_data.protobuf_type == PROTOBUF_TYPE_LEN) {
              cur = read_bytes_pbf_field_v2 (cur, end, &fld_data);

                if (!parse_string_table ( 
                     &string_table,
                     fld_data.pointer,
                     fld_data.pointer + fld_data.str_len - 1,
                     g_little_endian_cpu
                   ))
                   wrong_assumption("sta");

                array_from_string_table (&string_table);
          }
          else if(fld_data.field_id == 2 && fld_data.protobuf_type == PROTOBUF_TYPE_LEN) {
              cur = read_bytes_pbf_field_v2 (cur, end, &fld_data);

             // the PrimitiveGroup to be parsed
                if (!parse_primitive_group_v2 (
                    &string_table,
                     fld_data.pointer,
                     fld_data.pointer + fld_data.str_len - 1,
                     g_little_endian_cpu
                    ))

                    wrong_assumption("yuh");

          }
          else if (fld_data.field_id == 17 && fld_data.protobuf_type == PROTOBUF_TYPE_VARINT) {
             // assumed to be a termination marker (???)
                wrong_assumption("termination marker never reached");
                break;
          }
          else {
                printf("field_id = %d\n", fld_data.field_id);
                wrong_assumption("field_id not in 1, 2, 17");
          }

          if (cur  > end )
              break;
      }



    finalize_string_table (&string_table);
    return 1;

  error:
    wrong_assumption("err");
    if (buf != NULL)
        free (buf);


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

//  while(read_osm_data_block_v2()) {
    while(read_osm_data_block_v3()) {
       verbose_1("  iteration (load_osm_pbf)\n");

    }
    free(ptr_uncompressed_buffer);

// -------------------------------------------------------------------------------------------------------


    if (g_pbf_file)
       fclose (g_pbf_file);

    return 0;
}
