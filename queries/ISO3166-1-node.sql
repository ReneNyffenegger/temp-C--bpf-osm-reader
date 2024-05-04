-- select
--    count(*),
--    x.k
-- from
--    tag  iso3166    join
--    tag  x        on iso3166.nod_id = x.nod_id
-- where
--    iso3166.k = 'ISO3166-1' and
--    x.v not in (26847709, 4070744431) -- wrong AT nodes
-- group by
--    x.k
-- order by
--    count(*)
;

select
   iso3166.nod_id,
   iso3166.v,
   wd.v                  wd,
   n_en.v                en_n,
-- n_ru.v                ru_n,
-- n_fr.v                fr_n,
-- n_eo.v                eo_n,
   n_de.v                de_n,
-- n_zh.v                zh_n,
-- n_pl.v                pl_n,
-- n_ar.v                ar_v,
   plac.v                place
from
   tag  iso3166                                                            left join
   tag  wd         on iso3166.nod_id = wd.nod_id   and wd.k   = 'wikidata' left join
   tag  n_en       on iso3166.nod_id = n_en.nod_id and n_en.k = 'name:en'  left join
   tag  n_ru       on iso3166.nod_id = n_ru.nod_id and n_ru.k = 'name:ru'  left join
   tag  n_fr       on iso3166.nod_id = n_fr.nod_id and n_fr.k = 'name:fr'  left join
   tag  n_eo       on iso3166.nod_id = n_eo.nod_id and n_eo.k = 'name:eo'  left join
   tag  n_de       on iso3166.nod_id = n_de.nod_id and n_de.k = 'name:de'  left join
   tag  n_zh       on iso3166.nod_id = n_zh.nod_id and n_zh.k = 'name:zh'  left join
   tag  n_pl       on iso3166.nod_id = n_pl.nod_id and n_pl.k = 'name:pl'  left join
   tag  n_ar       on iso3166.nod_id = n_ar.nod_id and n_ar.k = 'name:ar'  left join
   tag  plac       on iso3166.nod_id = plac.nod_id and plac.k = 'place'
where
   iso3166.k = 'ISO3166-1' and
   iso3166.nod_id not in (26847709, 4070744431) -- wrong AT nodes
;
