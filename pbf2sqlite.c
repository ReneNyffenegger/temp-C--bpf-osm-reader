#include <stdio.h>
#include <stdlib.h>

#ifdef PBF2SQLITE
#include <sqlite3.h>
#elif defined PBF2MYSQL
#include "mysql.h"
#else
#error neither PBF2SQLITE nor PBF2MYSQL defined
#endif

#include "readosm.h"

#ifdef PBF2SQLITE

    sqlite3      *db;
    sqlite3_stmt* stmt_ins_nod;
    sqlite3_stmt* stmt_ins_nod_way;
    sqlite3_stmt* stmt_ins_rel_mem_nod;
    sqlite3_stmt* stmt_ins_rel_mem_way;
    sqlite3_stmt* stmt_ins_rel_mem_rel;
    sqlite3_stmt* stmt_ins_tag_nod;
    sqlite3_stmt* stmt_ins_tag_way;
    sqlite3_stmt* stmt_ins_tag_rel;

#elif defined PBF2MYSQL

    MYSQL        *db;

    MYSQL_STMT   *stmt_ins_nod         ; MYSQL_BIND  bind_ins_nod         [3];
    MYSQL_STMT   *stmt_ins_nod_way     ; MYSQL_BIND  bind_ins_nod_way     [3];
    MYSQL_STMT   *stmt_ins_rel_mem_nod ; MYSQL_BIND  bind_ins_rel_mem_nod [4];
    MYSQL_STMT   *stmt_ins_rel_mem_way ; MYSQL_BIND  bind_ins_rel_mem_way [4];
    MYSQL_STMT   *stmt_ins_rel_mem_rel ; MYSQL_BIND  bind_ins_rel_mem_rel [4];
    MYSQL_STMT   *stmt_ins_tag_nod     ; MYSQL_BIND  bind_ins_tag_nod     [3];
    MYSQL_STMT   *stmt_ins_tag_way     ; MYSQL_BIND  bind_ins_tag_way     [3];
    MYSQL_STMT   *stmt_ins_tag_rel     ; MYSQL_BIND  bind_ins_tag_rel     [3];

#endif


void dbExec(const char* sql) {
#ifdef PBF2SQLITE
  if (sqlite3_exec(db, sql, NULL, NULL, NULL)) {
     printf("Could not exec %s\n", sql);
     exit(-1);
  }
#elif defined PBF2MYSQL
  if (mysql_query(db, sql)) {
    fprintf(stderr, "Error:     %s\nStatement: %s\n", mysql_error(db), sql);
    mysql_close(db);
    exit(1);
  }

#endif
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


#ifdef PBF2SQLITE
   sqlite3_bind_int64 (stmt_ins_nod, 1, node ->id);
   sqlite3_bind_double(stmt_ins_nod, 2, node ->latitude);
   sqlite3_bind_double(stmt_ins_nod, 3, node ->longitude);
   sqlite3_step       (stmt_ins_nod);
   sqlite3_reset      (stmt_ins_nod);

#elif defined PBF2MYSQL

#endif

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

#ifdef PBF2SQLITE
        sqlite3_bind_int64(stmt_ins_tag_nod, 1, node->id  );
        sqlite3_bind_text (stmt_ins_tag_nod, 2, tag->key  , -1, NULL);
        sqlite3_bind_text (stmt_ins_tag_nod, 3, tag->value, -1, NULL);
        sqlite3_step      (stmt_ins_tag_nod);
        sqlite3_reset     (stmt_ins_tag_nod);
#elif defined PBF2MYSQL
#endif

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
#ifdef PBF2SQLITE
         sqlite3_bind_int64(stmt_ins_nod_way, 1, way->id);
         sqlite3_bind_int64(stmt_ins_nod_way, 2, *(way->node_refs+i)); // TODO: Should this be checked for NULL?
         sqlite3_bind_int  (stmt_ins_nod_way, 3, i);
         sqlite3_step      (stmt_ins_nod_way);
         sqlite3_reset     (stmt_ins_nod_way);
#elif defined PBF2MYSQL
#endif
      }

      for (i = 0; i < way->tag_count; i++) {
          tag = way->tags + i;
#ifdef PBF2SQLITE
          sqlite3_bind_int64(stmt_ins_tag_way, 1, way->id  );
          sqlite3_bind_text (stmt_ins_tag_way, 2, tag->key  , -1, NULL);
          sqlite3_bind_text (stmt_ins_tag_way, 3, tag->value, -1, NULL);
          sqlite3_step      (stmt_ins_tag_way);
          sqlite3_reset     (stmt_ins_tag_way);
#elif defined PBF2MYSQL
#endif

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
#ifdef PBF2SQLITE
              sqlite3_bind_int64(stmt_ins_rel_mem_nod, 1, relation->id);
              sqlite3_bind_int  (stmt_ins_rel_mem_nod, 2, i);
              sqlite3_bind_int64(stmt_ins_rel_mem_nod, 3, member->id);
              sqlite3_bind_text (stmt_ins_rel_mem_nod, 4, member->role, -1, NULL); // TODO: should be checked for NULL?
              sqlite3_step      (stmt_ins_rel_mem_nod);
              sqlite3_reset     (stmt_ins_rel_mem_nod);
#elif defined PBF2MYSQL
#endif
              break;

          case READOSM_MEMBER_WAY:
//            printf ("\t\t<member type=\"way\" ref=\"%s\"", buf);
#ifdef PBF2SQLITE
              sqlite3_bind_int64(stmt_ins_rel_mem_way, 1, relation->id);
              sqlite3_bind_int  (stmt_ins_rel_mem_way, 2, i);
              sqlite3_bind_int64(stmt_ins_rel_mem_way, 3, member->id);
              sqlite3_bind_text (stmt_ins_rel_mem_way, 4, member->role, -1, NULL); // TODO: should be checked for NULL?
              sqlite3_step      (stmt_ins_rel_mem_way);
              sqlite3_reset     (stmt_ins_rel_mem_way);
#elif defined PBF2MYSQL
#endif
              break;

          case READOSM_MEMBER_RELATION:
//            printf ("\t\t<member type=\"relation\" ref=\"%s\"", buf);
#ifdef PBF2SQLITE
              sqlite3_bind_int64(stmt_ins_rel_mem_rel, 1, relation->id);
              sqlite3_bind_int  (stmt_ins_rel_mem_rel, 2, i);
              sqlite3_bind_int64(stmt_ins_rel_mem_rel, 3, member->id);
              sqlite3_bind_text (stmt_ins_rel_mem_rel, 4, member->role, -1, NULL); // TODO: should be checked for NULL?
              sqlite3_step      (stmt_ins_rel_mem_rel);
              sqlite3_reset     (stmt_ins_rel_mem_rel);
#elif defined PBF2MYSQL
#endif
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
#ifdef PBF2SQLITE
        sqlite3_bind_int64(stmt_ins_tag_rel, 1, relation->id);
        sqlite3_bind_text (stmt_ins_tag_rel, 2, tag->key  , -1, NULL);
        sqlite3_bind_text (stmt_ins_tag_rel, 3, tag->value, -1, NULL);
        sqlite3_step      (stmt_ins_tag_rel);
        sqlite3_reset     (stmt_ins_tag_rel);
#elif defined PBF2MYSQL
#endif

////      printf ("\t\t<tag k=\"%s\" v=\"%s\" />\n", tag->key,
////          tag->value);
      }
////      printf ("\t</relation>\n");
//     }
    return READOSM_OK;
}


void createDB(const char* name) {
#ifdef PBF2SQLITE
  remove(name);
  sqlite3_open(filename, &db);
#elif defined PBF2MYSQL
   dbExec("drop database if exists osm_ch");  // TODO: osm_ch should of course not be hard coded.
   dbExec("create database osm_ch");
   dbExec("use osm_ch");
#endif

//sqlite3_exec(db,
  dbExec(
"CREATE TABLE nod ("
"          id             integer primary key,"
"          lat            double not null, -- real not null,"
"          lon            double not null  -- real not null"
"        )"
  );

  dbExec ( 
"CREATE TABLE nod_way ("
"          way_id         integer not null,"
"          nod_id         integer not null,"
"          order_         integer not null"
"        )"
  );

  dbExec (
"CREATE table rel_mem ("
"          rel_of  integer not null,"
"          order_  integer not null,"
"          nod_id  integer,"
"          way_id  integer,"
"          rel_id  integer,"
"          rol     varchar(1024)  -- text"
")"
  );

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

   dbExec(
"CREATE TABLE tag("
"          nod_id         integer null,"
"          way_id         integer null,"
"          rel_id         integer null,"
"          key            varchar(1024), -- text not null,"
"          val            varchar(1024)  -- text not null"
"        )"
);
//  NULL, NULL, NULL);

}

#ifdef PBF2SQLITE 
sqlite3_stmt* prepareStatement(const char* sql) {
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) {
    printf("Could not prepare %s\n", sql);
    exit(-1);
  }
  return stmt;
}
#elif defined PBF2MYSQL
MYSQL_STMT *prepareStatement(const char* sql) {
  MYSQL_STMT *stmt;
  stmt = mysql_stmt_init(db);

  if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
    printf("Could not prepare %s\n", sql);
    exit(-1);
  }
  return stmt;
}
#endif 

#if defined PBF2MYSQL

    int    ins_nod__nod_id;
    double ins_nod__lat;
    double ins_nod__lon;

    int  ins_nod_way__way_id    ;
    int  ins_nod_way__nod_id    ;
    int  ins_nod_way__order_    ;

    int  ins_rel_mem_nod__rel_of;
    int  ins_rel_mem_nod__order_;
    int  ins_rel_mem_nod__nod_id;
    char ins_rel_mem_nod__rol   [1000];

    int  ins_rel_mem_way__rel_of;
    int  ins_rel_mem_way__order_;
    int  ins_rel_mem_way__nod_id;
    char ins_rel_mem_way__rol   [1000];


    int  ins_rel_mem_rel__rel_of;
    int  ins_rel_mem_rel__order_;
    int  ins_rel_mem_rel__nod_id;
    char ins_rel_mem_rel__rol   [1000];

    int  ins_tag_nod__nod_id    ;
    char ins_tag_nod__key       [1000];
    char ins_tag_nod__val       [1000];

    int  ins_tag_way__way_id    ;
    char ins_tag_way__key       [1000];
    char ins_tag_way__val       [1000];

    int  ins_tag_ral__rel_id    ;
    char ins_tag_ral__key       [1000];
    char ins_tag_ral__val       [1000];

#endif


void prepareStatements() {

    stmt_ins_nod          = prepareStatement("insert into nod (id, lat, lon) values(?, ?, ?)");
    stmt_ins_nod_way      = prepareStatement("insert into nod_way(way_id, nod_id, order_) values (?, ?, ?)");
    stmt_ins_rel_mem_nod  = prepareStatement("insert into rel_mem (rel_of, order_, nod_id, rol) values (?, ?, ?, ?)");
    stmt_ins_rel_mem_way  = prepareStatement("insert into rel_mem (rel_of, order_, way_id, rol) values (?, ?, ?, ?)");
    stmt_ins_rel_mem_rel  = prepareStatement("insert into rel_mem (rel_of, order_, rel_id, rol) values (?, ?, ?, ?)");
    stmt_ins_tag_nod      = prepareStatement("insert into tag (nod_id, key, val) values (?, ?, ?)");
    stmt_ins_tag_way      = prepareStatement("insert into tag (way_id, key, val) values (?, ?, ?)");
    stmt_ins_tag_rel      = prepareStatement("insert into tag (rel_id, key, val) values (?, ?, ?)");

#if defined PBF2MYSQL

    memset( bind_ins_nod         , 0, sizeof(bind_ins_nod         ));  bind_ins_nod         [0].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_nod         [0].buffer = (char*) &ins_nod__nod_id        ;
                                                                       bind_ins_nod         [1].buffer_type = MYSQL_TYPE_DOUBLE; bind_ins_nod         [1].buffer = (char*) &ins_nod__lat           ;
                                                                       bind_ins_nod         [2].buffer_type = MYSQL_TYPE_DOUBLE; bind_ins_nod         [2].buffer = (char*) &ins_nod__lon           ;

    memset( bind_ins_nod_way     , 0, sizeof(bind_ins_nod_way     ));  bind_ins_nod_way     [0].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_nod_way     [0].buffer = (char*) &ins_nod_way__way_id    ;
                                                                       bind_ins_nod_way     [1].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_nod_way     [1].buffer = (char*) &ins_nod_way__nod_id    ;
                                                                       bind_ins_nod_way     [2].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_nod_way     [2].buffer = (char*) &ins_nod_way__order_    ;

    memset( bind_ins_rel_mem_nod , 0, sizeof(bind_ins_rel_mem_nod ));  bind_ins_rel_mem_nod [0].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_rel_mem_nod [0].buffer = (char*) &ins_rel_mem_nod__rel_of;
                                                                       bind_ins_rel_mem_nod [1].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_rel_mem_nod [1].buffer = (char*) &ins_rel_mem_nod__order_;
                                                                       bind_ins_rel_mem_nod [2].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_rel_mem_nod [2].buffer = (char*) &ins_rel_mem_nod__nod_id;
                                                                       bind_ins_rel_mem_nod [3].buffer_type = MYSQL_TYPE_STRING; bind_ins_rel_mem_nod [3].buffer = (char*) &ins_rel_mem_nod__rol   ;

    memset( bind_ins_rel_mem_way , 0, sizeof(bind_ins_rel_mem_way ));  bind_ins_rel_mem_way [0].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_rel_mem_way [0].buffer = (char*) &ins_rel_mem_way__rel_of;
                                                                       bind_ins_rel_mem_way [1].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_rel_mem_way [1].buffer = (char*) &ins_rel_mem_way__order_;
                                                                       bind_ins_rel_mem_way [2].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_rel_mem_way [2].buffer = (char*) &ins_rel_mem_way__nod_id;
                                                                       bind_ins_rel_mem_way [3].buffer_type = MYSQL_TYPE_STRING; bind_ins_rel_mem_way [3].buffer = (char*) &ins_rel_mem_way__rol   ;


    memset( bind_ins_rel_mem_rel , 0, sizeof(bind_ins_rel_mem_rel ));  bind_ins_rel_mem_rel [0].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_rel_mem_rel [0].buffer = (char*) &ins_rel_mem_rel__rel_of;
                                                                       bind_ins_rel_mem_rel [1].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_rel_mem_rel [1].buffer = (char*) &ins_rel_mem_rel__order_;
                                                                       bind_ins_rel_mem_rel [2].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_rel_mem_rel [2].buffer = (char*) &ins_rel_mem_rel__nod_id;
                                                                       bind_ins_rel_mem_rel [3].buffer_type = MYSQL_TYPE_STRING; bind_ins_rel_mem_rel [3].buffer = (char*) &ins_rel_mem_rel__rol   ;

    memset( bind_ins_tag_nod     , 0, sizeof(bind_ins_tag_nod     ));  bind_ins_tag_nod     [0].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_tag_nod     [0].buffer = (char*) &ins_tag_nod__nod_id    ;
                                                                       bind_ins_tag_nod     [1].buffer_type = MYSQL_TYPE_STRING; bind_ins_tag_nod     [1].buffer = (char*) &ins_tag_nod__key       ;
                                                                       bind_ins_tag_nod     [2].buffer_type = MYSQL_TYPE_STRING; bind_ins_tag_nod     [2].buffer = (char*) &ins_tag_nod__val       ;

    memset( bind_ins_tag_way     , 0, sizeof(bind_ins_tag_way     ));  bind_ins_tag_way     [0].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_tag_way     [0].buffer = (char*) &ins_tag_way__way_id    ;
                                                                       bind_ins_tag_way     [1].buffer_type = MYSQL_TYPE_STRING; bind_ins_tag_way     [1].buffer = (char*) &ins_tag_way__key       ;
                                                                       bind_ins_tag_way     [2].buffer_type = MYSQL_TYPE_STRING; bind_ins_tag_way     [2].buffer = (char*) &ins_tag_way__val       ;

    memset( bind_ins_tag_rel     , 0, sizeof(bind_ins_tag_rel     ));  bind_ins_tag_rel     [0].buffer_type = MYSQL_TYPE_LONG  ; bind_ins_tag_rel     [0].buffer = (char*) &ins_tag_ral__rel_id    ;
                                                                       bind_ins_tag_rel     [1].buffer_type = MYSQL_TYPE_STRING; bind_ins_tag_rel     [1].buffer = (char*) &ins_tag_ral__key       ;
                                                                       bind_ins_tag_rel     [2].buffer_type = MYSQL_TYPE_STRING; bind_ins_tag_rel     [2].buffer = (char*) &ins_tag_ral__val       ;

#endif
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

#ifdef PBF2SQLITE
  const char* dbName  = "/home/rene/github/github/OpenStreetMap/db/ch.db";
#elif defined PBF2MYSQL
  const char* dbName  = "osm_ch";
#endif
// const char* filename_pbf = "/home/rene/github/github/OpenStreetMap/pbf/ch.pbf";
// const char* filename_pbf = "../../github/OpenStreetMap/pbf/ch.pbf";
   const char* filename_pbf = "../../github/OpenStreetMap/pbf/li.pbf";

  createDB(dbName);

  prepareStatements(db);

//sqlite3_exec(db, "begin transaction", NULL, NULL, NULL);
  dbExec("begin transaction");
  init_readosm(filename_pbf);
//sqlite3_exec(db, "commit transaction", NULL, NULL, NULL);
  dbExec("commit transaction");

  createIndexes();
}
