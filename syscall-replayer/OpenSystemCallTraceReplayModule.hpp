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
  Variable32Field given_pathname;
  BoolField flag_read_only;
  BoolField flag_write_only;
  BoolField flag_read_and_write;
  BoolField flag_append;
  BoolField flag_async;
  BoolField flag_close_on_exec;
  BoolField flag_create;
  BoolField flag_direct;
  BoolField flag_directory;
  BoolField flag_exclusive;
  BoolField flag_largefile;
  BoolField flag_no_access_time;
  BoolField flag_no_controlling_terminal;
  BoolField flag_no_follow;
  BoolField flag_no_blocking_mode;
  BoolField flag_no_delay;
  BoolField flag_synchronous;
  BoolField flag_truncate;
  BoolField mode_R_user;
  BoolField mode_W_user;
  BoolField mode_X_user;
  BoolField mode_R_group;
  BoolField mode_W_group;
  BoolField mode_X_group;
  BoolField mode_R_others;
  BoolField mode_W_others;
  BoolField mode_X_others;

  /*
   * This function finds what flags were set in current
   * open system call that ExtentSeries is pointing to and
   * return a bitwise-or'd number that can be directly used in 
   * open system call
   * 
   * @return: a flag number of current open system call that 
   *          is about to replay
   */
  int getFlags();
  
  /*
   * This function finds what modes were set in current open 
   * system call that ExtenSeries is pointing and
   * return a bitwise-or'd number that can be directly used in 
   * open system call
   * 
   * @return: a mode number of current open system call that 
   *          is about to replay
   */
  mode_t getMode();

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
  OpenSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag, int warn_level_flag);

};

#endif /* OPEN_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
