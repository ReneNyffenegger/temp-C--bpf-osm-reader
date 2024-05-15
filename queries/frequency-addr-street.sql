select
   count(*) cnt,
   v
from
   tag
where
   k = 'addr:street'
group by
   v
order by
   count(*);
