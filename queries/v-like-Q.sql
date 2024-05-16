select
   count(*),
   k
from
   tag
where
   v glob 'Q*'
group by
   k
order by
   count(*);
