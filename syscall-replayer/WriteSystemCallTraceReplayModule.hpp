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
 * This header file provides members and functions for implementing write
 * system call.
 *
 * WriteSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying write system call. 
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef WRITE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define WRITE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <iostream>

#include "SystemCallTraceReplayModule.hpp"

class WriteSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
private:
  bool verify_;
  std::string pattern_data_;
  std::ifstream random_file_;
  /* Open System Call Trace Fields in Dataseries file */
  Int32Field descriptor_;
  Variable32Field data_written_;
  Int64Field bytes_requested_;
  
  /*
   * This function will prepare things before replaying any
   * write system call. Right now it displays a starting 
   * message and opens urandom device if pattern is random.
   */
  void prepareForProcessing();

  /*
   * This function will gather arguments in the trace file 
   * or create our own arguments (for example, pattern), 
   * then replay an write system call with those arguments. 
   */
  void processRow();

  /*
   * This function will do things that have be done 
   * after finishing replaying all write system calls. 
   * Now, it displays an ending message and close
   * urandom if pattern is random.
   */
  void completeProcessing();

public:
  WriteSystemCallTraceReplayModule(DataSeriesModule &source,
                                   bool verbose_flag,
                                   bool verify_flag,
                                   int warn_level_flag,
                                   std::string pattern_data);
};

#endif /* WRITE_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
