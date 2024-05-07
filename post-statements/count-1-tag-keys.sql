drop   table count_1_tag_keys;
create table count_1_tag_keys as
select
   k,
   v,
   nod_id,
   way_id,
   rel_id
from
   tag
where
   rel_id not in (8131479)   and
   k not like 'name:%'       and
   k not like 'old_name:%'   and
   k not like 'short_name:%' and
   k not like 'official_name:%'
group by
   k
having
   count(*) = 1;
