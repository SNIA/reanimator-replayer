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
 * execve system call.
 *
 * ExecveSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying exit system call.
 *
 * INITIALIZATION AND USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef EXECVE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define EXECVE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <unordered_set>
#include "SystemCallTraceReplayModule.hpp"

class ExecveSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 private:
  // DataSeries Execve System Call Trace Fields
  Variable32Field given_pathname_;
  Int32Field continuation_number_;
  Variable32Field argument_;
  Variable32Field environment_;
  int32_t continuation_num;
  int64_t retVal;
  /**
   * Print execve common and sys call field values
   * This function is overridden from its base class
   * SystemCallTraceReplayModule as common field values
   * are not set in the first record.
   */
  void print_sys_call_fields();

  /**
   * Print execve sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will simply return without replaying
   * execve system call.
   */
  void processRow();

 public:
  ExecveSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                    int warn_level_flag);
  SystemCallTraceReplayModule *move() {
    auto movePtr =
        new ExecveSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(continuation_num, retVal);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  void setMove(int32_t contNum, int64_t ret) {
    continuation_num = contNum;
    retVal = ret;
  }
  void prepareRow();
};

#endif /* EXECVE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
