all: ds-simple-write ds-simple-read

ds-simple-write: ds-simple-write.C
	g++ -lDataSeries -I/usr/include/libxml2/ -o ds-simple-write ds-simple-write.C
ds-simple-read: ds-simple-read.C
	g++ -lDataSeries -I/usr/include/libxml2/ -o ds-simple-read ds-simple-read.C
clean:
	rm -f ds-simple-write ds-simple-read
