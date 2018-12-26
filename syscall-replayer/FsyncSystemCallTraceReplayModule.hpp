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
 * fsync a specific system call.
 *
 * FsyncSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying cfsync system call.
 *
 * INITIALIZATION AND USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef FSYNC_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define FSYNC_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

class FsyncSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 private:
  // DataSeries Fsync System Call Trace Fields
  Int32Field descriptor_;
  int traced_fd;
  int simulated_ret_val;
  /**
   * Print fsync sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and replay an fsync system call with those arguments
   */
  void processRow();

 public:
  FsyncSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                   int warn_level_flag);
  SystemCallTraceReplayModule *move() {
    auto movePtr =
        new FsyncSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(traced_fd, simulated_ret_val);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  void setMove(int fd, int simulatedRetVal) {
    traced_fd = fd;
    simulated_ret_val = simulatedRetVal;
  }
  void prepareRow();
};

#endif /* FSYNC_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
