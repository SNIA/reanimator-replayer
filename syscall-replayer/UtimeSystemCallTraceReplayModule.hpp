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
 * This header file provides members and functions for implementing utime
 * and utimes system call.
 *
 * UtimeSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying utime system call.
 *
 * USAGE
 * A main program could initialize this object with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef UTIME_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define UTIME_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <unistd.h>
#include <utime.h>
#include "SystemCallTraceReplayModule.hpp"

class UtimeSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
protected:
  bool verify_;
  /* utime System Call Trace Fields in Dataseries file */
  Variable32Field given_pathname_;
  Int64Field access_time_;
  Int64Field mod_time_;

  /*
   * Print utime sys call field values in a nice format
   */
  void print_specific_fields();

  /*
   * This function will gather arguments in the trace file
   * and then replay utime system call with those arguments.
   */
  void processRow();

public:
  UtimeSystemCallTraceReplayModule(DataSeriesModule &source,
				   bool verbose_flag,
				   bool verify_flag,
				   int warn_level_flag);
};

class UtimesSystemCallTraceReplayModule :
  public UtimeSystemCallTraceReplayModule {
private:
  /*
   * This function will prepare things before replaying any
   * utimes system call. Right now it displays a starting
   * message.
   */
  void prepareForProcessing();

  /*
   * This function will gather arguments in the trace file
   * and then replay utimes system call with those arguments.
   */
  void processRow();

  /*
   * This function will do things that have be done
   * after finishing replaying all utimes system calls in the
   * trace files. Now, it only displays an ending message.
   */
  void completeProcessing();

public:
  UtimesSystemCallTraceReplayModule(DataSeriesModule &source,
				    bool verbose_flag,
				    bool verify_flag,
				    int warn_level_flag);
};
#endif /* UTIME_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
