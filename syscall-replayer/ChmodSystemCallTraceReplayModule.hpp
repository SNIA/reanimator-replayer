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
 * This header file provides members and functions for implementing chmod
 * system call.
 *
 * ChmodSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying chmod system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 *
 */
#ifndef CHMOD_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define CHMOD_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class ChmodSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  // Chmod System Call Trace Fields in Dataseries file
  Variable32Field given_pathname_;
  Int32Field mode_value_;
  mode_t modeVal;
  char *pathname;

  /**
   * Print chmod sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and replay a chmod system call with those arguments.
   */
  void processRow();

public:
  ChmodSystemCallTraceReplayModule(DataSeriesModule &source,
				   bool verbose_flag,
				   int warn_level_flag);

    SystemCallTraceReplayModule *move() {
    auto movePtr = new ChmodSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(pathname, modeVal);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal, timeRecordedVal,
                       executingPidVal, errorNoVal, returnVal, replayerIndex);
    return movePtr;
  }
  void setMove(char* path, int mode) {
    pathname = path;
    modeVal = mode;
  }
  void prepareRow();
};

#endif /* CHMOD_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
