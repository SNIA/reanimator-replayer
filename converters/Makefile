#DS_INCLUDE_DIR = /root/build/opt-centos-6.2-x86_64/include
#DS_LIBRARY_DIR = /root/build/opt-centos-6.2-x86_64/lib

DS_INCLUDE_DIR = $(HOME)/build/include
DS_LIBRARY_DIR = $(HOME)/build/lib
LIBXML_INCLUDE_DIR = /usr/include/libxml2/

OPTFLAGS = -O0 -std=c++0x

#-Wall -Wconversion

CXXFLAGS = -g $(OPTFLAGS) -lLintel -lDataSeries -lboost_regex -I$(DS_INCLUDE_DIR) -I$(LIBXML_INCLUDE_DIR) -L$(DS_LIBRARY_DIR)
CXX      = g++

TARGETS = csv2ds-extra pre-processor nfsparse
CSV2DS_EXTRA_OBJECTS = FieldSpace.o csv2ds-extra.o
NFSPARSE_OBJECTS = nfsparse.o
PREPROCESSOR_OBJECTS = pre-processor.o

all: $(TARGETS)

csv2ds-extra: $(CSV2DS_EXTRA_OBJECTS)
	$(CXX) -o csv2ds-extra $(CSV2DS_EXTRA_OBJECTS) $(CXXFLAGS)

pre-processor: $(PREPROCESSOR_OBJECTS)
	$(CXX) -o pre-processor $(PREPROCESSOR_OBJECTS) $(CXXFLAGS)

nfsparse: $(NFSPARSE_OBJECTS)
	$(CXX) -o nfsparse $(NFSPARSE_OBJECTS) $(CXXFLAGS)

clean:
		rm -f $(TARGETS) $(CSV2DS_EXTRA_OBJECTS) $(PREPROCESSOR_OBJECTS) $(NFSPARSE_OBJECTS)

csv2ds-extra.o: csv2ds-extra.cpp FieldSpace.hpp
FieldSpace.o: FieldSpace.cpp FieldSpace.hpp
pre-processor.o: pre-processor.cpp
nfsparse.o: nfsparse.cpp
