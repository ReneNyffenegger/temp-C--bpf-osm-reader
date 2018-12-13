#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "readosm.h"

sqlite3*      db;
sqlite3_stmt* stmt_ins_nod;
sqlite3_stmt* stmt_ins_nod_way;
sqlite3_stmt* stmt_ins_rel_mem_nod;
sqlite3_stmt* stmt_ins_rel_mem_way;
sqlite3_stmt* stmt_ins_rel_mem_rel;
sqlite3_stmt* stmt_ins_tag_way;
sqlite3_stmt* stmt_ins_tag_nod;
sqlite3_stmt* stmt_ins_tag_rel;

void dbExec(const char* sql) {
  if (sqlite3_exec(db, sql, NULL, NULL, NULL)) {
     printf("Could not exec %s\n", sql);
     exit(-1);
  }
}

static int callback_node (const void *user_data, const readosm_node * node) {
    char buf[128];
    int i;
    const readosm_tag *tag;

//  TQ84: commented
//
//  if (user_data != NULL) {
//    user_data = NULL;    /* silencing stupid compiler warnings */
//  }


//  TQ84: commented
//
// #if defined(_WIN32) || defined(__MINGW32__)
//     /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
//     sprintf (buf, "%I64d", node->id);
// #else
//     sprintf (buf, "%lld", node->id);
// #endif
//     printf ("\t<node id=\"%s\"", buf);

/*
* some individual values may be set, or may be not
* unset values are identified by the READOSM_UNDEFINED
* conventional value, and must be consequently ignored
*/
// TQ84 commented:
//
//    if (node->latitude != READOSM_UNDEFINED) 
//    printf (" lat=\"%1.7f\"", node->latitude);
//
//    if (node->longitude != READOSM_UNDEFINED)
//    printf (" lon=\"%1.7f\"", node->longitude);
//
//    if (node->version != READOSM_UNDEFINED)
//    printf (" version=\"%d\"", node->version);
//
//    if (node->changeset != READOSM_UNDEFINED) {
//
//#if defined(_WIN32) || defined(__MINGW32__)
//      /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
//      sprintf (buf, "%I64d", node->changeset);
//#else
//      sprintf (buf, "%lld", node->changeset);
//#endif
//      printf (" changeset=\"%s\"", buf);
//   }

/*
* unset string values are identified by a NULL pointer
* and must be consequently ignored
*/
// TQ84
//    if (node->user != NULL)
//    printf (" user=\"%s\"", node->user);
//
//    if (node->uid != READOSM_UNDEFINED)
//    printf (" uid=\"%d\"", node->uid);
//
//    if (node->timestamp != NULL)
//    printf (" timestamp=\"%s\"", node->timestamp);

   sqlite3_bind_int64 (stmt_ins_nod, 1, node ->id);
   sqlite3_bind_double(stmt_ins_nod, 2, node ->latitude);
   sqlite3_bind_double(stmt_ins_nod, 3, node ->longitude);
   sqlite3_step       (stmt_ins_nod);
   sqlite3_reset      (stmt_ins_nod);

/*
* the Node object may have its own tag list
* please note: this one is a variable-length list,
* and may be empty: in this case tag_count will be ZERO
*/
// TQ84
//    if (node->tag_count == 0) {
//       printf (" />\n");
//    }
//    else {
//      printf (">\n");
// 
    for (i = 0; i < node->tag_count; i++) {
        tag = node->tags + i;
        sqlite3_bind_int64(stmt_ins_tag_nod, 1, node->id  );
        sqlite3_bind_text (stmt_ins_tag_nod, 2, tag->key  , -1, NULL);
        sqlite3_bind_text (stmt_ins_tag_nod, 3, tag->value, -1, NULL);
        sqlite3_step      (stmt_ins_tag_nod);
        sqlite3_reset     (stmt_ins_tag_nod);

//      printf ("\t\t<tag k=\"%s\" v=\"%s\" />\n", tag->key,
//          tag->value);
//      }
//      printf ("\t</node>\n");
    }
    return READOSM_OK;
}

static int callback_way (const void *user_data, const readosm_way * way) {
    char buf[128];
    int i;
    const readosm_tag *tag;

// TQ84
//  if (user_data != NULL) {
//      user_data = NULL;    /* silencing stupid compiler warnings */
//  }

// #if defined(_WIN32) || defined(__MINGW32__)
//     /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
//     sprintf (buf, "%I64d", way->id);
// #else
//     sprintf (buf, "%lld", way->id);
// #endif
// TQ84   printf ("\t<way id=\"%s\"", buf);

/*
* some individual values may be set, or may be not
* unset values are identified by the READOSM_UNDEFINED
* conventional value, and must be consequently ignored
*/
//    if (way->version != READOSM_UNDEFINED) {
// TQ84      printf (" version=\"%d\"", way->version);
//    }
//    if (way->changeset != READOSM_UNDEFINED) {
//#if defined(_WIN32) || defined(__MINGW32__)
//      /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
//      sprintf (buf, "%I64d", way->changeset);
//#else
//      sprintf (buf, "%lld", way->changeset);
//#endif
//// TQ84      printf (" changeset=\"%s\"", buf);
//    }

/*
* unset string values are identified by a NULL pointer
* and must be consequently ignored
*/
//    if (way->user != NULL) {
//// TQ84      printf (" user=\"%s\"", way->user);
//    }
//    if (way->uid != READOSM_UNDEFINED) {
//// TQ84      printf (" uid=\"%d\"", way->uid);
//    }
//    if (way->timestamp != NULL) {
//// TQ84      printf (" timestamp=\"%s\"", way->timestamp);
//    }
/*
* the Way object may have a noderefs-list and a tag-list
* please note: these are variable-length lists, and may 
* be empty: in this case the corresponding item count 
* will be ZERO
*/
//  if (way->tag_count == 0 && way->node_ref_count == 0) {
//    printf (" />\n");
//  }
//  else {
//    printf (">\n");
      for (i = 0; i < way->node_ref_count; i++) {
        /* we'll now print each <nd ref> for this way */
//#if defined(_WIN32) || defined(__MINGW32__)
//        /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
//        sprintf (buf, "%I64d", *(way->node_refs + i));
//#else
//        sprintf (buf, "%lld", *(way->node_refs + i));
//#endif
//       printf ("\t\t<nd ref=\"%s\" />\n", buf);
         sqlite3_bind_int64(stmt_ins_nod_way, 1, way->id);
         sqlite3_bind_int64(stmt_ins_nod_way, 2, *(way->node_refs+i)); // TODO: Should this be checked for NULL?
         sqlite3_bind_int  (stmt_ins_nod_way, 3, i);
         sqlite3_step      (stmt_ins_nod_way);
         sqlite3_reset     (stmt_ins_nod_way);
      }

      for (i = 0; i < way->tag_count; i++) {
          tag = way->tags + i;
          sqlite3_bind_int64(stmt_ins_tag_way, 1, way->id  );
          sqlite3_bind_text (stmt_ins_tag_way, 2, tag->key  , -1, NULL);
          sqlite3_bind_text (stmt_ins_tag_way, 3, tag->value, -1, NULL);
          sqlite3_step      (stmt_ins_tag_way);
          sqlite3_reset     (stmt_ins_tag_way);

      }

//    for (i = 0; i < way->tag_count; i++) {
//      /* we'll now print each <tag> for this way */
//      tag = way->tags + i;
//      printf ("\t\t<tag k=\"%s\" v=\"%s\" />\n", tag->key,
//          tag->value);
//    }
//    printf ("\t</way>\n");
//  }
    return READOSM_OK;
}

static int callback_relation (const void *user_data, const readosm_relation * relation) {
/* 
* printing an OSM Relation (callback function) 
*
* this function is called by the OSM parser for each 
* RELATION object found
*
* please note well: the passed pointer corresponds to
* a READ-ONLY object; you can can query any relation-related
* value, but you cannot alter them.
*
************************************************
*
* this didactic sample will simply print the relation object
* on the standard output adopting the appropriate OSM XML
* notation
*/
    char buf[128];
    int i;
    const readosm_member *member;
    const readosm_tag *tag;

//  if (user_data != NULL) {
//    user_data = NULL;    /* silencing stupid compiler warnings */
//  }


// #if defined(_WIN32) || defined(__MINGW32__)
//     /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
//     sprintf (buf, "%I64d", relation->id);
// #else
//     sprintf (buf, "%lld", relation->id);
// #endif
//     printf ("\t<relation id=\"%s\"", buf);

/*
* some individual values may be set, or may be not
* unset values are identified by the READOSM_UNDEFINED
* conventional value, and must be consequently ignored
*/
//    if (relation->version != READOSM_UNDEFINED) {
//      printf (" version=\"%d\"", relation->version);
//    }
//    if (relation->changeset != READOSM_UNDEFINED) {
//#if defined(_WIN32) || defined(__MINGW32__)
//      /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
//      sprintf (buf, "%I64d", relation->changeset);
//#else
//      sprintf (buf, "%lld", relation->changeset);
//#endif
//      printf (" changeset=\"%s\"", buf);
//    }

/*
* unset string values are identified by a NULL pointer
* and must be consequently ignored
*/
//    if (relation->user != NULL) {
//      printf (" user=\"%s\"", relation->user);
//    }
//    if (relation->uid != READOSM_UNDEFINED) {
//      printf (" uid=\"%d\"", relation->uid);
//    }
//    if (relation->timestamp != NULL) {
//      printf (" timestamp=\"%s\"", relation->timestamp);
//    }

/*
* the Relation object may have a member-list and a tag-list
* please note: these are variable-length lists, and may 
* be empty: in this case the corresponding item count 
* will be ZERO
*/
//    if (relation->tag_count == 0 && relation->member_count == 0) {
//      printf (" />\n");
//    }
//    else {
//      printf (">\n");
//
      for (i = 0; i < relation->member_count; i++) {
        /* we'll now print each <member> for this way */
        member = relation->members + i;
// #if defined(_WIN32) || defined(__MINGW32__)
//         /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
//         sprintf (buf, "%I64d", member->id);
// #else
//         sprintf (buf, "%lld", member->id);
// #endif
        /* any <member> may be of "node", "way" or "relation" type */
        switch (member->member_type) {
          case READOSM_MEMBER_NODE:
//            printf ("\t\t<member type=\"node\" ref=\"%s\"", buf);
              sqlite3_bind_int64(stmt_ins_rel_mem_nod, 1, relation->id);
              sqlite3_bind_int  (stmt_ins_rel_mem_nod, 2, i);
              sqlite3_bind_int64(stmt_ins_rel_mem_nod, 3, member->id);
              sqlite3_bind_text (stmt_ins_rel_mem_nod, 4, member->role, -1, NULL); // TODO: should be checked for NULL?
              sqlite3_step      (stmt_ins_rel_mem_nod);
              sqlite3_reset     (stmt_ins_rel_mem_nod);
              break;

          case READOSM_MEMBER_WAY:
//            printf ("\t\t<member type=\"way\" ref=\"%s\"", buf);
              sqlite3_bind_int64(stmt_ins_rel_mem_way, 1, relation->id);
              sqlite3_bind_int  (stmt_ins_rel_mem_way, 2, i);
              sqlite3_bind_int64(stmt_ins_rel_mem_way, 3, member->id);
              sqlite3_bind_text (stmt_ins_rel_mem_way, 4, member->role, -1, NULL); // TODO: should be checked for NULL?
              sqlite3_step      (stmt_ins_rel_mem_way);
              sqlite3_reset     (stmt_ins_rel_mem_way);
              break;

          case READOSM_MEMBER_RELATION:
//            printf ("\t\t<member type=\"relation\" ref=\"%s\"", buf);
              sqlite3_bind_int64(stmt_ins_rel_mem_rel, 1, relation->id);
              sqlite3_bind_int  (stmt_ins_rel_mem_rel, 2, i);
              sqlite3_bind_int64(stmt_ins_rel_mem_rel, 3, member->id);
              sqlite3_bind_text (stmt_ins_rel_mem_rel, 4, member->role, -1, NULL); // TODO: should be checked for NULL?
              sqlite3_step      (stmt_ins_rel_mem_rel);
              sqlite3_reset     (stmt_ins_rel_mem_rel);
              break;

          default:
              printf("???\n");
//            printf ("\t\t<member ref=\"%s\"", buf);
//            break;
        };
//      if (member->role != NULL) {
//          printf (" role=\"%s\" />\n", member->role);
//      }
//      else {
//          printf (" />\n");
//      }
      }

      for (i = 0; i < relation->tag_count; i++) {
//        /* we'll now print each <tag> for this way */
        tag = relation->tags + i;
        sqlite3_bind_int64(stmt_ins_tag_rel, 1, relation->id);
        sqlite3_bind_text (stmt_ins_tag_rel, 2, tag->key  , -1, NULL);
        sqlite3_bind_text (stmt_ins_tag_rel, 3, tag->value, -1, NULL);
        sqlite3_step      (stmt_ins_tag_rel);
        sqlite3_reset     (stmt_ins_tag_rel);

////      printf ("\t\t<tag k=\"%s\" v=\"%s\" />\n", tag->key,
////          tag->value);
      }
////      printf ("\t</relation>\n");
//     }
    return READOSM_OK;
}

void createDB(const char* filename) {
  remove(filename);

  sqlite3_open(filename, &db);

//sqlite3_exec(db,
  dbExec(
"CREATE TABLE nod ("
"          id             integer primary key,"
"          lat            real not null,"
"          lon            real not null"
"        );"
""
"CREATE TABLE nod_way ("
"          way_id         integer not null,"
"          nod_id         integer not null,"
"          order_         integer not null"
"        );"
""
"CREATE table rel_mem ("
"          rel_of  integer not null,"
"          order_  integer not null,"
"          nod_id  integer,"
"          way_id  integer,"
"          rel_id  integer,"
"          rol     text"
");"
// "CREATE TABLE nod_rel ("
// "          nod_id         integer not null,"
// "          rel_of         integer null,"
// "          rol            text"
// "        );"
// ""
// "CREATE TABLE way_rel ("
// "          way_id         integer not null,"
// "          rel_of         integer null,"
// "          rol            text"
// "        );"
// ""
// "CREATE TABLE rel_rel ("
// "          rel_id         integer not null,"
// "          rel_of         integer null,"
// "          rol            text"
// "        );"
// ""
"CREATE TABLE tag("
"          nod_id         integer null,"
"          way_id         integer null,"
"          rel_id         integer null,"
"          key            text not null,"
"          val            text not null"
"        );"
);
//  NULL, NULL, NULL);

}

sqlite3_stmt* prepareStatement(const char* sql) {
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) {
    printf("Could not prepare %s\n", sql);
    exit(-1);
  }
  return stmt;
}

void prepareStatements() {
    stmt_ins_nod          = prepareStatement("insert into nod values(?, ?, ?)");
    stmt_ins_nod_way      = prepareStatement("insert into nod_way(way_id, nod_id, order_) values (?, ?, ?)");
    stmt_ins_rel_mem_nod  = prepareStatement("insert into rel_mem (rel_of, order_, nod_id, rol) values (?, ?, ?, ?)");
    stmt_ins_rel_mem_way  = prepareStatement("insert into rel_mem (rel_of, order_, way_id, rol) values (?, ?, ?, ?)");
    stmt_ins_rel_mem_rel  = prepareStatement("insert into rel_mem (rel_of, order_, rel_id, rol) values (?, ?, ?, ?)");
    stmt_ins_tag_nod      = prepareStatement("insert into tag (nod_id, key, val) values (?, ?, ?)");
    stmt_ins_tag_way      = prepareStatement("insert into tag (way_id, key, val) values (?, ?, ?)");
    stmt_ins_tag_rel      = prepareStatement("insert into tag (rel_id, key, val) values (?, ?, ?)");

}

int init_readosm(const char* filename_pbf) {
    const void *osm_handle;
    int ret;

//  if (argc != 2) {
//    fprintf (stderr, "usage: test_osm1 path-to-OSM-file\n");
//    return -1;
//  }
    ret = readosm_open (filename_pbf, &osm_handle);
    if (ret != READOSM_OK) {
      fprintf (stderr, "OPEN error: %d\n", ret);
      goto stop;
    }

    ret = readosm_parse (osm_handle, (const void *) 0, callback_node, callback_way, callback_relation);
    if (ret != READOSM_OK) {
      fprintf (stderr, "PARSE error: %d\n", ret);
      goto stop;
    }

    fprintf (stderr, "Ok, OSM input file successfully parsed\n");

  stop:

    readosm_close (osm_handle);
    return 0;
}

void createIndexes() {

  dbExec("create index nod_way_ix_way_id on nod_way(way_id  )");
  dbExec("create index tag_ix_val        on tag    (     val)");
  dbExec("create index tag_ix_key_val    on tag    (key, val)");
  dbExec("create index tag_ix_nod_id     on tag    (nod_id  )");
  dbExec("create index tag_ix_way_id     on tag    (way_id  )");
  dbExec("create index tag_ix_rel_id     on tag    (rel_id  )");
  dbExec("create index rel_mem_ix_rel_of on rel_mem(rel_of  )");


}

int main (int argc, char *argv[]) {
  const char* filename_db  = "/home/rene/github/github/OpenStreetMap/db/ch.db";
  const char* filename_pbf = "/home/rene/github/github/OpenStreetMap/pbf/ch.pbf";

  createDB(filename_db);
  prepareStatements(db);

  sqlite3_exec(db, "begin transaction", NULL, NULL, NULL);
  init_readosm(filename_pbf);
  sqlite3_exec(db, "commit transaction", NULL, NULL, NULL);

  createIndexes();
}
