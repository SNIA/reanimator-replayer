/*
 * Copyright (c) 2016 Nina Brown
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2015-2016 Sonam Mandal
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions for implementing stat,
 * lstat, and fstat system calls.
 *
 * BasicStatSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying stat, lstat, fstat and
 * fstatat system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef BASIC_STAT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define BASIC_STAT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class BasicStatSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
protected:
  // System Call Trace Fields in Dataseries file common to Stat, LStat, and
  bool verify_;
  Int32Field stat_result_dev_;
  Int32Field stat_result_ino_;
  Int32Field stat_result_mode_;
  Int32Field stat_result_nlink_;
  Int32Field stat_result_uid_;
  Int32Field stat_result_gid_;
  Int32Field stat_result_rdev_;
  Int32Field stat_result_blksize_;
  Int32Field stat_result_blocks_;
  Int64Field stat_result_size_;
  Int64Field stat_result_atime_;
  Int64Field stat_result_mtime_;
  Int64Field stat_result_ctime_;

  /**
   * Print stat, lstat, and fstat sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * Print stat, lstat, and fstat sys call mode values in format as Format: drwxrwxrwx
   */
  int print_mode_value(u_int st_mode);

  /**
   * This function will gather arguments in the trace file
   * and replay a stat, lstat, or fstat system call with those arguments.
   * This function will be defined in the derived classes.
   */
  virtual void processRow() = 0;

  /**
   * This function will verify that the data contained in the stat buffer
   * of the replayed system call matches the stat buffer data captured in
   * the trace.
   */
  void verifyResult(struct stat replayed_stat_buf);

public:
  BasicStatSystemCallTraceReplayModule(DataSeriesModule &source,
				       bool verbose_flag,
				       bool verify_flag,
				       int warn_level_flag);

};

class StatSystemCallTraceReplayModule :
  public BasicStatSystemCallTraceReplayModule {
private:
  // System Call Field pathname stored in DataSeries file
  Variable32Field given_pathname_;

  /**
   * Print stat sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and call replay a stat system call with those arguments.
   */
  void processRow();

public:
  StatSystemCallTraceReplayModule(DataSeriesModule &source,
				  bool verbose_flag,
				  bool verify_flag,
				  int warn_level_flag);
};

class LStatSystemCallTraceReplayModule :
  public BasicStatSystemCallTraceReplayModule {
private:
  // System Call Field pathname stored in DataSeries file
  Variable32Field given_pathname_;

  /**
   * Print lstat sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and call replay an lstat system call with those arguments.
   */
  void processRow();

public:
  LStatSystemCallTraceReplayModule(DataSeriesModule &source,
				   bool verbose_flag,
				   bool verify_flag,
				   int warn_level_flag);
};

class FStatSystemCallTraceReplayModule :
  public BasicStatSystemCallTraceReplayModule {
private:
  // System Call Field descriptor stored in Dataseries ilfe
  Int32Field descriptor_;

  /**
   * Print fstat sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and call replay an fstat system call with those arguments.
   */
  void processRow();

public:
  FStatSystemCallTraceReplayModule(DataSeriesModule &source,
				   bool verbose_flag,
				   bool verify_flag,
				   int warn_level_flag);
};

class FStatatSystemCallTraceReplayModule :
  public BasicStatSystemCallTraceReplayModule {
private:
  // System Call Field descriptor stored in Dataseries file
  Int32Field descriptor_;
  Variable32Field given_pathname_;
  Int32Field flags_value_;

  /**
   * Print fstatat sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and call replay an fstatat system call with those arguments.
   */
  void processRow();

public:
  FStatatSystemCallTraceReplayModule(DataSeriesModule &source,
                                   bool verbose_flag,
                                   bool verify_flag,
                                   int warn_level_flag);
};
#endif /* BASIC_STAT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
