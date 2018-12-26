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
 * socket system call.
 *
 * SocketSystemCallTraceReplayerModule is a class/module that
 * has members and functions of replaying socket system call.
 *
 * USAGE
 * A main program could initialize this class with a dataseries file
 * and call execute() function until all extents are processed.
 */
#ifndef SOCKET_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP
#define SOCKET_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP

#include "SystemCallTraceReplayModule.hpp"

#include <sys/socket.h>

class SocketSystemCallTraceReplayModule : public SystemCallTraceReplayModule {
 protected:
  // Socket System Call Trace Fields in Dataseries file
  Int32Field domain_value_;
  Int32Field type_value_;
  Int32Field protocol_value_;

  /**
   * Print socket sys call field values in a nice format
   */
  void print_specific_fields();

  /**
   * This function will gather arguments in the trace file
   * and handles the replay of a socket system call.
   *
   * Handling:
   * - if traced socket() call succeeded we add a special FD
   *   (SYSCALL_SIMULATED) to the fd-map.
   */
  void processRow();

 public:
  SocketSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                    int warn_level_flag);
};
#endif /* SOCKET_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
