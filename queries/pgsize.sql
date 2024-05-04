select
   name,
   printf('%5.2f', sum(pgsize) / 1024.0 / 1024.0 / 1024.0) gb
from
   dbstat
group by
   name
order by
   sum(pgsize);
