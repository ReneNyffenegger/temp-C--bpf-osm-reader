select
   count(*),
   k
from
   tag
group by
   k
order by
   count(*);
 
