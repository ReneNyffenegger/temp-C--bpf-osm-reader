qry=$1

# sqlite3 /mnt/a/osm/pbf/ch.db ".read $qry" ".exit"
  sqlite3 ~/ch.db ".read $qry" ".exit"
