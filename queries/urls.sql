select
   nod_id,
   way_id,
   rel_id,
   k,v
from
   tag
where
   lower(v) like 'http%';
