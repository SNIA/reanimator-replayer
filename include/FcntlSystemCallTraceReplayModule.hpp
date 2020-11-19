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
 * This header file provides members and functions for implementing fcntl
 * system call.
 *
 * FcntlSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying fcntl system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef FCNTL_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define FCNTL_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

class FcntlSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 private:
  // Fcntl System Call Trace Fields in Dataseries file
  Int32Field descriptor_;
  Int32Field command_value_;
  Int32Field argument_value_;
  Int32Field lock_type_;
  Int32Field lock_whence_;
  Int32Field lock_start_;
  Int32Field lock_length_;
  Int32Field lock_pid_;

  int traced_fd;
  int command_val;
  int arg_val;
  int lock_type_val;
  int lock_whence_val;
  int lock_start_val;
  int lock_length_val;
  int lock_pid_val;
  int simulated_ret_val;

  /**
   * Print fcntl sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and replay an fcntl system call with those arguments.
   */
  void processRow() override;

 public:
  FcntlSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                   int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr =
        new FcntlSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(traced_fd, command_val, arg_val, lock_type_val,
                     lock_whence_val, lock_start_val, lock_length_val,
                     lock_pid_val, simulated_ret_val);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  void setMove(int fd, int commandVal, int argVal, int lockTypeVal,
               int lockWhenceVal, int lockStartVal, int lockLengthVal,
               int lockPidVal, int simulatedRetVal) {
    traced_fd = fd;
    command_val = commandVal;
    arg_val = argVal;
    lock_type_val = lockTypeVal;
    lock_whence_val = lockWhenceVal;
    lock_start_val = lockStartVal;
    lock_length_val = lockLengthVal;
    lock_pid_val = lockPidVal;
    simulated_ret_val = simulatedRetVal;
  }
  void prepareRow() override;
};

#endif /* FCNTL_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
