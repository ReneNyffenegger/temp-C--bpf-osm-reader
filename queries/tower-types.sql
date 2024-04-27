select
   count(*),
   v
from
   tag
where
   k = 'tower:type'
group by
   v
order by
   count(*);
