/*
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
 * This header file provides members and functions for implementing statfs
 * and fstatfs system call.
 *
 * BasicStatSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying statfs and fstatfs system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef BASIC_STATFS_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define BASIC_STATFS_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include <sys/statfs.h>
#include "SystemCallTraceReplayModule.hpp"

class BasicStatfsSystemCallTraceReplayModule
    : public SystemCallTraceReplayModule {
 protected:
  // System Call Trace Fields in Dataseries file common to statfs and fstatfs
  // system call.
  bool verify_;
  Int32Field statfs_result_type_;
  Int32Field statfs_result_bsize_;
  Int64Field statfs_result_blocks_;
  Int64Field statfs_result_bfree_;
  Int64Field statfs_result_bavail_;
  Int64Field statfs_result_files_;
  Int64Field statfs_result_ffree_;
  Int64Field statfs_result_namelen_;
  Int64Field statfs_result_frsize_;
  Int64Field statfs_result_flags_;

  /**
   * Print statfs and fstatfs common fields in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and replay a statfs/fstatfs system call with those arguments.
   * This function will be defined in the derived classes.
   */
  void processRow() override = 0;

  /**
   * This function will verify that the data contained in the statfs buffer
   * of the replayed system call matches the statfs buffer data captured in
   * the trace.
   */
  void verifyResult(struct statfs replayed_statfs_buf);

 public:
  BasicStatfsSystemCallTraceReplayModule(DataSeriesModule &source,
                                         bool verbose_flag, bool verify_flag,
                                         int warn_level_flag);
};

class StatfsSystemCallTraceReplayModule
    : public BasicStatfsSystemCallTraceReplayModule {
 private:
  // System Call Field pathname stored in DataSeries file
  Variable32Field given_pathname_;

  /**
   * Print statfs sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and call replay a statfs system call with those arguments.
   */
  void processRow() override;

 public:
  StatfsSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                    bool verify_flag, int warn_level_flag);
};

class FStatfsSystemCallTraceReplayModule
    : public BasicStatfsSystemCallTraceReplayModule {
 private:
  // System Call Field descriptor stored in DataSeries file
  Int32Field descriptor_;

  /**
   * Print fstatfs sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and call replay a fstatfs system call with those arguments.
   */
  void processRow() override;

 public:
  FStatfsSystemCallTraceReplayModule(DataSeriesModule &source,
                                     bool verbose_flag, bool verify_flag,
                                     int warn_level_flag);
};
#endif /* BASIC_STATFS_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
