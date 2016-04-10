# Copyright (c) 2015-2016 Leixiang Wu
# Copyright (c) 2015-2016 Shubhi Rani
# Copyright (c) 2015-2016 Erez Zadok
# Copyright (c) 2015-2016 Stony Brook University
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# Note: This make file is going to be invoked by replayer makefile.
#

OPTFLAGS = -O2 -g -Wall -Werror

CPPFLAGS =

LDFLAGS =

CXXFLAGS = $(OPTFLAGS) $(LDFLAGS) $(CPPFLAGS)

CXX = g++

TEST_PROGS = open-test-prog
DEFAULT_TESTS = test-default-open
WARN_TESTS = test-warn-open
ABORT_TESTS = test-abort-open

all-tests: all-tests-default
.PHONY: all-tests

# Default mode testing
all-tests-default: $(DEFAULT_TESTS)
.PHONY: all-tests-default

test-default-%: system-call-test.sh Makefile %-test-prog
	@bash system-call-test.sh $@
.PHONY: test-default-%

# Warn mode testing
all-tests-warn: $(WARN_TESTS)
.PHONY: all-tests-warn

test-warn-%: system-call-test.sh Makefile %-test-prog
	@bash system-call-test.sh $@
.PHONY: test-warn-%

# Abort mode testingx
all-tests-abort: $(ABORT_TESTS)
.PHONY: all-tests-abort

test-abort-%: system-call-test.sh Makefile %-test-prog
	@bash system-call-test.sh $@
.PHONY: test-abort-%

# Build test programs
%-test-prog: %-test-prog.cpp
	$(CXX) -o $@ $? $(CXXFLAGS)

clean:
	rm -f $(TEST_PROGS) *~
