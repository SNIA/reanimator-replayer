/*
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
 * This header file provides members and functions for implementing symlink
 * system call.
 *
 * SymlinkSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying symlink system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */

#ifndef SYMLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define SYMLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <unistd.h>

class SymlinkSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 protected:
  // Symlink System Call Trace Fields in Dataseries file
  Variable32Field target_pathname_;
  Variable32Field given_pathname_;

  /**
   * Print symlink sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and replay a symlink system call with those arguments.
   */
  void processRow();

 public:
  SymlinkSystemCallTraceReplayModule(DataSeriesModule &source,
                                     bool verbose_flag, int warn_level_flag);
};

#endif /* SYMLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
