all: readtest ds-simple-write ds-simple-read ds-nfs-blockmap extract-snia-nfs-write

LDFLAGS =  -lboost_program_options -laio -lDataSeries \
	   -lLintel -L$(HOME)/build/lib -L/usr/local/dataseries/lib

CXXFLAGS = $(OPTFLAGS) -D_GNU_SOURCE -D_LARGEFILE_SOURCE \
	   -I/usr/include/libxml2/ -I/usr/local/dataseries/include \
	   -D_FILE_OFFSET_BITS=64 $(LDFLAGS) $(CPPFLAGS) -std=c++11

ds-simple-write: ds-simple-write.C
	g++ -I$(HOME)/build/include -lDataSeries -o $@ $^  $(CXXFLAGS)

ds-simple-read: ds-simple-read.C
	g++ -I$(HOME)/build/include -lDataSeries -o $@ $^  $(CXXFLAGS)

ds-nfs-blockmap: ds-nfs-blockmap.cpp
	g++ -I$(HOME)/build/include -lDataSeries -o $@ $^  $(CXXFLAGS)

extract-snia-nfs-write: extract-snia-nfs-write.cpp
	g++ -I$(HOME)/build/include -lDataSeries -o $@ $^  $(CXXFLAGS)

readtest: readtest.c
	gcc -Wall -Werror -O2 -o $@ $^

fork: fork-fd-test.c
	gcc -Wall -Werror -O2 -o $@ $^

clean:
	rm -f ds-simple-write ds-simple-read ds-nfs-blockmap
	rm -f  extract-snia-nfs-write readtest
	rm -f core \#* *~
	rm -f fork
