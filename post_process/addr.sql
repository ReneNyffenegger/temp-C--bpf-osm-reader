-- explain

create table addr as
select
-- nod_id, way_id, rel_id,
           way_id,
   max(case when k='name'             then v end) name,
   max(case when k='addr:street'      then v end) street,
   max(case when k='addr:housenumber' then v end) nousenumber,
   max(case when k='addr:postcode'    then v end) postcode,
   max(case when k='addr:city'        then v end) city,
   max(case when k='addr:country'     then v end) country
from
   tag
where
            way_id          in (
      select
            way_id
      from
         tag
      where
         way_id is not null and
         k = 'addr:street'
   )
group by
            way_id
;

-- create table addr as
-- select
--    nod_id, way_id, rel_id,
--    max(case when k='name'             then v end) name,
--    max(case when k='addr:street'      then v end) street,
--    max(case when k='addr:housenumber' then v end) nousenumber,
--    max(case when k='addr:postcode'    then v end) postcode,
--    max(case when k='addr:city'        then v end) city,
--    max(case when k='addr:country'     then v end) country
-- from
--    tag not indexed
-- where
--    k in ('addr:street', 'addr:housenumber', 'addr:postcode', 'addr:city', 'addr:country')
-- group by
--    nod_id, way_id, rel_id;

