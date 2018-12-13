.PHONY: proto-files

all: pbf2sqlite

pbf2sqlite: pbf2sqlite.c
	gcc  $< -o $@ -lreadosm -lsqlite3

#
# It seems that they're not really used
#
proto-files:
	make -C proto-files
