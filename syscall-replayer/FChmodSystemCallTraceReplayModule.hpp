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
 * This header file provides members and functions for implementing fchmod
 * and fchmodat system call.
 *
 * FChmodSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying fchmod and fchmodat
 * system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef FCHMOD_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define FCHMOD_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "SystemCallTraceReplayModule.hpp"

class FChmodSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 protected:
  // FChmod System Call Trace Fields in Dataseries file
  Int32Field descriptor_;
  Int32Field mode_value_;
  int32_t traced_fd, mode_value;

  /**
   * Print fchmod sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and replay an fchmod  system call with those arguments.
   */
  void processRow() override;

 public:
  FChmodSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                    int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr =
        new FChmodSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(traced_fd, mode_value);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  void setMove(int fd, int mode) {
    traced_fd = fd;
    mode_value = mode;
  }
  void prepareRow() override;
};

class FChmodatSystemCallTraceReplayModule
    : public FChmodSystemCallTraceReplayModule {
 private:
  // FChmodat System Call Trace Fields in Dataseries file
  Variable32Field given_pathname_;
  Int32Field flag_value_;

  /**
   * Print fchmod sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and replay an fchmod  system call with those arguments.
   */
  void processRow() override;

 public:
  FChmodatSystemCallTraceReplayModule(DataSeriesModule &source,
                                      bool verbose_flag, int warn_level_flag);
};
#endif /* FCHMOD_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
