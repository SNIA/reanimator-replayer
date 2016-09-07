/*
 * Copyright (c) 2016 Nina Brown
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
 * This header file provides members and functions for implementing pipe
 * system call.
 *
 * PipeSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying pipe system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 *
 */
#ifndef PIPE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define PIPE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

class PipeSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  bool verify_;
  /* Pipe System Call Trace Fields in Dataseries file */
  Int32Field read_descriptor_;
  Int32Field write_descriptor_;

  /*
   * Print pipe sys call field values in a nice format
   */
  void print_specific_fields();

  /*
   * This function will gather arguments in the trace file
   * and replay a pipe system call with those arguments.
   */
  void processRow();

public:
  PipeSystemCallTraceReplayModule(DataSeriesModule &source,
                                  bool verify_flag,
                                  bool verbose_flag,
                                  int warn_level_flag);

};

#endif /* PIPE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
