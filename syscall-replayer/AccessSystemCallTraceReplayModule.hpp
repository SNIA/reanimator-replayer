/*
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
 * This header file provides members and functions for implementing access
 * system call.
 *
 * AccessSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying access and faccessat system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */

#ifndef ACCESS_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define ACCESS_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <unistd.h>

class AccessSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
protected:
  // Access System Call Trace Fields in Dataseries file
  Variable32Field given_pathname_;
  Int32Field mode_value_;
  char *pathname;
  int mode_value;

  /**
   * Print this sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and replay a access system call with those arguments.
   */
  void processRow();

public:
  AccessSystemCallTraceReplayModule(DataSeriesModule &source,
				    bool verbose_flag,
				    int warn_level_flag);
  SystemCallTraceReplayModule *move() {
    auto movePtr = new AccessSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(pathname, mode_value);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal, timeRecordedVal,
                       executingPidVal, errorNoVal, returnVal, replayerIndex);
    return movePtr;
  }
  void setMove(char* path, int mode) {
    pathname = path;
    mode_value = mode;
  }
  void prepareRow();
};

class FAccessatSystemCallTraceReplayModule : public AccessSystemCallTraceReplayModule {
private:
  // FAccessat System Call Trace Fields in Dataseries file
  Int32Field descriptor_;
  Int32Field flags_value_;

  /**
   * Print this sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and replay a faccessat system call with those arguments.
   */
  void processRow();

public:
  FAccessatSystemCallTraceReplayModule(DataSeriesModule &source,
				       bool verbose_flag,
				       int warn_level_flag);
};
#endif /* ACCESS_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
