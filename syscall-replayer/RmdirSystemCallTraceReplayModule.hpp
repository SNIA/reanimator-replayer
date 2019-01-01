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
 * This header file provides members and functions in order to replay
 * rmdir a specific system call.
 *
 * RmdirSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying rmdir system call.
 *
 * INITIALIZATION AND USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef RMDIR_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define RMDIR_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <unistd.h>
#include "SystemCallTraceReplayModule.hpp"

class RmdirSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 private:
  // DataSeries Rmdir System Call Trace Fields
  Variable32Field given_pathname_;
  char *pathname;

  /**
   * Print rmdir sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and replay an rmdir system call with those arguments
   */
  void processRow() override;

 public:
  RmdirSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                   int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr =
        new RmdirSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(pathname);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  void setMove(char *path) { pathname = path; }

  void prepareRow() override;
};

#endif /* RMDIR_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
