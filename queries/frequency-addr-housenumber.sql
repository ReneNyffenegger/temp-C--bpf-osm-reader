select
   count(*) cnt,
   v
from
   tag
where
   k = 'addr:housenumber'
group by
   v
order by
   count(*);
