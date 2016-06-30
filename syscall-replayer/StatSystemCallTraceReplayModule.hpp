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
 * This header file provides members and functions for implementing stat
 * system call.
 *
 * StatSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying stat system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 *
 */
#ifndef STAT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define STAT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class StatSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
protected:
  /* Stat System Call Trace Fields in Dataseries file */
  bool verify_;
  Variable32Field given_pathname_;
  Int32Field stat_result_dev_;
  Int32Field stat_result_ino_;
  Int32Field stat_result_mode_;
  Int32Field stat_result_nlink_;
  Int32Field stat_result_uid_;
  Int32Field stat_result_gid_;
  Int32Field stat_result_blksize_;
  Int32Field stat_result_blocks_;
  Int64Field stat_result_size_;
  Int64Field stat_result_atime_;
  Int64Field stat_result_mtime_;
  Int64Field stat_result_ctime_;

  /*
   * This function will prepare things before replaying any
   * stat system call. Right now it displays a starting
   * message.
   */
  void prepareForProcessing();

  /*
   * Print stat sys call field values in a nice format
   */
  void print_specific_fields();

  /*
   * Print stat sys call mode values in format as
   * Format: drwxrwxrwx
   */
  void print_mode_value(u_int st_mode);

  /*
   * This function will gather arguments in the trace file
   * and replay an stat system call with those arguments.
   */
  void processRow();

  /*
   * This function will do things that have be done
   * after finishing replaying all stat system calls.
   * Now, it only displays an ending message.
   */
  void completeProcessing();

public:
  StatSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  bool verify_flag,
				  int warn_level_flag);

};

class LStatSystemCallTraceReplayModule : public StatSystemCallTraceReplayModule {
private:
  /*
   * This function will prepare things before replaying any
   * lstat system call. Right now it displays a starting
   * message.
   */
  void prepareForProcessing();

  /*
   * Print lstat sys call field values in a nice format
   */
  void print_specific_fields();

  /*
   * This function will gather arguments in the trace file
   * and replay an lstat system call with those arguments.
   */
  void processRow();

  /*
   * This function will do things that have be done
   * after finishing replaying all lstat system calls.
   * Now, it only displays an ending message.
   */
  void completeProcessing();

public:
  LStatSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  bool verify_flag,
				  int warn_level_flag);
};
#endif /* STAT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
