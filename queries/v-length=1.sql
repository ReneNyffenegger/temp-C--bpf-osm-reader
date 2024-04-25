-- select * from tag where length(v) = 1 and k not in ('addr:housenumber', 'layer', 'capacity', 'level', 'public_transport:version', 'ref',
-- 'local_ref', 'rapids', 'openGeoDB:layer', 'seats', 'stars', 'screen', 'admin_level') limit 100;

select
   count(*),
   k, v
from
   tag
where
   length(v) = 1
group by
   k, v
order by
  count(*) limit 200;
