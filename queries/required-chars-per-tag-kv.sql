select
   k,
   printf('%8.3f', round(count(*) * length(k) * length(v)/1024.0/1024.1,3)) req_chars,
   printf('%9d', count(*)) cnt
from
   tag
group by
   k
order by
   req_chars desc
limit
   60;
