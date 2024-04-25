select
   way_id,
   count(*) cnt
from
   nod_way
group by
   way_id
order by
   cnt desc
limit 100;
