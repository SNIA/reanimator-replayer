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
 * This header file provides members and functions for implementing unlink
 * system call.
 *
 * UnlinkSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying unlink system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */

#ifndef UNLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define UNLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <unistd.h>
#include <fcntl.h>

class UnlinkSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
protected:
  // Unlink System Call Trace Fields in Dataseries file
  Variable32Field given_pathname_;

  /**
   * Print this sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and replay a unlink system call with those arguments.
   */
  void processRow();

public:
  UnlinkSystemCallTraceReplayModule(DataSeriesModule &source,
				    bool verbose_flag,
				    int warn_level_flag);
};

class UnlinkatSystemCallTraceReplayModule :
  public UnlinkSystemCallTraceReplayModule {
private:
  // Unlinkat System Call Trace Fields in Dataseries file
  Int32Field descriptor_;
  Int32Field flag_value_;

  /**
   * Print this sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and replay a unlinkat system call with those arguments.
   */
  void processRow();

public:
  UnlinkatSystemCallTraceReplayModule(DataSeriesModule &source,
				      bool verbose_flag,
				      int warn_level_flag);
};

#endif /* UNLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
