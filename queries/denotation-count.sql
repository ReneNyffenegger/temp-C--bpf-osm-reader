select
   count(nod_id),
   count(way_id),
   count(rel_id)
from
   tag
where
   k = 'denotation'
;
