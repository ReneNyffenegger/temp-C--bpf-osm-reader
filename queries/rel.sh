rel=$1

sqlite3 /mnt/a/osm/pbf/ch.db "
select
   k, v
from
   tag
where
   rel_id = $rel;"
