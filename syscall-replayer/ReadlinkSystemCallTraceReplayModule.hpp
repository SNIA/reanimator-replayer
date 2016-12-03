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
 * This header file provides members and functions for implementing readlink
 * system call.
 *
 * ReadlinkSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying readlink system call.
 *
 * USAGE
 * A main program could initialize this object with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef READLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define READLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <unistd.h>

#include "SystemCallTraceReplayModule.hpp"

class ReadlinkSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  /* Readlink System Call Trace Fields in Dataseries file */
  bool verify_;
  Variable32Field given_pathname_;
  Variable32Field link_value_;
  Int32Field buffer_size_;

  /*
   * Print readlink sys call field values in a nice format
   */
  void print_specific_fields();

  /*
   * This function will gather arguments in the trace file
   * and then replay readlink system call with those arguments.
   */
  void processRow();

public:
  ReadlinkSystemCallTraceReplayModule(DataSeriesModule &source,
				   bool verbose_flag,
				   bool verify_flag,
				   int warn_level_flag);
};
#endif /* READLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
