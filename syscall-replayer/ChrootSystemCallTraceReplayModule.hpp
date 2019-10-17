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
 * This header file provides members and functions for implementing chroot
 * system call.
 *
 * ChrootSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying chroot system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */

#ifndef CHROOT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define CHROOT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <unistd.h>

class ChrootSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 protected:
  // Chroot System Call Trace Fields in Dataseries file
  Variable32Field given_pathname_;
  char *pathname;
  /**
   * Print chroot sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and replay a chroot system call with those arguments.
   */
  void processRow() override;

 public:
  ChrootSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                   int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr =
        new ChrootSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(pathname);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  void setMove(char *path) {
    pathname = path;
  }
  void prepareRow() override;
};
#endif /* CHROOT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
