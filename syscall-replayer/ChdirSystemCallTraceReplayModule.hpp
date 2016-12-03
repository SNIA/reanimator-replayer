/*
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
 * This header file provides members and functions for implementing chdir
 * system call.
 *
 * ChdirSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying chdir system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */

#ifndef CHDIR_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define CHDIR_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <unistd.h>

class ChdirSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
protected:
  // Chdir System Call Trace Fields in Dataseries file
  Variable32Field given_pathname_;

  /*
   * Print chdir sys call field values in a nice format
   */
  void print_specific_fields();

  /*
   * This function will gather arguments in the trace file
   * and replay a chdir system call with those arguments.
   */
  void processRow();

public:
  ChdirSystemCallTraceReplayModule(DataSeriesModule &source,
				   bool verbose_flag,
				   int warn_level_flag);
};
#endif /* CHDIR_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
