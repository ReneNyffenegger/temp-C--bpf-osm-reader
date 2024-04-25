node=$1

sqlite3 /mnt/a/osm/pbf/ch.db "
select
   k, v
from
   tag
where
   nod_id = $node;"
