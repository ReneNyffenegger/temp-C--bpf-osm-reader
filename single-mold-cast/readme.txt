OSM PBF Format
--------------
   the reference implementation of PBF is the Osmosis implementation, split into two parts,
     -the Osmosis-specific part, contained in the Osmosis repository at ==> https://github.com/openstreetmap/osmosis
     - and an application-generic part at ==> https://github.com/openstreetmap/OSM-binary


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
