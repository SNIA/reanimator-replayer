/*
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions for implementing fcntl
 * system call.
 *
 * FcntlSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying fcntl system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 *
 */
#ifndef FCNTL_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define FCNTL_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class FcntlSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  /* Fcntl System Call Trace Fields in Dataseries file */
  Int32Field descriptor_;
  Int32Field command_value_;
  Int32Field argument_value_;
  Int32Field lock_type_;
  Int32Field lock_whence_;
  Int32Field lock_start_;
  Int32Field lock_length_;
  Int32Field lock_pid_;

  /*
   * Print fcntl sys call field values in a nice format
   */
  void print_specific_fields();

  /*
   * This function will gather arguments in the trace file
   * and replay an fcntl system call with those arguments.
   */
  void processRow();

public:
  FcntlSystemCallTraceReplayModule(DataSeriesModule &source,
				   bool verbose_flag,
				   int warn_level_flag);

};

#endif /* FCNTL_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
