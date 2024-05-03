create table count_1_tag_keys as
select
   k,
   v,
   nod_id,
   way_id,
   rel_id
from
   tag
group by
   k
having
   count(*) = 1;
