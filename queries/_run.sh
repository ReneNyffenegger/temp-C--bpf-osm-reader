db=$1
qry=$2

  sqlite3 /mnt/a/osm/pbf/$db.db ".read $qry" ".exit"
# sqlite3 /mnt/a/osm/pbf/li.db ".read $qry" ".exit"
# sqlite3 ~/ch.db ".read $qry" ".exit"
