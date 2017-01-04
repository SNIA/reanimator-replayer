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
 * vfork system call.
 *
 * VForkSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying vfork system call.
 *
 * INITIALIZATION AND USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef VFORK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define VFORK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"
#include <sched.h>

class VForkSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  // DataSeries VFork System Call Trace Fields (VFork takes no arguments)

  /**
   * Print vfork sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will simply return without replaying
   * vfork system call.
   */
  void processRow();

public:
  VForkSystemCallTraceReplayModule(DataSeriesModule &source,
                                   bool verbose_flag,
                                   int warn_level_flag);
};

#endif /* VFORK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
