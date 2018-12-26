/*
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions for implementing umask
 * system call.
 *
 * UmaskSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying umask system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef UMASK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define UMASK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

class UmaskSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 protected:
  // Umask System Call Trace Fields in Dataseries file
  Int32Field mode_value_;
  mode_t mode;
  /**
   * Print umask sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and replay an umask system call with those arguments.
   */
  void processRow();

 public:
  UmaskSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                   int warn_level_flag);
  SystemCallTraceReplayModule *move() {
    auto movePtr =
        new UmaskSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(mode);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  void setMove(mode_t mod) { mode = mod; }
  void prepareRow();
};
#endif /* UMASK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
