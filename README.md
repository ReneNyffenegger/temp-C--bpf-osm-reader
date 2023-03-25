# temp-C--bpf-osm-reader

Trying to create a C++ reader for OpenStreetMap bpf files.

Getting pbf files
  - geofabrik.de
  - https://mirror.init7.net/openstreetmap/pbf/ --> planet-YYMMDD.osm.pbf
  - etc.

## Clones

### clones/OSM-binary: https://github.com/scrosby/OSM-binary

    protoc --proto_path=. --cpp_out=. fileformat.proto
    #   [libprotobuf WARNING google/protobuf/compiler/parser.cc:562] No syntax specified for the proto file: fileformat.proto. Please use 'syntax = "proto2";' or 'syntax = "proto3";' to specify a syntax version. (Defaulted to proto2 syntax.)
    
    cp -v fileformat.pb.h ../include/osmpbf/
    #   'fileformat.pb.h' -> '../include/osmpbf/fileformat.pb.h'
    
    g++ -O3 -Wall -Wextra -pedantic -Wredundant-decls -Wdisabled-optimization -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long -fPIC -c -o fileformat.pb.o fileformat.pb.cc
    
    protoc --proto_path=. --cpp_out=. osmformat.proto
    #   [libprotobuf WARNING google/protobuf/compiler/parser.cc:562] No syntax specified for the proto file: osmformat.proto. Please use 'syntax = "proto2";' or 'syntax = "proto3";' to specify a syntax version. (Defaulted to proto2 syntax.)
    
    cp -v osmformat.pb.h ../include/osmpbf/
    #   'osmformat.pb.h' -> '../include/osmpbf/osmformat.pb.h'
    
    g++ -O3 -Wall -Wextra -pedantic -Wredundant-decls -Wdisabled-optimization -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long -fPIC -c -o osmformat.pb.o osmformat.pb.cc
    
    ar -cr libosmpbf.a fileformat.pb.o osmformat.pb.o
    
    g++ -shared -Wl,-soname,libosmpbf.so.1 -o libosmpbf.so fileformat.pb.o osmformat.pb.o
    
    rm osmformat.pb.cc fileformat.pb.cc
    
    make: Leaving directory '/var/tmp/temp-C--bpf-osm-reader/clones/OSM-binary/src'

### clones/tiny-osmpbf: https://github.com/tyrasd/tiny-osmpbf

## Copies

### readosm-1.1.0

readosm-1.1.0 was downloaded from https://www.gaia-gis.it/fossil/readosm/index
    
