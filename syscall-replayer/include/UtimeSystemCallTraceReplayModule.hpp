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

#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <utime.h>
#include "SystemCallTraceReplayModule.hpp"

class UtimeSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 protected:
  bool verify_;
  // utime System Call Trace Fields in Dataseries file
  Variable32Field given_pathname_;
  Int64Field access_time_;
  Int64Field mod_time_;
  char *pathname;
  int64_t access_t;
  int64_t mod_t;

  /**
   * Print utime sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and then replay utime system call with those arguments.
   */
  void processRow() override;

 public:
  UtimeSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                   bool verify_flag, int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr = new UtimeSystemCallTraceReplayModule(source, verbose_,
                                                        verify_, warn_level_);
    movePtr->setMove(pathname, access_t, mod_t);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  void setMove(char *path, int64_t access, int64_t mod) {
    pathname = path;
    access_t = access;
    mod_t = mod;
  }
  void prepareRow() override;
};

class UtimesSystemCallTraceReplayModule
    : public UtimeSystemCallTraceReplayModule {
 private:
  /**
   * This function will gather arguments in the trace file
   * and then replay utimes system call with those arguments.
   */
  void processRow() override;

 public:
  UtimesSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                    bool verify_flag, int warn_level_flag);
};

class UtimensatSystemCallTraceReplayModule
    : public UtimeSystemCallTraceReplayModule {
 private:
  // Utimensat System Call Trace Fields in Dataseries file
  Int32Field descriptor_;
  Int32Field flag_value_;

  /**
   * Print utimensat sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and then replay utimensats system call with those arguments.
   */
  void processRow() override;

 public:
  UtimensatSystemCallTraceReplayModule(DataSeriesModule &source,
                                       bool verbose_flag, bool verify_flag,
                                       int warn_level_flag);
};
#endif /* UTIME_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
