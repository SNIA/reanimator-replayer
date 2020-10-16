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

  uint32_t domain;
  uint32_t type;
  uint32_t protocol;
  /**
   * Print socket sys call field values in a nice format
   */
  void print_specific_fields() override;

  /**
   * This function will gather arguments in the trace file
   * and handles the replay of a socket system call.
   *
   * Handling:
   * - if traced socket() call succeeded we add a special FD
   *   (SYSCALL_SIMULATED) to the fd-map.
   */
  void processRow() override;

 public:
  SocketSystemCallTraceReplayModule(DataSeriesModule &source, bool verbose_flag,
                                    int warn_level_flag);
  SystemCallTraceReplayModule *move() override {
    auto movePtr =
        new SocketSystemCallTraceReplayModule(source, verbose_, warn_level_);
    movePtr->setMove(domain, type, protocol);
    movePtr->setCommon(uniqueIdVal, timeCalledVal, timeReturnedVal,
                       timeRecordedVal, executingPidVal, errorNoVal, returnVal,
                       replayerIndex);
    return movePtr;
  }
  inline void setMove(uint32_t domain_val, uint32_t type_val,
                      uint32_t protocol_val) {
    domain = domain_val;
    type = type_val;
    protocol = protocol_val;
  }
  void prepareRow() override;
};
#endif /* SOCKET_SYSTEM_CALL_TRACE_REPLAY_MODULE_HPP */
