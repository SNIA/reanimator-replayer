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
 * This header file provides members and functions for implementing link and
 * linkat system calls.
 *
 * LinkSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying link and linkat system calls.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */

#ifndef LINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define LINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <unistd.h>

class LinkSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 protected:
  // Link System Call Trace Fields in Dataseries file
  Variable32Field given_oldpathname_;
  Variable32Field given_newpathname_;
  char *old_pathname;
  char *new_pathname;

  /**
   * Print link sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and replay a link system call with those arguments.
   */
  void processRow() override;

 public:
  LinkSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                  int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr =
        new LinkSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(old_pathname, new_pathname);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  void setMove(char *old_path, char *new_path) {
    old_pathname = old_path;
    new_pathname = new_path;
  }
  void prepareRow() override;
};

class LinkatSystemCallTraceReplayModule
    : public LinkSystemCallTraceReplayModule {
 protected:
  // Linkat System Call Trace Fields in Dataseries file
  Int32Field old_descriptor_;
  Int32Field new_descriptor_;
  Int32Field flag_value_;

  /**
   * Print linkat sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and replay a linkat system call with those arguments.
   */
  void processRow() override;

 public:
  LinkatSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                    int warn_level_flag);
};
#endif /* LINK_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
