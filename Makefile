.PHONY: proto-files



#pbf2mysql-windows.exe: pbf2sqlite.c
#	@rem gcc  -I"C:\Program Files\MySQL\MySQL Server 8.0\include" -Icopies\readosm-1.1.0\headers -DPBF2MYSQL  $< copies\readosm-1.1.0-win-gcc\src\*.o  -o $@ -lreadosm -L"C:\Program Files\MySQL\MySQL Server 8.0\lib -lmysql  -lzlib -lexpat
#	gcc  -I"C:\Program Files\MySQL\MySQL Server 8.0\include" -Icopies\readosm-1.1.0\headers  -L"C:\Program Files\MySQL\MySQL Server 8.0\lib" -DPBF2MYSQL -lmysql  $< copies/readosm-1.1.0-win-gcc/src/*.o  -o $@ -lzlib -lexpat

pbf2sqlite      : pbf2sqlite.c
	gcc                                              -Icopies/reakdosm    -Icopies/readosm/headers -DPBF2SQLITE $< -o $@ -L copies/readosm/src/.libs -lreadosm -lsqlite3
#	gcc                                              -Icopies/readosm    -Icopies/readosm/headers -DPBF2SQLITE $< -o $@                                       -lsqlite3

#
# It seems that they're not really used
#
# proto-files:
# 	make -C proto-files
