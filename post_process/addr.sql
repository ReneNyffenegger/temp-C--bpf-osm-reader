-- explain

drop  table addr;

create table addr (
   nod_id bigint,
   way_id bigint,
   rel_id bigint,
   name        text,
   street      text,
   housenumber text,
   postcode    text,
   city        text,
   country     text
);

-- create table addr as
insert into addr
select
-- nod_id, way_id, rel_id,
   null,   way_id, null,
   max(case when k='name'             then v end) name,
   max(case when k='addr:street'      then v end) street,
   max(case when k='addr:housenumber' then v end) housenumber,
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

insert into addr
select
   nod_id, null, null,
   max(case when k='name'             then v end) name,
   max(case when k='addr:street'      then v end) street,
   max(case when k='addr:housenumber' then v end) housenumber,
   max(case when k='addr:postcode'    then v end) postcode,
   max(case when k='addr:city'        then v end) city,
   max(case when k='addr:country'     then v end) country
from
   tag
where
            nod_id          in (
      select
            nod_id
      from
         tag
      where
         nod_id is not null and
         k = 'addr:street'
   )
group by
            way_id
;


insert into addr
select
   null, null, rel_id,
   max(case when k='name'             then v end) name,
   max(case when k='addr:street'      then v end) street,
   max(case when k='addr:housenumber' then v end) housenumber,
   max(case when k='addr:postcode'    then v end) postcode,
   max(case when k='addr:city'        then v end) city,
   max(case when k='addr:country'     then v end) country
from
   tag
where
            rel_id          in (
      select
            rel_id
      from
         tag
      where
         rel_id is not null and
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

