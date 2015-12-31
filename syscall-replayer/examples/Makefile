#
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

TARGETS = syscall-simple-program write-simple-program

SYSCALL_SIMPLE_PROGRAM_OBJECTS = syscall-simple-program.o

WRITE_SIMPLE_PROGRAM_OBJECTS = write-simple-program.o

all: $(TARGETS)

syscall-simple-program: $(SYSCALL_SIMPLE_PROGRAM_OBJECTS)
	$(CXX) -o $@ $(SYSCALL_SIMPLE_PROGRAM_OBJECTS)

write-simple-program: $(WRITE_SIMPLE_PROGRAM_OBJECTS)
	$(CXX) -o $@ $(WRITE_SIMPLE_PROGRAM_OBJECTS)

%.o: %.cpp
	$(CXX) -c -o $@ $? $(CXXFLAGS)

clean:
	rm -f $(TARGETS) $(SYSCALL_SIMPLE_PROGRAM_OBJECTS) $(WRITE_SIMPLE_PROGRAM_OBJECTS) *~

