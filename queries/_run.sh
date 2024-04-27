db=$1
qry=$2

# osmdir=/mnt/a/osm/pbf
  osmdir=/home/rene/osm/pbf

  sqlite3 $osmdir/$db.db ".read $qry" ".exit"  | pspg --csv --csv-separator '|'
# sqlite3 /mnt/a/osm/pbf/li.db ".read $qry" ".exit"
# sqlite3 ~/ch.db ".read $qry" ".exit"
