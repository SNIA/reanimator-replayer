/*
 * Copyright (c) 2015-2016 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2016 Erez Zadok
 * Copyright (c) 2015-2016 Stony Brook University
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

  /*
   * Print rename sys call field values in a nice format
   */
  void print_specific_fields();

  /*
   * This function will prepare things before replaying any
   * rename system call. Right now it displays a starting
   * message.
   */
  void prepareForProcessing();

  /*
   * This function will gather arguments in the trace file
   * and replay a rename system call with those arguments.
   */
  void processRow();

  /*
   * This function will do things that have be done
   * after finishing replaying all rename system calls.
   * Now, it only displays an ending message.
   */
  void completeProcessing();

public:
  RenameSystemCallTraceReplayModule(DataSeriesModule &source,
                                  bool verbose_flag,
                                  int warn_level_flag);
};

#endif /* RENAME_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
