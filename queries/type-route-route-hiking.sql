select              -- type should be reserved for relations
   count(*),
   tag.k
from
   tag tag_type                                                   join
   tag tag_route  on tag_type.rel_id = tag_route.rel_id           join
   tag tag        on tag.rel_id      = tag_route.rel_id
where
   tag_type.k  = 'type'  and tag_type.v  = 'route' and
   tag_route.k = 'route' and tag_route.v = 'hiking'
group by
   tag.k 
order by
   count(*) desc limit 20;
