select
   count(*) cnt,
   k
from
   tag
group by
   k
order by
   count(*) desc
limit 100;
