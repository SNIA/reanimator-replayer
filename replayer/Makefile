#
# Copyright (c) 2011 Jack Ma
# Copyright (c) 2011 Vasily Tarasov
# Copyright (c) 2011 Santhosh Kumar Koundinya
# Copyright (c) 2011 Erez Zadok
# Copyright (c) 2011 Geoff Kuenning
# Copyright (c) 2011 Stony Brook University
# Copyright (c) 2011 Harvey Mudd College
# Copyright (c) 2011 The Research Foundation of SUNY
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#

INCLUDES = -I/usr/include/libxml2/
LIBS = -lDataSeries -laio
CXXFLAGS = -O2 -g -Wall -Werror -std=c++0x -D_GNU_SOURCE -D_LARGEFILE_SOURCE \
			-D_FILE_OFFSET_BITS=64 $(INCLUDES) $(LIBS)
CXX = g++

TARGETS = dsblkreplay
REPLAY_OBJECTS = ReplayStats.o ReplayModule.o Commander.o

all: $(TARGETS)

dsblkreplay: $(REPLAY_OBJECTS)
	$(CXX) $(CXXFLAGS) $(LIBS) -o dsblkreplay $(REPLAY_OBJECTS)

# the lines below are obtained using `g++ -MM`
Commander.o: Commander.cpp Commander.hpp ReplayStats.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o Commander.o Commander.cpp

ReplayModule.o: ReplayModule.cpp Commander.hpp ReplayStats.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o ReplayModule.o ReplayModule.cpp

ReplayStats.o: ReplayStats.hpp ReplayStats.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o ReplayStats.o ReplayStats.cpp

clean:
	rm -f $(TARGETS) $(REPLAY_OBJECTS)

install:
	cp dsblkreplay /usr/local/bin/
