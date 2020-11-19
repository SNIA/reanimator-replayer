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

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "SystemCallTraceReplayModule.hpp"

class BasicStatSystemCallTraceReplayModule
    : public SystemCallTraceReplayModule {
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

  // value of data fields
  uint32_t statDev;
  uint32_t statINo;
  uint32_t statMode;
  uint32_t statNLink;
  uint32_t statUID;
  uint32_t statGID;
  uint32_t statRDev;
  uint32_t statBlkSize;
  uint32_t statBlocks;
  int64_t statSize;
  uint64_t statATime;
  uint64_t statMTime;
  uint64_t statCTime;

  /**
   * Print stat, lstat, and fstat sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * Print stat, lstat, and fstat sys call mode values in format as Format:
   * drwxrwxrwx
   */
  int print_mode_value(u_int st_mode);

  /**
   * This function will gather arguments in the trace file
   * and replay a stat, lstat, or fstat system call with those arguments.
   * This function will be defined in the derived classes.
   */
  void processRow() override = 0;

  /**
   * This function will verify that the data contained in the stat buffer
   * of the replayed system call matches the stat buffer data captured in
   * the trace.
   */
  void verifyResult(struct stat replayed_stat_buf);
  void copyStatStruct(uint32_t dev, uint32_t ino, uint32_t mode, uint32_t nlink,
                      uint32_t uid, uint32_t gid, uint32_t rdev,
                      uint32_t blksize, uint32_t blocks, int64_t size,
                      uint64_t atime, uint64_t mtime, uint64_t ctime);

 public:
  BasicStatSystemCallTraceReplayModule(DataSeriesModule &source,
                                       bool verbose_flag, bool verify_flag,
                                       int warn_level_flag);
  void prepareRow() override;
};

class StatSystemCallTraceReplayModule
    : public BasicStatSystemCallTraceReplayModule {
 private:
  // System Call Field pathname stored in DataSeries file
  Variable32Field given_pathname_;
  char *pathname;
  /**
   * Print stat sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and call replay a stat system call with those arguments.
   */
  void processRow() override;

 public:
  StatSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                  bool verify_flag, int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr = new StatSystemCallTraceReplayModule(source, verbose_,
                                                       verify_, warn_level_);
    movePtr->setMove(pathname);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    if (verify_) {
      movePtr->copyStatStruct(statDev, statINo, statMode, statNLink, statUID,
                              statGID, statRDev, statBlkSize, statBlocks,
                              statSize, statATime, statMTime, statCTime);
    }
    return movePtr;
  }
  void setMove(char *path) { pathname = path; }
  void prepareRow() override;
};

class LStatSystemCallTraceReplayModule
    : public BasicStatSystemCallTraceReplayModule {
 private:
  // System Call Field pathname stored in DataSeries file
  Variable32Field given_pathname_;
  char *pathname;

  /**
   * Print lstat sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and call replay an lstat system call with those arguments.
   */
  void processRow() override;

 public:
  LStatSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                   bool verify_flag, int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr = new LStatSystemCallTraceReplayModule(source, verbose_,
                                                        verify_, warn_level_);
    movePtr->setMove(pathname);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    if (verify_) {
      movePtr->copyStatStruct(statDev, statINo, statMode, statNLink, statUID,
                              statGID, statRDev, statBlkSize, statBlocks,
                              statSize, statATime, statMTime, statCTime);
    }
    return movePtr;
  }
  void setMove(char *path) { pathname = path; }
  void prepareRow() override;
};

class FStatSystemCallTraceReplayModule
    : public BasicStatSystemCallTraceReplayModule {
 private:
  // System Call Field descriptor stored in Dataseries ilfe
  Int32Field descriptor_;
  int descriptorVal;

  /**
   * Print fstat sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and call replay an fstat system call with those arguments.
   */
  void processRow() override;

 public:
  FStatSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                   bool verify_flag, int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr = new FStatSystemCallTraceReplayModule(source, verbose_,
                                                        verify_, warn_level_);
    movePtr->setMove(descriptorVal);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    if (verify_) {
      movePtr->copyStatStruct(statDev, statINo, statMode, statNLink, statUID,
                              statGID, statRDev, statBlkSize, statBlocks,
                              statSize, statATime, statMTime, statCTime);
    }
    return movePtr;
  }
  void setMove(int desc) { descriptorVal = desc; }
  void prepareRow() override;
};

class FStatatSystemCallTraceReplayModule
    : public BasicStatSystemCallTraceReplayModule {
 private:
  // System Call Field descriptor stored in Dataseries file
  Int32Field descriptor_;
  Variable32Field given_pathname_;
  Int32Field flags_value_;
  int32_t traced_fd;
  int32_t flag_value;
  char *pathname;

  /**
   * Print fstatat sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and call replay an fstatat system call with those arguments.
   */
  void processRow() override;

 public:
  FStatatSystemCallTraceReplayModule(DataSeriesModule &source,
                                     bool verbose_flag, bool verify_flag,
                                     int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr = new FStatatSystemCallTraceReplayModule(source, verbose_,
                                                          verify_, warn_level_);
    movePtr->setMove(traced_fd, flag_value, pathname);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    if (verify_) {
      movePtr->copyStatStruct(statDev, statINo, statMode, statNLink, statUID,
                              statGID, statRDev, statBlkSize, statBlocks,
                              statSize, statATime, statMTime, statCTime);
    }
    return movePtr;
  }
  void setMove(int fd, int flag, char *path) {
    traced_fd = fd;
    flag_value = flag;
    pathname = path;
  }
  void prepareRow() override;
};
#endif /* BASIC_STAT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
