all: src/osm_objects.o src/osmxml.o src/protobuf.o src/readosm.o

# readosm.dll: src/osm_objects.o src/osmxml.o src/protobuf.o src/readosm.o
# 	rem gcc -dll $^ -o $@ -L"c:\Program Files (x86)\Expat 2.2.6\Source" -lexpat -lzlib
# 	gcc     -dll $^ -o $@                                               -lexpat -lzlib

src/osm_objects.o: src/osm_objects.c
	gcc -Iheaders -c $< -o $@

src/osmxml.o: src/osmxml.c
	gcc -Iheaders -I"c:\Program Files (x86)\Expat 2.2.6\Source" -c $< -o $@

src/protobuf.o: src/protobuf.c
	gcc -Iheaders -c $< -o $@

src/readosm.o: src/readosm.c
	gcc -Iheaders -c $< -o $@
