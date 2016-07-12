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
 * This header file provides members and functions for implementing
 * writev system call.
 *
 * WritevSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying writev system call.
 *
 * USAGE
 * A main program could initialize this object with a dataseries file
 * and call execute() function until all extents are processed.
 *
 */
#ifndef WRITEV_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define WRITEV_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <unistd.h>
#include <sys/uio.h>

#include "SystemCallTraceReplayModule.hpp"
#include "WriteSystemCallTraceReplayModule.hpp"

class WritevSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  std::string pattern_data_;
  std::ifstream random_file_;
  /* Writev System Call Trace Fields in Dataseries file */
  Int32Field descriptor_;
  Int32Field count_;
  Int32Field iov_number_;
  Variable32Field data_written_;
  Int64Field bytes_requested_;

  /*
   * Print writev sys call field values in a nice format
   */
  void print_specific_fields();

  /*
   * This function will gather arguments in the trace file
   * and then replay a writev system call with those arguments.
   */
  void processRow();

public:
  WritevSystemCallTraceReplayModule(DataSeriesModule &source,
				    bool verbose_flag,
				    int warn_level_flag,
				    std::string pattern_data);
};
#endif /* WRITEV_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
