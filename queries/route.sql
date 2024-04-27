select
   tag_type.rel_id,
   tag_name.v,
   tag_route.v,
   tag_network.v,
   tag_osmc.v,
   tag_operator.v
from
   tag   tag_type                                                                                        left join
   tag   tag_route         on tag_route.rel_id     = tag_type.rel_id and tag_route.k     = 'route'       left join
   tag   tag_network       on tag_network.rel_id   = tag_type.rel_id and tag_network.k   = 'network'     left join
   tag   tag_name          on tag_name.rel_id      = tag_type.rel_id and tag_name.k      = 'name'        left join
   tag   tag_osmc          on tag_osmc.rel_id      = tag_type.rel_id and tag_osmc.k      = 'osmc:symbol' left join
   tag   tag_operator      on tag_operator.rel_id  = tag_type.rel_id and tag_operator.k  = 'operator' /*   left join
-- tag   tag       on tag.rel_id   = tag_type .rel_id */
where
   tag_type.k = 'type' and tag_type .v = 'route'
   and tag_route.v = 'hiking'
;
--group by
--   tag.k
--order by
--   count(*);
-- 2510|description
-- 3050|operator:wikidata
-- 4129|duration
-- 4800|gtfs:release_date
-- 5079|colour
-- 5147|ref_trips
-- 5160|gtfs:trip_id:sample
-- 5162|gtfs:trip_id:like
-- 5220|gtfs:feed
-- 5262|gtfs:route_id
-- 5579|nat_ref
-- 5635|network:wikidata
-- 7132|osmc:name
-- 7142|note
-- 7156|public_transport:version
-- 10001|ref
-- 15389|symbol
-- 16434|to
-- 16682|from
