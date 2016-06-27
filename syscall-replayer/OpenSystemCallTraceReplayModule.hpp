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
 * This header file provides members and functions for implementing open
 * system call.
 *
 * OpenSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying open system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 *
 */
#ifndef OPEN_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define OPEN_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class OpenSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  /* Open System Call Trace Fields in Dataseries file */
  Variable32Field given_pathname_;
  Int32Field open_value_;
  Int32Field mode_value_;

  /*
   * Print open sys call field values in a nice format
   */
  void print_specific_fields();

  /*
   * This function will prepare things before replaying any
   * open system call. Right now it displays a starting
   * message.
   */
  void prepareForProcessing();

  /*
   * This function will gather arguments in the trace file
   * and replay an open system call with those arguments.
   */
  void processRow();

  /*
   * This function will do things that have be done
   * after finishing replaying all open system calls.
   * Now, it only displays an ending message.
   */
  void completeProcessing();

public:
  OpenSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  int warn_level_flag);

};

#endif /* OPEN_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
