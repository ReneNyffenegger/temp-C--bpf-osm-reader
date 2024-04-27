select
   count(*),
   v
from
   tag
where
   k = 'network'
group by
   v
order by
   count(*);

