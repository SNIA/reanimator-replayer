/*
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions in order to replay
 * chown, a specific system call.
 *
 * ChownSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying chown system call.
 *
 * INITIALIZATION AND USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef CHOWN_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define CHOWN_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

class ChownSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 private:
  // DataSeries Chown System Call Trace Fields
  Variable32Field given_pathname_;
  Int32Field new_owner_;
  Int32Field new_group_;

  /**
   * Print chown sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and replay a chown system call with those arguments
   */
  void processRow() override;

 public:
  ChownSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                   int warn_level_flag);
};

#endif /* CHOWN_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
