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
           variant->value.int64_value = (int) value64;
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
          if ((ptr + len - 1) > stop)
              return NULL;

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
   
// exit(100); // TQ84 - remove moe
    verbose_1("       returning from read_header_block_v2\n");
    return 1;

}


static int parse_pbf_nodes_v2 (
                 readosm_string_table * strings,
                 unsigned char *start,
                 unsigned char *stop,
                 char           little_endian_cpu
)
{
/* 
 / Attempting to parse a valid PBF DenseNodes 
 /
 / Remark: a PBF DenseNodes block consists in five strings:
 / - ids
 / - DenseInfos
 / - longitudes
 / - latitudes
 / - packed-keys (*)
 /
 / Each "string" in turn contains an array of INT values;
 / and individual values are usually encoded as DELTAs,
 / i.e. differences respect the immediately preceding value.
 /
 / (*) packed keys actually are encoded as arrays of index
 / to StringTable entries.
 / alternatively we have a key-index and then a value-index;
 / any 0 value means that the current Node stops: next index
 / will be a key-index for the next Node item
*/
    pbf_field       variant;
    unsigned char *base = start;
    readosm_uint32_packed  packed_keys;
    readosm_int64_packed   packed_ids;
    readosm_int64_packed   packed_lats;
    readosm_int64_packed   packed_lons;
    readosm_packed_infos   packed_infos;
    readosm_internal_node *nodes = NULL;
    int nd_count                 = 0;
    int valid                    = 0;
    int fromPackedInfos          = 0;

/* initializing empty packed objects */
    init_uint32_packed (&packed_keys);
    init_int64_packed  (&packed_ids);
    init_int64_packed  (&packed_lats);
    init_int64_packed  (&packed_lons);
    init_packed_infos  (&packed_infos);

/* initializing an empty variant field */
    init_variant (&variant, little_endian_cpu);
   #ifdef TQ84_USE_PBF_FIELD_HINTS
    add_variant_hints (&variant, READOSM_LEN_BYTES,  1);
    add_variant_hints (&variant, READOSM_LEN_BYTES,  5);
    add_variant_hints (&variant, READOSM_LEN_BYTES,  8);
    add_variant_hints (&variant, READOSM_LEN_BYTES,  9);
    add_variant_hints (&variant, READOSM_LEN_BYTES, 10);
   #endif

/* reading the Node */
    while (1) {
       // resetting an empty variant field
          reset_variant (&variant);

          base = read_pbf_field (start, stop, &variant);

          if (base == NULL && variant.valid == 0)
              goto error;

          start = base;

          if (variant.field_id ==  1 && variant.type == READOSM_LEN_BYTES) { /* NODE IDs    */ if (!parse_sint64_packed  (&packed_ids  , variant.pointer, variant.pointer + variant.str_len - 1, variant.little_endian_cpu)) goto error; array_from_int64_packed (&packed_ids);  }
          if (variant.field_id ==  5 && variant.type == READOSM_LEN_BYTES) { /* DenseInfos  */ if (!parse_pbf_node_infos (&packed_infos, variant.pointer, variant.pointer + variant.str_len - 1, variant.little_endian_cpu)) goto error;                                         }
          if (variant.field_id ==  8 && variant.type == READOSM_LEN_BYTES) { /* latitudes   */ if (!parse_sint64_packed  (&packed_lats , variant.pointer, variant.pointer + variant.str_len - 1, variant.little_endian_cpu)) goto error; array_from_int64_packed (&packed_lats); }
          if (variant.field_id ==  9 && variant.type == READOSM_LEN_BYTES) { /* longitudes  */ if (!parse_sint64_packed  (&packed_lons , variant.pointer, variant.pointer + variant.str_len - 1, variant.little_endian_cpu)) goto error; array_from_int64_packed (&packed_lons); }
          if (variant.field_id == 10 && variant.type == READOSM_LEN_BYTES) { /* packes-keys */ if (!parse_uint32_packed  (&packed_keys , variant.pointer, variant.pointer + variant.str_len - 1, variant.little_endian_cpu)) goto error; array_from_uint32_packed(&packed_keys); }
          if (base > stop)
              break;
      }
    if (packed_ids.count == packed_lats.count
        && packed_ids.count == packed_lons.count)
      {
          /* not using PackedInfos */
          valid = 1;
      }
    if (   packed_ids.count == packed_lats.count
        && packed_ids.count == packed_lons.count
        && packed_ids.count == packed_infos.ver_count
        && packed_ids.count == packed_infos.tim_count
        && packed_ids.count == packed_infos.cng_count
        && packed_ids.count == packed_infos.uid_count
        && packed_ids.count == packed_infos.usr_count)
      {
        // from PackedInfos
          valid           = 1;
          fromPackedInfos = 1;
      }

    if (!valid)
        goto error;

    else {
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
          nd_count = packed_ids.count;

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
                      const char *key = NULL;
                      const char *value = NULL;
                      time_t xtime;
                      struct tm *times;
                      int s_id;
                      nd = nodes + i;
                      delta_id += *(packed_ids.values + base + i);
                      delta_lat += *(packed_lats.values + base + i);
                      delta_lon += *(packed_lons.values + base + i);
                      nd->id = delta_id;
                  /* latitudes and longitudes require to be rescaled as DOUBLEs */
                      nd->latitude  = delta_lat / 10000000.0;
                      nd->longitude = delta_lon / 10000000.0;

                      if (fromPackedInfos) {
                            nd->version = *(packed_infos.versions + base + i);
                            xtime       = *(packed_infos.timestamps + base + i);
                            times       = gmtime (&xtime);

                            if (times) {

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
                            nd->changeset =
                                *(packed_infos.changesets + base + i);
                            if (*(packed_infos.uids + base + i) >= 0)
                                nd->uid = *(packed_infos.uids + base + i);
                            s_id = *(packed_infos.users + base + i);
                            if (s_id > 0)
                              {
                                  /* retrieving user-names as strings (by index) */
                                  pbf_string_table_elem *s_ptr =
                                      *(strings->strings + s_id);
                                  int len = strlen (s_ptr->string);
                                  if (nd->user != NULL)
                                      free (nd->user);
                                  if (len > 0)
                                    {
                                        nd->user = malloc (len + 1);
                                        strcpy (nd->user, s_ptr->string);
                                    }
                              }
                        }

                      for (; i_keys < packed_keys.count; i_keys++) {
                          // decoding packed-keys
                            int is = *(packed_keys.values + i_keys);

                            if (is == 0) {
                                  /* next Node */
                                  i_keys++;
                                  break;
                              }
                            if (key == NULL)
                              {
                                  pbf_string_table_elem *s_ptr =
                                      *(strings->strings + is);
                                  key = s_ptr->string;
                              }
                            else
                              {
                                  pbf_string_table_elem *s_ptr =
                                      *(strings->strings + is);
                                  value = s_ptr->string;
                                  append_tag_to_node (nd, key, value);
                                  key = NULL;
                                  value = NULL;
                              }
                        }
                  }
                base += max_nodes;

                /* processing each Node in the block */
//              if (params->node_callback != NULL && params->stop == 0) {
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
//                }

                /* memory cleanup: destroying Nodes */
                if (nodes != NULL)
                  {
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
      }

/* memory cleanup */
    finalize_uint32_packed (&packed_keys);
    finalize_int64_packed (&packed_ids);
    finalize_int64_packed (&packed_lats);
    finalize_int64_packed (&packed_lons);
    finalize_packed_infos (&packed_infos);
   #ifdef TQ84_USE_PBF_FIELD_HINTS
    finalize_variant (&variant);
   #endif
    return 1;

  error:
    finalize_uint32_packed(&packed_keys);
    finalize_int64_packed (&packed_ids);
    finalize_int64_packed (&packed_lats);
    finalize_int64_packed (&packed_lons);
    finalize_packed_infos (&packed_infos);
   #ifdef TQ84_USE_PBF_FIELD_HINTS
    finalize_variant (&variant);
   #endif

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

               if (!parse_pbf_nodes_v2 (
                     strings,
                     fld.pointer,
                     fld.pointer + fld.str_len - 1,
                     g_little_endian_cpu
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


   #ifdef TQ84_USE_PBF_FIELD_HINTS
   #endif

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
