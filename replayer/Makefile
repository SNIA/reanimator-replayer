#
# Copyright (c) 2015 Leixiang Wu
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

CPPFLAGS = -I$(HOME)/build/include -I/usr/include/libxml2/
LDFLAGS =  -lboost_program_options -laio -lDataSeries \
           -L$(HOME)/build/lib
OPTFLAGS = -O2 -g -std=c++0x
CXXFLAGS = $(OPTFLAGS) -D_GNU_SOURCE -D_LARGEFILE_SOURCE \
	   -D_FILE_OFFSET_BITS=64 $(LDFLAGS) $(CPPFLAGS)
CXX = g++

TARGETS = dsblkreplay
REPLAY_OBJECTS = ReplayStats.o ReplayModule.o Commander.o

all: $(TARGETS)

dsblkreplay: $(REPLAY_OBJECTS)
	$(CXX) -o $@ $(REPLAY_OBJECTS) $(CXXFLAGS) 

# the lines below are obtained using `g++ -MM`
Commander.o: Commander.cpp Commander.hpp ReplayStats.hpp
	$(CXX) -c -o Commander.o Commander.cpp $(CXXFLAGS)

ReplayModule.o: ReplayModule.cpp Commander.hpp ReplayStats.hpp
	$(CXX) -c -o ReplayModule.o ReplayModule.cpp $(CXXFLAGS) 

ReplayStats.o: ReplayStats.hpp ReplayStats.cpp
	$(CXX) -c -o ReplayStats.o ReplayStats.cpp $(CXXFLAGS)

clean:
	rm -f $(TARGETS) $(REPLAY_OBJECTS)

install:
	cp dsblkreplay /usr/local/bin/
