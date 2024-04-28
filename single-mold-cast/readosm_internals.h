#pragma once
/* 
/ readosm_internals.h
/
/ internal declarations
/
/ version  1.0, 2012 April 10
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
/ for the specific language governing rights and limitations under theskip:
/ License.
/
/ The Original Code is the ReadOSM library
/
/ The Initial Developer of the Original Code is Alessandro Furieri
/ 
/ Portions created by the Initial Developer are Copyright (C) 2012
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

/* endianness */
#define READOSM_BIG_ENDIAN      0
#define READOSM_LITTLE_ENDIAN   1

/* magic signatures */
// #define READOSM_MAGIC_START     945371767
// #define READOSM_MAGIC_END       1472954381

// /* file formats */
// #define READOSM_OSM_FORMAT      4589
// #define READOSM_PBF_FORMAT      7491

// /* XML tags */
// #define READOSM_CURRENT_TAG_UNKNOWN  0
// #define READOSM_CURRENT_TAG_IS_NODE  101
// #define READOSM_CURRENT_TAG_IS_WAY   102
// #define READOSM_CURRENT_TAG_IS_RELATION      103

// block size
#define READOSM_BLOCK_SZ        128

typedef struct readosm_internal_tag_struct {

// a struct wrapping TAG items
    char *key;                  // pointer to KEY value (NULL terminated string)
    char *value;                // pointer to VALUE (NULL terminated string)
} readosm_internal_tag;

typedef struct readosm_internal_tag_block readosm_internal_tag_block;
struct  readosm_internal_tag_block {
 //
 // a struct wrapping a block of TAG items
 //
    readosm_internal_tag tags[READOSM_BLOCK_SZ];        /* array of Tags */
    int next_item;                                      /* index to next free (unused) item */
    readosm_internal_tag_block *next;                   /* supporting linked list */
};

typedef struct readosm_export_tag_struct
{
/* a struct intended to export TAG items */
    char *key;                  /* pointer to KEY value (NULL terminated string) */
    char *value;                /* pointer to VALUE (NULL terminated string) */
} readosm_export_tag;

typedef struct /* readosm_internal_node_struct */ {
 // 
 // a struct wrapping NODE items */
 //
    long long                   id;               /* NODE-ID (unique value) */
    double                      latitude;            /* geographic latitude */
    double                      longitude;           /* geographic longitude */
    int                         version;                /* version id */
    long long                   changeset;        /* changeset id */
    char                       *user;                 /* pointer to user name (NULL terminated string) */
    int                         uid;                    /* uid identifying the user */
    char                       *timestamp;            /* last modified timestamp */
    int                         tag_count;              /* how many TAG items are there */
    readosm_internal_tag_block  first_tag;       /* pointers supporting a linked list */
    readosm_internal_tag_block *last_tag;       /* of TAG blocks (first block is static) */
} readosm_internal_node;


typedef struct readosm_export_node_struct {
 // a struct intended to export NODE items
    long long id;               /* NODE-ID (unique value) */
    double latitude;            /* geographic latitude */
    double longitude;           /* geographic longitude */
    int version;                /* version id */
    long long changeset;        /* changeset id */
    char *user;                 /* pointer to user name (NULL terminated string) */
    int uid;                    /* uid identifying the user */
    char *timestamp;            /* last modified timestamp */
    int tag_count;              /* how many TAG items are there */
    readosm_export_tag *tags;   /* array of TAG items */
} readosm_export_node;

typedef struct readosm_internal_ref readosm_internal_ref;
struct readosm_internal_ref {

 // a block of WAY-ND items
 //
    long long node_refs[READOSM_BLOCK_SZ];      /* referenced NODE-ID array */
    int next_item;                              /* index to next free (unused) item */
    readosm_internal_ref *next;                 /* supporting linked list */
};

typedef struct readosm_internal_way_struct {

/* a struct wrapping WAY items */
    long long id;               /* WAY-ID (unique value) */
    int version;                /* version id */
    long long changeset;        /* changeset id */
    char *user;                 /* pointer to user name (NULL terminated string) */
    int uid;                    /* uid identifying the user */
    char *timestamp;            /* last modified timestamp */
    int ref_count;              /* how many WAY-ND items are there */
    readosm_internal_ref first_ref;     /* pointers supporting a linked list */
    readosm_internal_ref *last_ref;     /* of WAY-ND items (first block is static) */
    int tag_count;              /* how many TAG items are there */
    readosm_internal_tag_block first_tag;       /* pointers supporting a linked list */
    readosm_internal_tag_block *last_tag;       /* of TAG blocks (first block is static) */
} readosm_internal_way;

typedef struct readosm_export_way_struct
{
/* a struct intended to export WAY items */
    long long id;               /* WAY-ID (unique value) */
    int version;                /* version id */
    long long changeset;        /* changeset id */
    char *user;                 /* pointer to user name (NULL terminated string) */
    int uid;                    /* uid identifying the user */
    char *timestamp;            /* last modified timestamp */
    int node_ref_count;         /* how many WAY-ND items are there */
    long long *node_refs;       /* array of WAY-ND items */
    int tag_count;              /* how many TAG items are there */
    readosm_export_tag *tags;   /* array of TAG items */
} readosm_export_way;

typedef struct readosm_internal_member_struct
{
/* a struct wrapping RELATION-MEMBER items */
    int member_type;            /* member type [some READOSM_MEMBER_xx constant] */
    long long id;               /* ID of the referenced object */
    char *role;                 /* pointer to role value (NULL terminated string) */
} readosm_internal_member;

typedef struct readosm_internal_member_block_struct
{
/* a block of RELATION-MEMBER items */
    readosm_internal_member members[READOSM_BLOCK_SZ];  /* array of MEMBERs */
    int next_item;              /* index to next free (unused) item */
    struct readosm_internal_member_block_struct *next;  /* supporting linked list */
} readosm_internal_member_block;

typedef struct readosm_export_member_struct
{
/* a struct intended to export RELATION-MEMBER items */
    int member_type;            /* member type [some READOSM_MEMBER_xx constant] */
    long long id;               /* ID of the referenced object */
    char *role;                 /* pointer to role value (NULL terminated string) */
} readosm_export_member;

typedef struct readosm_internal_relation_struct
{
/* a struct wrapping RELATION items */
    long long id;               /* RELATION-ID (unique value) */
    int version;                /* version id */
    long long changeset;        /* changeset id */
    char *user;                 /* pointer to user name (NULL terminated string) */
    int uid;                    /* uid identifying the user */
    char *timestamp;            /* last modified timestamp */
    int member_count;           /* how many RELATION-MEMBER items are there */
    readosm_internal_member_block first_member; /* pointers supporting a linked list */
    readosm_internal_member_block *last_member; /* of RELATION-MEMBER items (first block is static) */
    int tag_count;              /* how many TAG items are there */
    readosm_internal_tag_block first_tag;       /* pointers supporting a linked list */
    readosm_internal_tag_block *last_tag;       /* of TAG-blocks (first block is static) */
} readosm_internal_relation;

typedef struct /* readosm_export_relation_struct */ {

 // a struct intended to export RELATION objects */

    long long id;               /* RELATION-ID (unique value) */
    int version;                /* version id */
    long long changeset;        /* changeset id */
    char *user;                 /* pointer to user name (NULL terminated string) */
    int uid;                    /* uid identifying the user */
    char *timestamp;            /* last modified timestamp */
    int member_count;           /* how many RELATION-MEMBER items are there */
    readosm_export_member  *members;     /* array of RELATION-MEMBER items */
    int                     tag_count;              /* how many TAG items are there */
    readosm_export_tag     *tags;   /* array of TAG items */
} readosm_export_relation;



typedef struct readosm_file_struct {

/* a struct representing an OSM input file */
//  int   magic1;               // magic signature #1
    FILE *in;                   // file handle */
//  int   file_format;          // the actual file format
    char  little_endian_cpu;    // actual CPU endianness
//  int   magic2;               // magic signature
}
readosm_file;

// functions handling common OSM objects
//
void release_internal_tag_block (readosm_internal_tag_block * tag_block, int destroy);
void release_internal_member_block (readosm_internal_member_block * member_block, int destroy);
void init_internal_node (readosm_internal_node * node);
void append_tag_to_node (readosm_internal_node * node, const char *key, const char *value);
void destroy_internal_node (readosm_internal_node * node);
readosm_internal_way *alloc_internal_way (void);
void append_reference_to_way (readosm_internal_way * way, long long node_ref);
void append_tag_to_way (readosm_internal_way * way, const char *key, const char *value);
void destroy_internal_way (readosm_internal_way * way);
readosm_internal_relation *alloc_internal_relation (void);
void append_member_to_relation (readosm_internal_relation * relation, int type, long long id, const char *role);
void append_tag_to_relation (readosm_internal_relation * relation, const char *key, const char *value);
void destroy_internal_relation (readosm_internal_relation * relation);


// callback handlers
int call_node_callback     (readosm_node_callback     node_callback    , readosm_internal_node     * node);
int call_way_callback      (readosm_way_callback      way_callback     , readosm_internal_way      * way);
int call_relation_callback (readosm_relation_callback relation_callback, readosm_internal_relation * relation);
