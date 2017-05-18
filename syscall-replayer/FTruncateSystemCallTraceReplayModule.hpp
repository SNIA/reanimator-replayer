/*
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions for implementing ftruncate
 * system call.
 *
 * FTruncateSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying ftruncate system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */

#ifndef FTRUNCATE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define FTRUNCATE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <unistd.h>
#include <sys/types.h>

class FTruncateSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
protected:
  // FTruncate System Call Trace Fields in Dataseries file
  Int32Field descriptor_;
  Int64Field truncate_length_;

  /**
   * Print ftruncate sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and replay a ftruncate system call with those arguments.
   */
  void processRow();

  /**
   * This function will return the file descriptor that
   * is going to used to replay.
   */
  int getReplayedFD();

public:
  FTruncateSystemCallTraceReplayModule(DataSeriesModule &source,
              bool verbose_flag,
              int warn_level_flag);
};

#endif /* FTRUNCATE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
