#include <stdio.h>
#include "readosm.h"

static int callback_node (const void *user_data, const readosm_node * node) {
    char buf[128];
    int i;
    const readosm_tag *tag;

    if (user_data != NULL)
    user_data = NULL;    /* silencing stupid compiler warnings */

#if defined(_WIN32) || defined(__MINGW32__)
    /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
    sprintf (buf, "%I64d", node->id);
#else
    sprintf (buf, "%lld", node->id);
#endif
    printf ("\t<node id=\"%s\"", buf);

/*
* some individual values may be set, or may be not
* unset values are identified by the READOSM_UNDEFINED
* conventional value, and must be consequently ignored
*/
    if (node->latitude != READOSM_UNDEFINED)
    printf (" lat=\"%1.7f\"", node->latitude);

    if (node->longitude != READOSM_UNDEFINED)
    printf (" lon=\"%1.7f\"", node->longitude);

    if (node->version != READOSM_UNDEFINED)
    printf (" version=\"%d\"", node->version);

    if (node->changeset != READOSM_UNDEFINED) {

#if defined(_WIN32) || defined(__MINGW32__)
      /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
      sprintf (buf, "%I64d", node->changeset);
#else
      sprintf (buf, "%lld", node->changeset);
#endif
      printf (" changeset=\"%s\"", buf);
   }

/*
* unset string values are identified by a NULL pointer
* and must be consequently ignored
*/
    if (node->user != NULL)
    printf (" user=\"%s\"", node->user);

    if (node->uid != READOSM_UNDEFINED)
    printf (" uid=\"%d\"", node->uid);

    if (node->timestamp != NULL)
    printf (" timestamp=\"%s\"", node->timestamp);

/*
* the Node object may have its own tag list
* please note: this one is a variable-length list,
* and may be empty: in this case tag_count will be ZERO
*/
    if (node->tag_count == 0) {
       printf (" />\n");
    }
    else {
      printf (">\n");
      for (i = 0; i < node->tag_count; i++) {
        /* we'll now print each <tag> for this node */
        tag = node->tags + i;
        printf ("\t\t<tag k=\"%s\" v=\"%s\" />\n", tag->key,
            tag->value);
        }
      printf ("\t</node>\n");
    }
    return READOSM_OK;
}

static int callback_way (const void *user_data, const readosm_way * way) {
    char buf[128];
    int i;
    const readosm_tag *tag;

    if (user_data != NULL)
    user_data = NULL;    /* silencing stupid compiler warnings */

#if defined(_WIN32) || defined(__MINGW32__)
    /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
    sprintf (buf, "%I64d", way->id);
#else
    sprintf (buf, "%lld", way->id);
#endif
    printf ("\t<way id=\"%s\"", buf);

/*
* some individual values may be set, or may be not
* unset values are identified by the READOSM_UNDEFINED
* conventional value, and must be consequently ignored
*/
    if (way->version != READOSM_UNDEFINED) {
      printf (" version=\"%d\"", way->version);
    }
    if (way->changeset != READOSM_UNDEFINED) {
#if defined(_WIN32) || defined(__MINGW32__)
      /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
      sprintf (buf, "%I64d", way->changeset);
#else
      sprintf (buf, "%lld", way->changeset);
#endif
      printf (" changeset=\"%s\"", buf);
    }

/*
* unset string values are identified by a NULL pointer
* and must be consequently ignored
*/
    if (way->user != NULL) {
      printf (" user=\"%s\"", way->user);
    }
    if (way->uid != READOSM_UNDEFINED) {
      printf (" uid=\"%d\"", way->uid);
    }
    if (way->timestamp != NULL) {
      printf (" timestamp=\"%s\"", way->timestamp);
    }
/*
* the Way object may have a noderefs-list and a tag-list
* please note: these are variable-length lists, and may 
* be empty: in this case the corresponding item count 
* will be ZERO
*/
    if (way->tag_count == 0 && way->node_ref_count == 0) {
      printf (" />\n");
    }
    else {
      printf (">\n");
      for (i = 0; i < way->node_ref_count; i++) {
        /* we'll now print each <nd ref> for this way */
#if defined(_WIN32) || defined(__MINGW32__)
        /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
        sprintf (buf, "%I64d", *(way->node_refs + i));
#else
        sprintf (buf, "%lld", *(way->node_refs + i));
#endif
        printf ("\t\t<nd ref=\"%s\" />\n", buf);
      }
      for (i = 0; i < way->tag_count; i++) {
        /* we'll now print each <tag> for this way */
        tag = way->tags + i;
        printf ("\t\t<tag k=\"%s\" v=\"%s\" />\n", tag->key,
            tag->value);
      }
      printf ("\t</way>\n");
    }
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

    if (user_data != NULL)
    user_data = NULL;    /* silencing stupid compiler warnings */

#if defined(_WIN32) || defined(__MINGW32__)
    /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
    sprintf (buf, "%I64d", relation->id);
#else
    sprintf (buf, "%lld", relation->id);
#endif
    printf ("\t<relation id=\"%s\"", buf);

/*
* some individual values may be set, or may be not
* unset values are identified by the READOSM_UNDEFINED
* conventional value, and must be consequently ignored
*/
    if (relation->version != READOSM_UNDEFINED) {
      printf (" version=\"%d\"", relation->version);
    }
    if (relation->changeset != READOSM_UNDEFINED) {
#if defined(_WIN32) || defined(__MINGW32__)
      /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
      sprintf (buf, "%I64d", relation->changeset);
#else
      sprintf (buf, "%lld", relation->changeset);
#endif
      printf (" changeset=\"%s\"", buf);
    }

/*
* unset string values are identified by a NULL pointer
* and must be consequently ignored
*/
    if (relation->user != NULL) {
      printf (" user=\"%s\"", relation->user);
    }
    if (relation->uid != READOSM_UNDEFINED) {
      printf (" uid=\"%d\"", relation->uid);
    }
    if (relation->timestamp != NULL) {
      printf (" timestamp=\"%s\"", relation->timestamp);
    }

/*
* the Relation object may have a member-list and a tag-list
* please note: these are variable-length lists, and may 
* be empty: in this case the corresponding item count 
* will be ZERO
*/
    if (relation->tag_count == 0 && relation->member_count == 0) {
      printf (" />\n");
    }
    else {
      printf (">\n");
      for (i = 0; i < relation->member_count; i++)
        {
        /* we'll now print each <member> for this way */
        member = relation->members + i;
#if defined(_WIN32) || defined(__MINGW32__)
        /* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
        sprintf (buf, "%I64d", member->id);
#else
        sprintf (buf, "%lld", member->id);
#endif
        /* any <member> may be of "node", "way" or "relation" type */
        switch (member->member_type)
          {
          case READOSM_MEMBER_NODE:
              printf ("\t\t<member type=\"node\" ref=\"%s\"", buf);
              break;
          case READOSM_MEMBER_WAY:
              printf ("\t\t<member type=\"way\" ref=\"%s\"", buf);
              break;
          case READOSM_MEMBER_RELATION:
              printf ("\t\t<member type=\"relation\" ref=\"%s\"", buf);
              break;
          default:
              printf ("\t\t<member ref=\"%s\"", buf);
              break;
        };
        if (member->role != NULL) {
            printf (" role=\"%s\" />\n", member->role);
        }
        else {
            printf (" />\n");
        }
      }
      for (i = 0; i < relation->tag_count; i++) {
        /* we'll now print each <tag> for this way */
        tag = relation->tags + i;
        printf ("\t\t<tag k=\"%s\" v=\"%s\" />\n", tag->key,
            tag->value);
        }
      printf ("\t</relation>\n");
    }
    return READOSM_OK;
}

int main (int argc, char *argv[]) {
    const void *osm_handle;
    int ret;

    if (argc != 2) {
      fprintf (stderr, "usage: test_osm1 path-to-OSM-file\n");
      return -1;
    }
    ret = readosm_open (argv[1], &osm_handle);
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
