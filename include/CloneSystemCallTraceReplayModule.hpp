/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions in order to replay
 * clone system call.
 *
 * CloneSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying clone system call.
 *
 * INITIALIZATION AND USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 *
 */
#ifndef CLONE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define CLONE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <sched.h>
#include "SystemCallTraceReplayModule.hpp"

class CloneSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 private:
  // DataSeries Execve System Call Trace Fields
  Int64Field flag_value_;
  Int64Field child_stack_address_;
  Int64Field parent_thread_id_;
  Int64Field child_thread_id_;
  Int64Field new_tls_;

  int64_t flagVal;
  int64_t childStackAddrVal;
  int64_t parentTIDVal;
  int64_t childTIDVal;
  int64_t newTLSVal;

  /**
   * Print clone sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will simply return without replaying
   * clone system call.
   */
  void processRow() override;

 public:
  CloneSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                   int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr =
        new CloneSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(flagVal, childStackAddrVal, parentTIDVal, childTIDVal,
                     newTLSVal);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }

  inline void setMove(int64_t flag, int64_t childStack, int64_t parentID,
                      int64_t childID, int64_t newTLS) {
    flagVal = flag;
    childStackAddrVal = childStack;
    parentTIDVal = parentID;
    childTIDVal = childID;
    newTLSVal = newTLS;
  }
  void prepareRow() override;
};

#endif /* CLONE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
