#
# Copyright (c) 2011 Jack Ma
# Copyright (c) 2011 Vasily Tarasov
# Copyright (c) 2011 Koundinya Santhosh Kumar
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

CXXFLAGS = -O2 -g -Wall -lDataSeries -I/usr/include/libxml2/
CXX = g++

TARGETS = dsblkreplay
REPLAY_OBJECTS = ReplayModule.o Commander.o

all: $(TARGETS)

dsblkreplay: $(REPLAY_OBJECTS)
	$(CXX) $(CXXFLAGS) -o dsblkreplay $(REPLAY_OBJECTS)

# the lines below are obtained using `g++ -MM`
Commander.o: Commander.cpp Commander.hpp
ReplayModule.o: ReplayModule.cpp Commander.hpp

clean:
	rm -f $(TARGETS) $(REPLAY_OBJECTS)

install:
	cp dsblkreplay /usr/local/bin/
