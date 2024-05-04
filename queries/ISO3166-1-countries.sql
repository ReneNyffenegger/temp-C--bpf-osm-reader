with 
   n       as (select * from tag where k = 'ISO3166-1' and nod_id is not null),
   r       as (select * from tag where k = 'ISO3166-1' and rel_id is not null),
   iso3166 as (
select
   coalesce(n.v, r.v) v,
   r.rel_id,
   n.nod_id
from
   n full join r on n.v = r.v
)
select
   iso3166.v,
   iso3166.rel_id,
   iso3166.nod_id,
   nam_r_en.v               nam_r_en,
   nam_n_en.v               nam_n_en,
   nam_int.v                nam_int,
   nam_r.v                  nam_r,
   nam_n.v                  nam_n
from
   iso3166                                                                             left join
   tag       nam_int  on iso3166.rel_id = nam_int.rel_id  and nam_int.k  = 'int_name'  left join
   tag       nam_r    on iso3166.rel_id = nam_r.rel_id    and nam_r.k    = 'name'      left join
   tag       nam_n    on iso3166.nod_id = nam_n.nod_id    and nam_n.k    = 'name'      left join
   tag       nam_r_en on iso3166.rel_id = nam_r_en.rel_id and nam_r_en.k = 'name:en'   left join
   tag       nam_n_en on iso3166.nod_id = nam_n_en.nod_id and nam_n_en.k = 'name:en'


order by
   iso3166.v
;
