/*
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions for implementing open
 * and openat system calls.
 *
 * OpenSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying open and openat system calls.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef OPEN_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define OPEN_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class OpenSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
protected:
  // Open System Call Trace Fields in Dataseries file
  Variable32Field given_pathname_;
  Int32Field open_value_;
  Int32Field mode_value_;
  int flags;
  int modeVal;
  int traced_fd;
  char *pathname;

  /**
   * Print open sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and replay an open system call with those arguments.
   */
  void processRow();

public:
  OpenSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  int warn_level_flag);

  void prepareRow();
};

class OpenatSystemCallTraceReplayModule :
  public OpenSystemCallTraceReplayModule {
private:
  // Openat System Call Trace Fields in Dataseries file
  Int32Field descriptor_;

  /**
   * Print openat sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and replay an openat system call with those arguments.
   */
  void processRow();

public:
  OpenatSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  int warn_level_flag);

};

#endif /* OPEN_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
