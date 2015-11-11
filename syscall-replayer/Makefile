#
# Copyright (c) 2015 Leixiang Wu
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

OPTFLAGS = -g -O2 -Wall

CPPFLAGS = -I$(HOME)/build/include -I/usr/include/libxml2/

LDFLAGS =  -lboost_program_options -laio -lDataSeries \
		   -lLintel -L$(HOME)/build/lib

CXXFLAGS = $(OPTFLAGS) -D_GNU_SOURCE -D_LARGEFILE_SOURCE \
		   -D_FILE_OFFSET_BITS=64 $(LDFLAGS) $(CPPFLAGS)
CXX      = g++

TARGETS = syscall-simple-program syscall-replayer

SYSCALL_SIMPLE_READ_OBJECTS = syscall-simple-program.o

SYSCALL_REPLAY_MODULE_OBJECTS = SysCallReplayModule.o

all: $(TARGETS)

syscall-simple-read: $(SYSCALL_SIMPLE_READ_OBJECTS)
	$(CXX) -o $@ $(SYSCALL_SIMPLE_READ_OBJECTS)

syscall-replayer: $(SYSCALL_REPLAY_MODULE_OBJECTS)
	$(CXX) -o $@ $(SYSCALL_REPLAY_MODULE_OBJECTS) $(CXXFLAGS)

%.o: %.cpp
	$(CXX) -c -o $@ $? $(CXXFLAGS)

clean:
	rm -f $(TARGETS) $(SYSCALL_SIMPLE_READ_OBJECTS) $(SYSCALL_REPLAY_MODULE_OBJECTS) *~

dsblkreplay: $(REPLAY_OBJECTS)
	$(CXX) -o $@ $(REPLAY_OBJECTS)
