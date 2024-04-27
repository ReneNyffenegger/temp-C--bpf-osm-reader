select              -- type should be reserved for relations
   count(*),
   v
from
   tag
where
   nod_id is not null and
   k = 'type'
group by
   v
order by
   count(*) desc limit 20;

.print ways

select              -- type should be reserved for relations
   count(*),
   v
from
   tag
where
   way_id is not null and
   k = 'type'
group by
   v
order by
   count(*) desc limit 20;

.print relations

select              -- type should be reserved for relations
   count(*),
   v
from
   tag
where
   rel_id is not null and
   k = 'type'
group by
   v
order by
   count(*) desc limit 20;
