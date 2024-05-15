# C_OPTS = -Werror -O3 -g
  C_OPTS = -Werror -O3
# C_OPTS = -Werror -Ofast #  -Ofast  Enables all -O3 optimizations and some aggressive ones that may break strict compliance with standards.

#C_OPTS = \
   -Werror \
   -Wall \
   -Wextra \
   -pedantic \
   -Wcast-align \
   -Wcast-qual \
   -Wdisabled-optimization \
   -Wformat=2 \
   -Winit-self \
   -Wlogical-op \
   -Wmissing-include-dirs \
   -Wredundant-decls \
   -Wshadow \
   -Wstrict-overflow=5 \
   -Wundef \
   -Wno-unused \
   -Wno-variadic-macros \
   -Wno-parentheses \
   -Wconversion \
   -fdiagnostics-show-option \
 	 -O3

pbf2sqlite: pbf2sqlite.c readosm.o client.h osm-pbf-data-extractor.h Makefile
	gcc $(C_OPTS) readosm.o pbf2sqlite.c  -lz  -lsqlite3 -o pbf2sqlite

readosm.o: readosm.c pbf2sqlite.c protobuf.c osm_objects.c readosm.h readosm_protobuf.h readosm_internals.h client.h osm-pbf-data-extractor.h Makefile
	gcc $(C_OPTS) -c readosm.c -o readosm.o

