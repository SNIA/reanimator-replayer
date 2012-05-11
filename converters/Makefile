
# ----- Make Constants (used by the default build rules in make) -----

OPTFLAGS = -O0 -Wall -Wconversion -Werror -std=c++0x
#CXXFLAGS = -g -Wall -Wextra -pedantic -lDataSeries -I/usr/include/libxml2/ $(OPTFLAGS)
CXXFLAGS = -g -lDataSeries -lboost_regex -I/usr/include/libxml2/ -I/usr/local/boost_1_46_1 $(OPTFLAGS)
CXX      = g++

# ----- Handy Constants (used in this file) -----

TARGETS = csv2ds-extra pre-processor nfsparse
CSV2DS-EXTRA_OBJECTS = FieldSpace.o csv2ds-extra.o
PRE-PROCESSOR_OBJECTS = pre-processor.o
NFSPARSE_OBJECTS = nfsparse.o

# ----- Make Rules -----

all:    $(TARGETS)

csv2ds-extra:    $(CSV2DS-EXTRA_OBJECTS)
		$(CXX) $(CXXFLAGS) -o csv2ds-extra $(CSV2DS-EXTRA_OBJECTS)
pre-processor: $(PRE-PROCESSOR_OBJECTS)
				$(CXX) $(CXXFLAGS) -o pre-processor $(PRE-PROCESSOR_OBJECTS)
nfsparse:	$(NFSPARSE_OBJECTS)
		$(CXX) $(CXXFLAGS) -o nfsparse $(NFSPARSE_OBJECTS)
clean:
		rm -f $(TARGETS) $(CSV2DS-EXTRA_OBJECTS) $(PRE-PROCESSOR_OBJECTS)

# ----- Dependencies (as produced by g++ -MM *.cpp) ----- 
csv2ds-extra.o: csv2ds-extra.cpp FieldSpace.hpp
FieldSpace.o: FieldSpace.cpp FieldSpace.hpp
pre-processor.o: pre-processor.cpp
nfsparse.o: nfsparse.cpp
