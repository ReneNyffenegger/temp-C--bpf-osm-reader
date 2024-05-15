drop table if exists count_1_tag_keys;

.timer on
create table count_1_tag_keys as
select
   k,
   v,
   nod_id,
   way_id,
   rel_id
from
   tag not indexed
where
  (rel_id is null or rel_id not in (8131479))   and
   k not like 'name:%'       and
   k not like 'old_name:%'   and
   k not like 'short_name:%' and
   k not like 'official_name:%'
group by
   k
having
   count(*) = 1;
