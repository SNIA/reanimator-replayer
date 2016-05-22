# This Makefile builds strace2csv and csv2ds

DS_INCLUDE_DIR = $(HOME)/build/include
DS_LIBRARY_DIR = $(HOME)/build/lib
LIBXML_INCLUDE_DIR = /usr/include/libxml2/

OPTFLAGS = -O0 -std=c++0x

#-Wall -Wconversion

CXXFLAGS = -g $(OPTFLAGS) -lLintel -lDataSeries -I$(DS_INCLUDE_DIR) -I$(LIBXML_INCLUDE_DIR) -L$(DS_LIBRARY_DIR)
CXX      = g++

TARGETS = strace2csv csv2ds
CSV2DS_OBJECTS = FieldSpace.o csv2ds.o
STRACE2CSV_OBJECTS = strace2csv.o strace2csv-util.o

all: $(TARGETS)

csv2ds: $(CSV2DS_OBJECTS)
	$(CXX) -o $@ $(CSV2DS_OBJECTS) $(CXXFLAGS)

strace2csv: $(STRACE2CSV_OBJECTS)
	$(CXX) -o $@ $(STRACE2CSV_OBJECTS) $(CXXFLAGS)

%.o: %.cpp %.hpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
	rm -f $(TARGETS) $(STRACE2CSV_OBJECTS) $(CSV2DS_OBJECTS) *~
