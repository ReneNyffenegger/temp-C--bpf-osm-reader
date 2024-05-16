TODO:
  - Use STRICT tables
    - Needs version > 3.37.0 (2021-11-27)
  - SQLite concurrent access.


OSM PBF Format
--------------
   the reference implementation of PBF is the Osmosis implementation, split into two parts,
     -the Osmosis-specific part, contained in the Osmosis repository at ==> https://github.com/openstreetmap/osmosis
     - and an application-generic part at ==> https://github.com/openstreetmap/OSM-binary


   https://github.com/osmandapp/OsmAnd-resources/blob/master/protos/osmformat.proto 


QuadTile
--------
  QuadTiles are a geo-data storage/indexing strategy 

  QuadTiles is analog to the Geohash algorithm ( https://wiki.openstreetmap.org/wiki/Geohash  )
  
  https://wiki.openstreetmap.org/wiki/QuadTiles

H3
--
  H3 is a hierarchical geospatial index

  Every hexagonal cell, up to the maximum resolution supported by H3, has seven
  child cells below it in this hierarchy.

  This subdivision is referred to as
    APERTURE 7.


    https://github.com/uber/h3

  https://github.com/igor-suhorukov/openstreetmap_h3

  https://github.com/ReneNyffenegger/leaflet-h3-viewer

------
   Query wiki
      https://wiki.openstreetmap.org/w/api.php?action=query&list=allpages&apprefix=Key:
      curl 'https://en.wikipedia.org/w/api.php?action=query&format=json&list=allpages&aplimit=50&apprefix=Pfung' | jq


------------

   This gives a very basic parsing of osm.pbf files.
      https://gist.github.com/JesseKPhillips/6051600
