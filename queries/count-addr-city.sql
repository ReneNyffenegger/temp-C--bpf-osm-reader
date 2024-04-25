select
   count(*) cnt,
   v
from
   tag
where
   k = 'addr:city'
group by
   v
order by
   cnt desc limit 200;
