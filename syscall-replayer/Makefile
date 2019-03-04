#
# Copyright (c) 2016 Nina Brown
# Copyright (c) 2015-2017 Leixiang Wu
# Copyright (c) 2015-2016 Shubhi Rani
# Copyright (c) 2015-2016 Erez Zadok
# Copyright (c) 2015-2017 Geoff Kuenning
# Copyright (c) 2015-2017 Stony Brook University
# Copyright (c) 2015-2017 Harvey Mudd College
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
# make test - test the replayer to see if it is working

OPTFLAGS = -g -O4 -Wall

CPPFLAGS = -I/usr/local/dataseries/include -I/usr/include/libxml2/ \
	   -I$(HOME)/lib/strace2ds/include -std=c++11 -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free

LDFLAGS =  -ltbb -lpthread -lboost_program_options -laio -lDataSeries \
	   -lLintel -L/usr/local/dataseries/lib -lprofiler -ltcmalloc

CXXFLAGS = $(OPTFLAGS) -D_GNU_SOURCE -D_LARGEFILE_SOURCE \
	   -D_FILE_OFFSET_BITS=64 $(LDFLAGS) $(CPPFLAGS)
CXX      = g++

TARGETS = system-call-replayer

SYSTEM_CALL_REPLAY_RESOURCES_MANAGER = ReplayerResourcesManager

SYSTEM_CALL_REPLAY_MODULE_OBJECTS = SystemCallTraceReplayer.o \
				    SystemCallTraceReplayLogger.o SystemCallTraceReplayModule.o \
				    OpenSystemCallTraceReplayModule.o \
				    CloseSystemCallTraceReplayModule.o \
				    ReadSystemCallTraceReplayModule.o \
				    WriteSystemCallTraceReplayModule.o \
				    LSeekSystemCallTraceReplayModule.o \
				    TruncateSystemCallTraceReplayModule.o \
				    AccessSystemCallTraceReplayModule.o \
				    UnlinkSystemCallTraceReplayModule.o \
				    RmdirSystemCallTraceReplayModule.o \
				    LinkSystemCallTraceReplayModule.o \
				    CreatSystemCallTraceReplayModule.o \
				    ChdirSystemCallTraceReplayModule.o \
				    SymlinkSystemCallTraceReplayModule.o \
				    MkdirSystemCallTraceReplayModule.o \
				    BasicStatSystemCallTraceReplayModule.o \
				    BasicStatfsSystemCallTraceReplayModule.o \
				    ReadlinkSystemCallTraceReplayModule.o \
				    UtimeSystemCallTraceReplayModule.o \
				    ChmodSystemCallTraceReplayModule.o \
				    ChownSystemCallTraceReplayModule.o \
				    ReadvSystemCallTraceReplayModule.o \
				    WritevSystemCallTraceReplayModule.o \
				    RenameSystemCallTraceReplayModule.o \
				    FsyncSystemCallTraceReplayModule.o \
				    FdatasyncSystemCallTraceReplayModule.o \
				    MknodSystemCallTraceReplayModule.o \
				    PipeSystemCallTraceReplayModule.o \
				    DupSystemCallTraceReplayModule.o \
				    Dup2SystemCallTraceReplayModule.o \
				    ExitSystemCallTraceReplayModule.o \
				    ExecveSystemCallTraceReplayModule.o \
				    MmapSystemCallTraceReplayModule.o \
				    MunmapSystemCallTraceReplayModule.o \
				    FcntlSystemCallTraceReplayModule.o \
				    ExitSystemCallTraceReplayModule.o \
				    GetdentsSystemCallTraceReplayModule.o \
				    IoctlSystemCallTraceReplayModule.o \
				    FChmodSystemCallTraceReplayModule.o \
				    CloneSystemCallTraceReplayModule.o \
				    VForkSystemCallTraceReplayModule.o \
				    UmaskSystemCallTraceReplayModule.o \
				    FTruncateSystemCallTraceReplayModule.o \
				    SetxattrSystemCallTraceReplayModule.o \
				    SocketSystemCallTraceReplayModule.o

# Modify following variables if the path of your replayer is different.
SYS_CALL_REPLAYER_DIR=$(HOME)/lib/trace2model/syscall-replayer
SYS_CALL_REPLAYER_TEST_DIR=$(SYS_CALL_REPLAYER_DIR)/replayer-tests

all: $(TARGETS)

$(TARGETS): $(SYSTEM_CALL_REPLAY_RESOURCES_MANAGER) $(SYSTEM_CALL_REPLAY_MODULE_OBJECTS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(SYSTEM_CALL_REPLAY_RESOURCES_MANAGER): $(SYSTEM_CALL_REPLAY_RESOURCES_MANAGER).cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

%.o: %.cpp %.hpp SystemCallTraceReplayModule.hpp $(SYSTEM_CALL_REPLAY_RESOURCES_MANAGER)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

test: $(TARGETS)
	$(MAKE) -C $(SYS_CALL_REPLAYER_TEST_DIR)
.PHONY: test

clean:
	rm -f $(TARGETS) $(SYSTEM_CALL_REPLAY_MODULE_OBJECTS) $(SYSTEM_CALL_REPLAY_RESOURCES_MANAGER) *~ \#*\#


