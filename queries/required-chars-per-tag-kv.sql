select
   k,
   round(count(*) * avg(length(v))/1024/1024,2) req_chars,
   count(*)
from
   tag
group by
   k
order by
   req_chars desc
limit
   100;
