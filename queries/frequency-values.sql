select
   count(*) cnt,
   v
from
   tag
group by
   v
having
   count(*) > 100000
order by
   count(*);

