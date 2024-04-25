select
   t.nod_id, t.way_id, t.rel_id, t.k, t.v
from
  (select count(*), k from tag group by k order by count(*) limit 1000) s join tag t on s.k = t.k;
