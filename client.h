//
// Functions that need to be implemented by the client.
//

#include <time.h>

void osm_node(
       unsigned long long id,
       double             lat,
       double             lon,
       time_t             ts,
       unsigned int       version,
       unsigned long long changeset,
       int                uid,
       const char        *user,
       int                visible
);

void osm_node_key_value(
       unsigned long long id,
       const char        *key,
       const char        *value
);

void osm_way(
       unsigned long long id,
       time_t             ts,
       unsigned int       version,
       unsigned long long changeset,
       int                uid,
       const char        *user,
       int                visible
);

void osm_way_node_id(
       unsigned long long way_id,
       unsigned long long nod_id,
       int                nod_pos
);  

void osm_way_key_val(
   unsigned long long way_id,
   const char        *key,
   const char        *val
);

void osm_rel(
       unsigned long long id,
       time_t             ts,
       unsigned int       version,
       unsigned long long changeset,
       int                uid,
       const char        *user,
       int                visible
);

void osm_rel_member(
   unsigned long long  rel_id,
   unsigned long long  elem_id,
   const char         *role,
   int                 type,  // 0: Node, 1 = Way, 2= Relation
   int                 elem_pos
);

void osm_rel_key_val(
   unsigned long long rel_id,
   const char        *key,
   const char        *val
);
