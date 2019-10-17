/*
 * Copyright (c) 2017 Darshan Godhia
 * Copyright (c) 2015-2017 Erez Zadok
 * Copyright (c) 2015-2017 Stony Brook University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This header file provides members and functions for implementing the
 * accept system call.
 *
 * AcceptSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying accept system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef ACCEPT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define ACCEPT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <sys/socket.h>

class AcceptSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 protected:
  // accept System Call Trace Fields in Dataseries file
  Int32Field descriptor_value_;
  int32_t socket_fd;

  /**
   * Print accept sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and handles the replay of a accept system call.
   *
   * Handling:
   * - if traced accept() call succeeded we add a special FD
   *   (SYSCALL_SIMULATED) to the fd-map.
   */
  void processRow() override;

 public:
  AcceptSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                    int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr =
        new AcceptSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(socket_fd);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  inline void setMove(int32_t fd) { socket_fd = fd; }
  void prepareRow() override;
};
#endif /* ACCEPT_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
