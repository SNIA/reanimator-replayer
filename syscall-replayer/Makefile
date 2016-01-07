#
# Copyright (c) 2015-2016 Leixiang Wu
# Copyright (c) 2015-2016 Erez Zadok
# Copyright (c) 2015-2016 Geoff Kuenning
# Copyright (c) 2015-2016 Stony Brook University
# Copyright (c) 2015-2016 Harvey Mudd College
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This Makefile is designed to facilate building all system call modules
# and a system call replayer. 
#
# USAGE
# make clear - clear all the temp files and object files
# make - compile source files that are out-dated
#

OPTFLAGS = -g -O2 -Wall

CPPFLAGS = -I$(HOME)/build/include -I/usr/include/libxml2/

LDFLAGS =  -lboost_program_options -laio -lDataSeries \
	   -lLintel -L$(HOME)/build/lib

CXXFLAGS = $(OPTFLAGS) -D_GNU_SOURCE -D_LARGEFILE_SOURCE \
	   -D_FILE_OFFSET_BITS=64 $(LDFLAGS) $(CPPFLAGS)
CXX      = g++

TARGETS = system-call-replayer

SYSTEM_CALL_REPLAY_MODULE_OBJECTS = SystemCallTraceReplayer.o SystemCallTraceReplayModule.o \
				    OpenSystemCallTraceReplayModule.o CloseSystemCallTraceReplayModule.o \
				    ReadSystemCallTraceReplayModule.o WriteSystemCallTraceReplayModule.o

all: $(TARGETS)

$(TARGETS): $(SYSTEM_CALL_REPLAY_MODULE_OBJECTS)
	$(CXX) -o $@ $(SYSTEM_CALL_REPLAY_MODULE_OBJECTS) $(CXXFLAGS)

%.o: %.cpp
	$(CXX) -c -o $@ $? $(CXXFLAGS)

clean:
	rm -f $(TARGETS) $(SYSTEM_CALL_REPLAY_MODULE_OBJECTS) *~ \#*\#
