select
   count(*),
-- street,
   postcode,
   city
from
   addr
group by
-- street,
   postcode,
   city
order by
   count(*)
limit 1000;
