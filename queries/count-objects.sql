--
--    https://taginfo.osm.ch/reports/database_statistics
--
select "Number of nodes:    ", count(         *     ) cnt_nod from nod     union all
select "Number of ways:     ", count(distinct way_id) cnt_way from nod_way union all
select "Number of relations:", count(distinct rel_id) cnt_rel from rel_mem union all
select "Number of tags:     ", count(         *     ) cnt_tag from tag
;
