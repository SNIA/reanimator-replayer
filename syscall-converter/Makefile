#DS_INCLUDE_DIR = /root/build/opt-centos-6.2-x86_64/include
#DS_LIBRARY_DIR = /root/build/opt-centos-6.2-x86_64/lib

DS_INCLUDE_DIR = $(HOME)/build/include
DS_LIBRARY_DIR = $(HOME)/build/lib
LIBXML_INCLUDE_DIR = /usr/include/libxml2/

OPTFLAGS = -O0 -std=c++0x

#-Wall -Wconversion

CXXFLAGS = -g $(OPTFLAGS) -lLintel -lDataSeries -lboost_regex \
	   -I$(DS_INCLUDE_DIR) -I$(LIBXML_INCLUDE_DIR) \
	   -L$(DS_LIBRARY_DIR)
CXX      = g++

TARGETS = csv2ds strace2csv
CSV2DS_OBJECTS = FieldSpace.o csv2ds.o MultiOutputModule.o
STRACE2CSV_OBJECTS = strace2csv.o strace2csv-util.o

all: $(TARGETS)

csv2ds: $(CSV2DS_OBJECTS)
	$(CXX) -o $@ $(CSV2DS_OBJECTS) $(CXXFLAGS)x

strace2csv: $(STRACE2CSV_OBJECTS)
	$(CXX) -o $@ $(STRACE2CSV_OBJECTS) $(CXXFLAGS)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
	rm -f $(TARGETS) $(CSV2DS_OBJECTS) $(STRACE2CSV_OBJECTS)
