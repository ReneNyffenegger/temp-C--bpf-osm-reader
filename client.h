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
