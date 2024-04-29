select
   k,
   printf('%8.3f', round(count(*) * length(k)/1024.0/1024.1,3)) req_chars,
   printf('%9d', count(*)) cnt
from
   tag
group by
   k
order by
   count(*) * length(k) desc
limit
   60;
