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
 * This header file provides members and functions in order to replay
 * clone system call.
 *
 * CloneSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying clone system call.
 *
 * INITIALIZATION AND USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 *
 */
#ifndef CLONE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define CLONE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"
#include <sched.h>

class CloneSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  // DataSeries Execve System Call Trace Fields
  Int64Field flag_value_;
  Int64Field child_stack_address_;
  Int64Field parent_thread_id_;
  Int64Field child_thread_id_;
  Int64Field new_tls_;

  /**
   * Print clone sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will simply return without replaying
   * clone system call.
   */
  void processRow();

public:
  CloneSystemCallTraceReplayModule(DataSeriesModule &source,
				   bool verbose_flag,
				   int warn_level_flag);
};

#endif /* CLONE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
