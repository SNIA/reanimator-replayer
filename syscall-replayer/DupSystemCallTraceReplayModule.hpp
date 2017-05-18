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
 * This header file provides members and functions in order to replay
 * dup system call.
 *
 * DupSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying close system call.
 *
 * INITIALIZATION AND USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef DUP_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define DUP_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

class DupSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  // DataSeries Dup System Call Trace Fields
  Int32Field descriptor_;

  /**
   * Print dup sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and replay a dup system call with those arguments
   */
  void processRow();

public:
  DupSystemCallTraceReplayModule(DataSeriesModule &source,
				 bool verbose_flag,
				 int warn_level_flag);
};

#endif /* DUP_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
