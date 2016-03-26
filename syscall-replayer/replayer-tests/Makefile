B# Copyright (c) 2015-2016 Leixiang Wu
# Copyright (c) 2015-2016 Shubhi Rani
#1;2c Copyright (c) 2015-2016 Erez Zadok
# Copyright (c) 2015-2016 Stony Brook University
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# Note: This make file is going to be invoked by replayer makefile.
#1;2c

OPTFLAGS = -g -Wall -Werror

CPPFLAGS =

LDFLAGS =

CXXFLAGS = $(OPTFLAGS) $(LDFLAGS) $(CPPFLAGS)

CXX = g++

TARGETS = open-test-prog close-test-prog
# 	  write-test-prog read-test-prog \
#	  truncate-test-prog unlink-test-prog chdir-test-prog symlink-test-prog \
#	  access-test-prog

TESTS = open-test close-test

# Modify following variables if the path of your converter or replayer is different.
# Following variables are going to be used by the test scripts
export SYS_CALL_CONVERTER_DIR=$(HOME)/trace2model/syscall-converter
export SYS_CALL_REPLAYER_DIR=$(HOME)/trace2model/syscall-replayer
export SYS_CALL_REPLAYER_TEST_DIR=$(SYS_CALL_REPLAYER_DIR)/replayer-tests
export SYS_CALL_REPLAYER=system-call-replayer
export STRACE_STRSIZE=4096
export STRACE2DS_SCRIPT=strace2ds.sh

all: $(TARGETS)

%-test-prog: %-test-prog.cpp
	$(CXX) -o $@ $? $(CXXFLAGS)

test: test-messages $(TARGETS) $(TESTS)
.PHONY: test

test-messages:
	@echo "Running tests..."
.PHONY: test-messages

%-test: %-test-prog
	bash $(SYS_CALL_REPLAYER_TEST_DIR)/$@.sh
.PHONY: %-test

clean:
	rm -f $(TARGETS) *~
