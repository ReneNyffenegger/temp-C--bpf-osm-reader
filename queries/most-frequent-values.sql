select
   count(*) cnt,
   v
from
   tag
group by
   v
order by
   count(*) desc
limit 100;
