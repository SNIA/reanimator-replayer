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
 * This header file provides members and functions for implementing unlink
 * system call.
 *
 * UnlinkSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying unlink system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */

#ifndef UNLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define UNLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <unistd.h>

class UnlinkSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
protected:
  // Unlink System Call Trace Fields in Dataseries file
  Variable32Field given_pathname_;

  /*
   * Print this sys call field values in a nice format
   */
  void print_specific_fields();

  /*
   * This function will prepare things before replaying any
   * unlink system call. Right now it displays a starting
   * message.
   */
  void prepareForProcessing();

  /*
   * This function will gather arguments in the trace file
   * and replay a unlink system call with those arguments.
   */
  void processRow();

  /*
   * This function will do things that have be done
   * after finishing replaying all unlink system calls.
   * Now, it only displays an ending message.
   */
  void completeProcessing();

public:
  UnlinkSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag, int warn_level_flag);
};

#endif /* UNLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
