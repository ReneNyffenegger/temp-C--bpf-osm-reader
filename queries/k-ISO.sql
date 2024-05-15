select
   count(*) cnt,
   k
from
   tag
where
   k glob 'ISO*'
group by
   k
order by
   count(*);
