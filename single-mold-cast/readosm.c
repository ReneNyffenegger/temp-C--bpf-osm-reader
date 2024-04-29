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

// static readosm_file * alloc_osm_file (int little_endian_cpu/*, int format*/) {
// 
// /* allocating and initializing the OSM input file struct */
//     readosm_file *input = malloc (sizeof (readosm_file));
//     if (!input)
//         return NULL;
// 
// //  input->magic1 = READOSM_MAGIC_START;
// //  input->file_format = format;
//     input->little_endian_cpu = little_endian_cpu;
// //  input->magic2 = READOSM_MAGIC_END;
//     input->in = NULL;
//     return input;
// }

// static void destroy_osm_file (readosm_file * input) {
// 
// /* destroying the OSM input file struct */
//     if (input) {
//           if (input->in)
//               fclose (input->in);
//           free (input);
//       }
// }

// /* READOSM_DECLARE */ int readosm_open (const char *path, readosm_file **osm_handle) {
// 
// // opening and initializing the OSM input file 
// //  readosm_file *input;
// //  int len;
//     int format;
//     int little_endian_cpu = test_endianness();
// 
// //  *osm_handle = NULL;
// //  if (path == NULL || osm_handle == NULL)
// //      return READOSM_NULL_HANDLE;
// 
// //  len = strlen (path);
// 
// //  if (len > 4 && strcasecmp (path + len - 4, ".osm") == 0)
// //      format = READOSM_OSM_FORMAT;
// 
// //  else if (len > 4 && strcasecmp (path + len - 4, ".pbf") == 0)
// //      format = READOSM_PBF_FORMAT;
// 
// //  else
// //      return READOSM_INVALID_SUFFIX;
// 
// /* allocating the OSM input file struct */
// //  input = alloc_osm_file (little_endian_cpu/*, format*/);
//    *osm_handle = alloc_osm_file (little_endian_cpu/*, format*/);
// 
//     if (! *osm_handle)
//         return READOSM_INSUFFICIENT_MEMORY;
// 
// //  *osm_handle = input;
// 
//    ( *osm_handle)->in = fopen(path, "rb");
// //  input->in = fopen (path, "rb");
//     if ((*osm_handle)->in == NULL)
//         return READOSM_FILE_NOT_FOUND;
// 
//     return READOSM_OK;
// }

/* READOSM_DECLARE */
// int readosm_close (const void *osm_handle) {
// 
// // attempting to destroy the OSM input file 
// //
//     readosm_file *input = (readosm_file *) osm_handle;
//     if (!input)
//         return READOSM_NULL_HANDLE;
// 
// //  if ((input->magic1 == READOSM_MAGIC_START) && input->magic2 == READOSM_MAGIC_END) ;
// //  else
// //      return READOSM_INVALID_HANDLE;
// 
// /* destroying the workbook */
//     destroy_osm_file (input);
// 
//     return READOSM_OK;
// }


static int parse_osm_data (unsigned int sz) {
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


    printf("  parse_osm_data\n");

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
    add_variant_hints (&variant, READOSM_LEN_BYTES, 1);
    add_variant_hints (&variant, READOSM_LEN_BYTES, 2);
    add_variant_hints (&variant, READOSM_VAR_INT32, 3);

    rd = fread (buf, 1, sz, g_pbf_file/*, input->in*/);
    if (rd != sz)
        goto error;

     //  --------------------------------------------------------------------- Header ---------------------------------------------------------------------------------------------------

 //
 // reading the OSMData header
 //
    while (1) {
          printf("  iterating\n");

       // resetting an empty variant field
          reset_variant (&variant);

          base = parse_field (start, stop, &variant);

          if (base == NULL && variant.valid == 0)
              goto error;

          start = base;

          if (variant.field_id == 1 && variant.type == READOSM_LEN_BYTES && variant.str_len == 7) {
                printf("     field_id = 1\n");
                if (memcmp (variant.pointer, "OSMData", 7) == 0) ok_header = 1;
          }

          if (variant.field_id == 3 && variant.type == READOSM_VAR_INT32) {
              hdsz = variant.value.int32_value;
              printf("     field_id = 3, hdsz = %d\n", hdsz);
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
    finalize_variant  (&variant);
    add_variant_hints (&variant, READOSM_LEN_BYTES, 1);
    add_variant_hints (&variant, READOSM_VAR_INT32, 2);
    add_variant_hints (&variant, READOSM_LEN_BYTES, 3);
    while (1)
      {
       // resetting an empty variant field
          reset_variant (&variant);

          base = parse_field (start, stop, &variant);
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
    finalize_variant (&variant);
    add_variant_hints (&variant, READOSM_LEN_BYTES,  1);
    add_variant_hints (&variant, READOSM_LEN_BYTES,  2);
    add_variant_hints (&variant, READOSM_VAR_INT32, 17);
    add_variant_hints (&variant, READOSM_VAR_INT32, 18);
    add_variant_hints (&variant, READOSM_VAR_INT64, 19);
    add_variant_hints (&variant, READOSM_VAR_INT64, 20);

    while (1) {
       // resetting an empty variant field
          reset_variant (&variant);

          base = parse_field (start, stop, &variant);
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
    finalize_variant (&variant);
    finalize_string_table (&string_table);
    return 1;

  error:
    if (buf != NULL)
        free (buf);
    if (raw_ptr != NULL)
        free (raw_ptr);
    finalize_variant (&variant);
    finalize_string_table (&string_table);
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

//  readosm_file *osm_handle;

    g_cb_nod            = cb_nod;
    g_cb_way            = cb_way;
    g_cb_rel            = cb_rel;
    g_little_endian_cpu = test_endianness();

//  ret = readosm_open(filename_pbf, /*(const readosm_file**)*/ &osm_handle);

/* allocating and initializing the OSM input file struct */
//  osm_handle = malloc (sizeof (readosm_file));
//  if (!osm_handle)
//      return 999;

//  input->magic1 = READOSM_MAGIC_START;
//  input->file_format = format;
//  osm_handle -> little_endian_cpu = test_endianness();

//  osm_handle->magic2 = READOSM_MAGIC_END;


//  if (! osm_handle)
//      return READOSM_INSUFFICIENT_MEMORY;

//  *osm_handle = input;

//  osm_handle->in = fopen(filename_pbf, "rb");
    g_pbf_file = fopen(filename_pbf, "rb");

    if (g_pbf_file == NULL)
        return READOSM_FILE_NOT_FOUND;







 // if (ret != READOSM_OK) {
 //     fprintf (stderr, "OPEN error: %d (filename_pbf = %s)\n", ret, filename_pbf);
 //     goto stop;
 // }





//  int ret;
//  ret = parse_osm_pbf(/*osm_handle,*/  cb_nod, cb_way, cb_rel);

    rd = fread (buf, 1, 4, g_pbf_file);
    if (rd != 4) return READOSM_INVALID_PBF_HEADER;

    hdsz = get_header_size (buf /*, g_little_endian_cpu*/ /* input->little_endian_cpu */);

/* testing OSMHeader */
    if (!skip_osm_header (/*input,*/ hdsz))
        return READOSM_INVALID_PBF_HEADER;

// -------------------------------------------------------------------------------------------------------

/* 
 / the PBF file is internally organized as a collection
 / of many subsequent OSMData blocks 
*/
    while (1) {

          rd = fread (buf, 1, 4, g_pbf_file);

          if (rd == 0 && feof (g_pbf_file))
              break;

          if (rd != 4) return READOSM_INVALID_PBF_HEADER;

          hdsz = get_header_size (buf /*, g_little_endian_cpu */ /* input->little_endian_cpu*/);

        // parsing OSMData
          if (!parse_osm_data (/*input,*/ hdsz /*, &params */))
              return READOSM_INVALID_PBF_HEADER;
    }

// -------------------------------------------------------------------------------------------------------

//  if (ret != READOSM_OK) {
//      fprintf (stderr, "PARSE error: %d\n", ret);
//  }
//  else {
//     fprintf (stderr, "Ok, OSM input file successfully parsed\n");
//  }

//stop:

    if (g_pbf_file)
       fclose (g_pbf_file);

//  free (osm_handle);
//  destroy_osm_file(osm_handle);
//  readosm_close (osm_handle);
    return 0;
}
