.PHONY: proto-files



pbf2mysql-windows.exe: pbf2sqlite.c
	@rem gcc  -I"C:\Program Files\MySQL\MySQL Server 8.0\include" -Icopies\readosm-1.1.0\headers -DPBF2MYSQL  $< copies\readosm-1.1.0-win-gcc\src\*.o  -o $@ -lreadosm -L"C:\Program Files\MySQL\MySQL Server 8.0\lib -lmysql  -lzlib -lexpat
	gcc  -I"C:\Program Files\MySQL\MySQL Server 8.0\include" -Icopies\readosm-1.1.0\headers  -L"C:\Program Files\MySQL\MySQL Server 8.0\lib" -DPBF2MYSQL -lmysql  $< copies/readosm-1.1.0-win-gcc/src/*.o  -o $@ -lzlib -lexpat

pbf2sqlite-linux: pbf2sqlite.c
	gcc                                                  -Icopies/readosm-1.1.0/headers -DPBF2SQLITE $< -o $@ -L copies/readosm-1.1.0/src/.libs -lreadosm -lsqlite3

#
# It seems that they're not really used
#
proto-files:
	make -C proto-files
