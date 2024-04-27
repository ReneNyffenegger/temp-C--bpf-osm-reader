pbf2sqlite: pbf2sqlite.c readosm.o Makefile
	gcc -O3 readosm.o pbf2sqlite.c  -lz  -lsqlite3 -o pbf2sqlite

readosm.o: readosm.c pbf2sqlite.c protobuf.c readosm.h readosm_protobuf.h readosm_internals.h Makefile
	gcc -O3 -c readosm.c -o readosm.o

