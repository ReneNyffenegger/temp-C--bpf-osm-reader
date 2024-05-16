--
-- https://sqlite.org/forum/forumpost/f0f93922ad
--
explain query plan
with rel as ( select * from rel_mem where rel_of = 12120978),
n_c as (
select
  'nod' el,
   count(*) cnt,
   k
from
   rel                            join
   tag n_t on rel.nod_id = n_t.nod_id
group by
   k
),
w_c as (
select
  'way' el,
   count(*) cnt,
   k
from
   rel                            join
   tag w_t on rel.way_id = w_t.way_id
group by
   k
)
select * from n_c union all
select * from w_c
;     


explain query plan
with rel as ( select * from rel_mem where rel_of = 12120978),
w_c as (
select
  'way' el,
   count(*) cnt,
   k
from
   rel                            join
   tag w_t on rel.way_id = w_t.way_id
group by
   k
)
select * from w_c
;

explain query plan
with rel as ( select * from rel_mem where rel_of = 12120978),
n_c as (
select
  'nod' el,
   count(*) cnt,
   k
from
   rel                            join
   tag n_t on rel.nod_id = n_t.nod_id
group by
   k
)
select * from n_c
;     
