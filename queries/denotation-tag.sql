select
   count(*),
   oth.k
  ,oth.v
from
   tag den   join
   tag oth on den.nod_id = oth.nod_id and oth.k <> 'denotation'
where
   den.k = 'denotation'
-- and oth.k = 'natural'
-- and oth.k = 'leaf_type'  -- boradleaved, needleleaved, leafless
-- and oth.k = 'leaf_cycle'
-- and oth.k = 'source'
-- and oth.k = 'diameter'
-- and oth.k = 'species'
-- and oth.k = 'species:wikipedia'
-- and oth.k = 'genus'
-- and oth.k = 'taxon'
-- and oth.k = 'note'
   and oth.k = 'description'
group by
   oth.k
  ,oth.v
order by
   count(*) desc
--limit 60
;

