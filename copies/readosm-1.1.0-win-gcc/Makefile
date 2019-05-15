readosm.dll: src/osm_objects.o src/osmxml.o src/protobuf.o src/readosm.o
	gcc -dll $^ -o $@

src/osm_objects.o: src/osm_objects.c
	gcc -Iheaders -c $< -o $@

src/osmxml.o: src/osmxml.c
	gcc -Iheaders -c $< -o $@

src/protobuf.o: src/protobuf.c
	gcc -Iheaders -c $< -o $@

src/readosm.o: src/readosm.c
	gcc -Iheaders -c $< -o $@
