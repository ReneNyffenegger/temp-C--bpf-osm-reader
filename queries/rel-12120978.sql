select
   rel.nod_id,
   ref.v,
-- mrk.v,
   ans.v
from
   rel_mem  rel                                                    left join
   tag      ref on rel.nod_id = ref.nod_id and ref.k = 'ref'       left join
-- tag      mrk on rel.nod_id = mrk.nod_id and mrk.k = 'marker'    left join
   tag      ans on rel.nod_id = ans.nod_id and ans.k = 'answer'
where
   rel.rel_of = 12120978
order by
   cast(ref.v as integer);
