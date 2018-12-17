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
 * This header file provides members and functions for implementing rename
 * system call.
 *
 * RenameSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying rename system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */

#ifndef RENAME_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define RENAME_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <unistd.h>

class RenameSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
protected:
  // Rename System Call Trace Fields in Dataseries file
  Variable32Field given_oldname_;
  Variable32Field given_newname_;
  char *old_pathname;
  char *new_pathname;

  /**
   * Print rename sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and replay a rename system call with those arguments.
   */
  void processRow();

public:
  RenameSystemCallTraceReplayModule(DataSeriesModule &source,
                                  bool verbose_flag,
                                  int warn_level_flag);
  SystemCallTraceReplayModule *move() {
    auto movePtr = new RenameSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(old_pathname, new_pathname);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal, timeRecordedVal,
                       executingPidVal, errorNoVal, returnVal, replayerIndex);
    return movePtr;
  }
  void setMove(char *o_path, char *n_path) {
    old_pathname = o_path;
    new_pathname = n_path;
  }

  void prepareRow();

};

#endif /* RENAME_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
