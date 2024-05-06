-- select
--    count(*),
--    substr(v, 1, 3) t_3
-- from
--    tag
-- where
--    k = 'wikipedia'
-- group by
--    t_3
-- order by
--    count(*);

select * from tag where k = 'wikipedia' and v not like '%:%';
