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
 * This header file provides members and functions for implementing unlink
 * system call.
 *
 * UnlinkSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying unlink system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */

#ifndef UNLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define UNLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <fcntl.h>
#include <unistd.h>

class UnlinkSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 protected:
  // Unlink System Call Trace Fields in Dataseries file
  Variable32Field given_pathname_;
  char *pathname;
  /**
   * Print this sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and replay a unlink system call with those arguments.
   */
  void processRow() override;

 public:
  UnlinkSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                    int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr =
        new UnlinkSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(pathname);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  void setMove(char *path) { pathname = path; }

  void prepareRow() override;
};

class UnlinkatSystemCallTraceReplayModule
    : public UnlinkSystemCallTraceReplayModule {
 private:
  // Unlinkat System Call Trace Fields in Dataseries file
  Int32Field descriptor_;
  Int32Field flag_value_;

  /**
   * Print this sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and replay a unlinkat system call with those arguments.
   */
  void processRow() override;

 public:
  UnlinkatSystemCallTraceReplayModule(DataSeriesModule &source,
                                      bool verbose_flag, int warn_level_flag);
};

#endif /* UNLINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
