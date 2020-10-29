/*
 * Copyright (c) 2017      Darshan Godhia
 * Copyright (c) 2016-2019 Erez Zadok
 * Copyright (c) 2011      Jack Ma
 * Copyright (c) 2019      Jatin Sood
 * Copyright (c) 2017-2018 Kevin Sun
 * Copyright (c) 2015-2017 Leixiang Wu
 * Copyright (c) 2020      Lukas Velikov
 * Copyright (c) 2017-2018 Maryia Maskaliova
 * Copyright (c) 2017      Mayur Jadhav
 * Copyright (c) 2016      Ming Chen
 * Copyright (c) 2017      Nehil Shah
 * Copyright (c) 2016      Nina Brown
 * Copyright (c) 2011-2012 Santhosh Kumar
 * Copyright (c) 2015-2016 Shubhi Rani
 * Copyright (c) 2018      Siddesh Shinde
 * Copyright (c) 2014      Sonam Mandal
 * Copyright (c) 2012      Sudhir Kasanavesi
 * Copyright (c) 2020      Thomas Fleming
 * Copyright (c) 2018-2020 Ibrahim Umit Akgun
 * Copyright (c) 2011-2012 Vasily Tarasov
 * Copyright (c) 2019      Yinuo Zhang
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

  uint32_t statfsType;
  uint32_t statfsBsize;
  uint64_t statfsBlocks;
  uint64_t statfsBfree;
  uint64_t statfsBavail;
  uint64_t statfsFiles;
  uint64_t statfsFFree;
  uint64_t statfsNamelen;
  uint64_t statfsFrsize;
  uint64_t statfsFlags;

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
  void copyStatfsStruct(uint32_t type, uint32_t bsize, uint64_t blocks,
                        uint64_t bfree, uint64_t bavail, uint64_t files,
                        uint64_t ffree, uint64_t namelen, uint64_t frsize,
                        uint64_t flags);

 public:
  BasicStatfsSystemCallTraceReplayModule(DataSeriesModule &source,
                                         bool verbose_flag, bool verify_flag,
                                         int warn_level_flag);
  void prepareRow() override;
};

class StatfsSystemCallTraceReplayModule
    : public BasicStatfsSystemCallTraceReplayModule {
 private:
  // System Call Field pathname stored in DataSeries file
  Variable32Field given_pathname_;
  char *pathname;
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

  SystemCallTraceReplayModule *move() override {
    auto movePtr = new StatfsSystemCallTraceReplayModule(source, verbose_,
                                                         verify_, warn_level_);
    movePtr->setMove(pathname);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    if (verify_) {
      movePtr->copyStatfsStruct(
          statfsType, statfsBsize, statfsBlocks, statfsBfree, statfsBavail,
          statfsFiles, statfsFFree, statfsNamelen, statfsFrsize, statfsFlags);
    }
    return movePtr;
  }
  void setMove(char *path) { pathname = path; }
  void prepareRow() override;
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
