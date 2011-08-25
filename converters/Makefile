
# ----- Make Constants (used by the default build rules in make) -----

OPTFLAGS = -O2
#CXXFLAGS = -g -Wall -Wextra -pedantic -lDataSeries -I/usr/include/libxml2/ $(OPTFLAGS)
CXXFLAGS = -g -lDataSeries -I/usr/include/libxml2/ -I/usr/local/boost_1_46_1 $(OPTFLAGS)
CXX      = g++

# ----- Handy Constants (used in this file) -----

TARGETS = csv2ds-extra
CSV2DS-EXTRA_OBJECTS = FieldSpace.o csv2ds-extra.o

# ----- Make Rules -----

all:    $(TARGETS)

csv2ds-extra:    $(CSV2DS-EXTRA_OBJECTS)
		$(CXX) $(CXXFLAGS) -o csv2ds-extra $(CSV2DS-EXTRA_OBJECTS)
clean:
		rm -f $(TARGETS) $(CSV2DS-EXTRA_OBJECTS)

# ----- Dependencies (as produced by g++ -MM *.cpp) ----- 
FieldSpace.o: FieldSpace.cpp FieldSpace.hpp
csv2ds-extra.o: csv2ds-extra.cpp
