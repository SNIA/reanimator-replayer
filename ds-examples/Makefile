all: ds-simple-write ds-simple-read ds-nfs-blockmap

LDFLAGS =  -lboost_program_options -laio -lDataSeries \
	   -lLintel -L$(HOME)/build/lib -L/usr/local/dataseries/lib

CXXFLAGS = $(OPTFLAGS) -D_GNU_SOURCE -D_LARGEFILE_SOURCE \
	   -I/usr/include/libxml2/ -I/usr/local/dataseries/include \
	   -D_FILE_OFFSET_BITS=64 $(LDFLAGS) $(CPPFLAGS) -std=c++11

ds-simple-write: ds-simple-write.C
	g++ -I$(HOME)/build/include -lDataSeries -o ds-simple-write ds-simple-write.C  $(CXXFLAGS)
ds-simple-read: ds-simple-read.C
	g++ -I$(HOME)/build/include -lDataSeries -o ds-simple-read ds-simple-read.C  $(CXXFLAGS)
ds-nfs-blockmap: ds-nfs-blockmap.cpp
	g++ -I$(HOME)/build/include -lDataSeries -o ds-nfs-blockmap ds-nfs-blockmap.cpp  $(CXXFLAGS)
clean:
	rm -f ds-simple-write ds-simple-read ds-nfs-blockmap
