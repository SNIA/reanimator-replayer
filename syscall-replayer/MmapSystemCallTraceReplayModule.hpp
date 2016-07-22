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
 * This header file provides members and functions in order to replay
 * mmap system call.
 *
 * MmapSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying mmap system call.
 *
 * INITIALIZATION AND USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 *
 */
#ifndef MMAP_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define MMAP_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

class MmapSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  /* DataSeries Mmap System Call Trace Fields */
  Int64Field start_address_;
  Int64Field length_;
  Int32Field protection_value_;
  Int32Field flags_value_;
  Int32Field descriptor_;
  Int64Field offset_;

  /*
   * Print mmap sys call field values in a nice format
   */
  void print_specific_fields();

  /*
   * This function will simply return without replaying
   * mmap system call.
   */
  void processRow();

public:
  MmapSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  int warn_level_flag);
};

#endif /* MMAP_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
