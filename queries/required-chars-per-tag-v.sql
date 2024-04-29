select
   v,
   printf('%8.3f', round(count(*) * length(v)/1024.0/1024.0, 3)) req_chars,
   printf('%9d', count(*)) cnt
from
   tag
group by
   v
order by
   count(*) * length(v) desc
limit
   60;
