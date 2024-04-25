select
   v,
   round(count(*) * length(v)/1024/1024, 1) req_chars,
   count(*)
from
   tag
group by
   v
order by
   count(*) * length(v) desc
limit
   100;
