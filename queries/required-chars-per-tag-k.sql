select
   k,
   round(count(*) * length(k)/1024/1024,1) req_chars,
   count(*)
from
   tag
group by
   k
order by
   count(*) * length(k) desc
limit
   100;
