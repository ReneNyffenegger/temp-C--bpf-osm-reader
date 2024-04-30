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

void wrong_assumption(char* txt) {
   printf("\033[1;31m%s\033[0m\n", txt);
   exit(1);
}


   #define TQ84_USE_PBF_FIELD_HINTS
   #define TQ84_VERBOSE_1

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


static int read_osm_data_block (unsigned int sz) {
 //
 // expecting to retrieve a valid OSMData header
 //

    int ok_header = 0;
    int hdsz      = 0;
    size_t               rd;
    unsigned char       *buf     = malloc (sz);
    unsigned char       *base    = buf;
    unsigned char       *start   = buf;
    unsigned char       *stop    = buf + sz - 1;
    unsigned char       *zip_ptr = NULL;
    int                  zip_sz  = 0;
    unsigned char       *raw_ptr = NULL;
    int                  raw_sz  = 0;
    pbf_field            variant;


    verbose_1("  read_osm_data_block\n");

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

    rd = fread (buf, 1, sz, g_pbf_file/*, input->in*/);
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

    buf = malloc (hdsz);
    base = buf;
    start = buf;
    stop = buf + hdsz - 1;
    rd = fread (buf, 1, hdsz, g_pbf_file/*input->in*/);
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
       // resetting an empty variant field
          reset_variant (&variant);

          base = read_pbf_field (start, stop, &variant);
          if (base == NULL && variant.valid == 0)
              goto error;

          start = base;
          if (variant.field_id == 1 && variant.type == READOSM_LEN_BYTES) {
             // found an uncompressed block */
                raw_sz = variant.str_len;
                raw_ptr = malloc (raw_sz);
                memcpy (raw_ptr, variant.pointer, raw_sz);
          }
          if (variant.field_id == 2 && variant.type == READOSM_VAR_INT32) {
              // expected size of unZipped block */
                raw_sz = variant.value.int32_value;
          }
          if (variant.field_id == 3 && variant.type == READOSM_LEN_BYTES) {
              // found a ZIP-compressed block
                zip_ptr = variant.pointer;
                zip_sz = variant.str_len;
          }
          if (base > stop)
              break;
    }

    if (zip_ptr != NULL && zip_sz != 0 && raw_sz != 0) {
          /* unZipping a compressed block */
          raw_ptr = malloc (raw_sz);
          if (!unzip_compressed_block (zip_ptr, zip_sz, raw_ptr, raw_sz))
              goto error;
    }

    free (buf);
    buf = NULL;
    if (raw_ptr == NULL || raw_sz == 0)
        goto error;

     //  --------------------------------------------------------------------- PrimitiveBlock ---------------------------------------------------------------------------------------------------

 // parsing the PrimitiveBlock

    base  = raw_ptr;
    start = raw_ptr;
    stop  = raw_ptr + raw_sz - 1;
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
    unsigned char *base  = buf;
    unsigned char *start = buf;
    unsigned char *stop  = buf + sz - 1;
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

          base = read_pbf_field (start, stop, &fld);
          if (base == NULL && fld.valid == 0)
              goto error;

          start = base;
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
              verbose_1("      else\n");
          }

          if (base > stop) {
              verbose_1("      base > stop\n");
              break;
          }
    }

    free (buf);
    buf = NULL;

    if (!ok_header || !hdsz)
        goto error;

    buf   = malloc (hdsz);
    base  = buf;
    start = buf;
    stop  = buf + hdsz - 1;

    rd = fread (buf, 1, hdsz, g_pbf_file/*, input->in*/);

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

    rd = fread (buf, 1, 4, g_pbf_file);
    if (rd != 4) return READOSM_INVALID_PBF_HEADER;

    hdsz = get_header_size (buf);

    //  testing OSMHeader
    if (!read_header_block (hdsz))
        return READOSM_INVALID_PBF_HEADER;

// ----- Data blocks -------------------------------------------------------------------------------------

// 
// the PBF file is internally organized as a collection
// of many subsequent OSMData blocks 
//
    while (1) {
       verbose_1("  iteration (load_osm_pbf)\n");

          rd = fread (buf, 1, 4, g_pbf_file);

          if (rd == 0 && feof (g_pbf_file))
              break;

          if (rd != 4) return READOSM_INVALID_PBF_HEADER;
          hdsz = get_header_size (buf);

        // parsing OSMData
          if (!read_osm_data_block (hdsz))
              return READOSM_INVALID_PBF_HEADER;
    }

// -------------------------------------------------------------------------------------------------------


    if (g_pbf_file)
       fclose (g_pbf_file);

    return 0;
}
