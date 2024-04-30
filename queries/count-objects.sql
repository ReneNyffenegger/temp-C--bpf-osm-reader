--
--    https://taginfo.osm.ch/reports/database_statistics
--
select "Number of nodes:    " txt, count(         *     ) cnt from nod     union all
select "Number of ways:     " txt, count(distinct way_id) cnt from nod_way union all
select "Number of relations:" txt, count(distinct rel_id) cnt from rel_mem union all
select "Number of tags:     " txt, count(         *     ) cnt from tag
;
